#include "complex_lms_adaptive_fir_filter.h"

struct AmLMSFIRState_64fc {
    int32_t tapsLen;
    Am64fc *taps;
    Am64fc *dly;
};

int AmLMSFIRGetStateSize_64fc(int32_t tapsLen, int32_t *size)
{
    if(!size || tapsLen <= 0) {
        return -1;
    }

    *size = sizeof(Am64fc) * tapsLen * 2 + sizeof(int);
    return 0;
}

int AmLMSFIRInit(AmLMSFIRState_64fc **state,
                 Am64fc *taps,
                 int32_t tapsLen,
                 Am64fc *dlyLine,
                 uint8_t *buffer)
{
    if(!state || tapsLen <= 0) {
        return -1;
    }

    AmLMSFIRState_64fc *statePtr = (AmLMSFIRState_64fc*)buffer;
    *state = statePtr;

    // Initialize pointers
    statePtr->tapsLen = tapsLen;
    statePtr->taps = (Am64fc*)buffer + sizeof(int);
    statePtr->dly = statePtr->taps + sizeof(Am64fc) * tapsLen;

    // Initialize taps
    if(taps) {
        AmLMSFIRSetTaps_64fc(statePtr, taps);
    } else {
        for(int i = 0; i < tapsLen; i++) {
            statePtr->taps[i].re = 0.0;
            statePtr->taps[i].im = 0.0;
        }
    }

    // Initialize delay line
    if(dlyLine) {
        AmLMSFIRSetDlyLine_64fc(statePtr, dlyLine);
    } else {
        for(int i = 0; i < tapsLen; i++) {
            statePtr->dly[i].re = 0.0;
            statePtr->dly[i].im = 0.0;
        }
    }

    return 0;
}

int AmLMSFIRGetTapsLen_64fc(AmLMSFIRState_64fc *state, int32_t *tapsLen)
{
    if(!state || !tapsLen) {
        return -1;
    }

    *tapsLen = state->tapsLen;
    return 0;
}

int AmLMSFIRGetTaps_64fc(AmLMSFIRState_64fc *state, Am64fc *dst)
{
    if(!state || !dst) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        dst[i] = state->taps[i];
    }

    return 0;
}

int AmLMSFIRSetTaps_64fc(AmLMSFIRState_64fc *state, const Am64fc *src)
{
    if(!state || !src) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        state->taps[i] = src[i];
    }

    return 0;
}

int AmLMSFIRGetDlyLine_64fc(AmLMSFIRState_64fc *state, Am64fc *dst)
{
    if(!state || !dst) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        dst[i] = state->dly[i];
    }

    return 0;
}

int AmLMSFIRSetDlyLine_64fc(AmLMSFIRState_64fc *state, const Am64fc *src)
{
    if(!state || !src) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        state->dly[i] = src[i];
    }

    return 0;
}

int AmLMSFIR_64fc(AmLMSFIRState_64fc *state,
                  const Am64fc *src,
                  const Am64fc *ref,
                  Am64fc *dst,
                  int32_t len,
                  double mu,
                  Am64fc *e)
{
    if(!state || !src || !ref || !dst || (len <= 0)) {
        return -1;
    }

    for(int i = 0; i < len; i++) {
        // Update our delay line, move all values forward 1 index.
        for(int j = 1; j < state->tapsLen; j++) {
            state->dly[j-1] = state->dly[j];
        }
        // Set new input value at end.
        state->dly[state->tapsLen - 1] = src[i];

        // Complex filter
        Am64fc y = { 0.0, 0.0 };
        for(int j = 0; j < state->tapsLen; j++) {
            Am64fc r = {
                state->dly[j].re * state->taps[j].re - state->dly[j].im * state->taps[j].im,
                state->dly[j].re * state->taps[j].im + state->dly[j].im * state->taps[j].re
            };
            y.re += r.re;
            y.im += r.im;
        }

        dst[i] = y;

        // Adapt/update step
        // W[i] = W[i] + 2 * mu * e * x*[n]
        // e = error term (cplx)
        // x*[n] = complex conjugate of input

        // Calculate error term
        Am64fc error = {
            ref[i].re - y.re,
            ref[i].im - y.im
        };

        // Update error array output
        if(e) {
            e[i] = error;
        }

        for(int j = 0; j < state->tapsLen; j++) {
            Am64fc xConj = {
                state->dly[j].re,
                -state->dly[j].im
            };
            Am64fc r = {
                error.re * xConj.re - error.im * xConj.im,
                error.re * xConj.im + error.im * xConj.re
            };
            state->taps[j].re += 2.0 * mu * r.re;
            state->taps[j].im += 2.0 * mu * r.im;
        }
    }

    return 0;
}

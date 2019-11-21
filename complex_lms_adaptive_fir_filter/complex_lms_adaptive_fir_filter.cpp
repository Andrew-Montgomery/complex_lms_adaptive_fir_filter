// Copyright (c) 2019 Andrew Montgomery

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "complex_lms_adaptive_fir_filter.h"

struct LMSFIRState_64fc {
    int32_t tapsLen;
    Cplx64 *taps;
    Cplx64 *dly;
};

int LMSFIRGetStateSize_64fc(int32_t tapsLen, int32_t *size)
{
    if(!size || tapsLen <= 0) {
        return -1;
    }

    *size = sizeof(Cplx64) * tapsLen * 2 + sizeof(int);
    return 0;
}

int LMSFIRInit(LMSFIRState_64fc **state,
                 Cplx64 *taps,
                 int32_t tapsLen,
                 Cplx64 *dlyLine,
                 uint8_t *buffer)
{
    if(!state || tapsLen <= 0) {
        return -1;
    }

    LMSFIRState_64fc *statePtr = (LMSFIRState_64fc*)buffer;
    *state = statePtr;

    // Initialize pointers
    statePtr->tapsLen = tapsLen;
    statePtr->taps = (Cplx64*)buffer + sizeof(int);
    statePtr->dly = statePtr->taps + sizeof(Cplx64) * tapsLen;

    // Initialize taps
    if(taps) {
        LMSFIRSetTaps_64fc(statePtr, taps);
    } else {
        for(int i = 0; i < tapsLen; i++) {
            statePtr->taps[i].re = 0.0;
            statePtr->taps[i].im = 0.0;
        }
    }

    // Initialize delay line
    if(dlyLine) {
        LMSFIRSetDlyLine_64fc(statePtr, dlyLine);
    } else {
        for(int i = 0; i < tapsLen; i++) {
            statePtr->dly[i].re = 0.0;
            statePtr->dly[i].im = 0.0;
        }
    }

    return 0;
}

int LMSFIRGetTapsLen_64fc(LMSFIRState_64fc *state, int32_t *tapsLen)
{
    if(!state || !tapsLen) {
        return -1;
    }

    *tapsLen = state->tapsLen;
    return 0;
}

int LMSFIRGetTaps_64fc(LMSFIRState_64fc *state, Cplx64 *dst)
{
    if(!state || !dst) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        dst[i] = state->taps[i];
    }

    return 0;
}

int LMSFIRSetTaps_64fc(LMSFIRState_64fc *state, const Cplx64 *src)
{
    if(!state || !src) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        state->taps[i] = src[i];
    }

    return 0;
}

int LMSFIRGetDlyLine_64fc(LMSFIRState_64fc *state, Cplx64 *dst)
{
    if(!state || !dst) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        dst[i] = state->dly[i];
    }

    return 0;
}

int LMSFIRSetDlyLine_64fc(LMSFIRState_64fc *state, const Cplx64 *src)
{
    if(!state || !src) {
        return -1;
    }

    for(int i = 0; i < state->tapsLen; i++) {
        state->dly[i] = src[i];
    }

    return 0;
}

int LMSFIR_64fc(LMSFIRState_64fc *state,
                const Cplx64 *src,
                const Cplx64 *ref,
                Cplx64 *dst,
                int32_t len,
                double mu,
                Cplx64 *e)
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
        Cplx64 y = { 0.0, 0.0 };
        for(int j = 0; j < state->tapsLen; j++) {
            Cplx64 r = {
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
        Cplx64 error = {
            ref[i].re - y.re,
            ref[i].im - y.im
        };

        // Update error array output
        if(e) {
            e[i] = error;
        }

        for(int j = 0; j < state->tapsLen; j++) {
            Cplx64 xConj = {
                state->dly[j].re,
                -state->dly[j].im
            };
            Cplx64 r = {
                error.re * xConj.re - error.im * xConj.im,
                error.re * xConj.im + error.im * xConj.re
            };
            state->taps[j].re += 2.0 * mu * r.re;
            state->taps[j].im += 2.0 * mu * r.im;
        }
    }

    return 0;
}

#ifndef LMS_ADAPTIVE_FIR_FILTER_H
#define LMS_ADAPTIVE_FIR_FILTER_H

#include <cstdint>

// Complex type
typedef struct Am64fc {
    double re;
    double im;
} Am64fc;

struct AmLMSFIRState_64fc;

// Functions return 0 if completed successfully.
// Functions return -1 when required pointers are NULL or for invalid sizes/lengths.

// tapsLen : Size of the filter. Must be greater than zero.
// size : Pointer to an integer, will contain the size of the buffer in bytes to pass to the
//        init function.
int AmLMSFIRGetStateSize_64fc(int32_t tapsLen, int32_t *size);

// state : Double pointer to state structure. If function returns successfully will point to
//         buffer which will contain all state information.
// taps : If not null, contains tapsLen complex values representing the initial state of the filter.
//        If null, filter will be initialized to zero.
// tapsLen : Size of the filter. Must be greater than zero, must match value used in GetStateSize.
// dlyLine : If not null, contains tapsLen complex values representing initialize state of the
//           delay line. If null, the delay line is initialize to zero.
// buffer : Buffer with size in bytes equal to the size returned from GetStateSize.
int AmLMSFIRInit(AmLMSFIRState_64fc **state, Am64fc *taps, int32_t tapsLen, Am64fc *dlyLine, uint8_t *buffer);

// tapsLen : Pointer to integer to be set to the size of the FIR filter.
int AmLMSFIRGetTapsLen_64fc(AmLMSFIRState_64fc *state, int32_t *tapsLen);

// dst : Pointer to array equal to the size of the FIR filter. Array will be set to the current
//       taps of the FIR filter.
int AmLMSFIRGetTaps_64fc(AmLMSFIRState_64fc *state, Am64fc *dst);

// src : Pointer to array equal to the size of the FIR filter. Overrides the current taps
//       of the FIR filter.
int AmLMSFIRSetTaps_64fc(AmLMSFIRState_64fc *state, const Am64fc *src);

// dst : Pointer to array equal to the size of the FIR filter. Array will be set to the current
//       delay line of the FIR filter.
int AmLMSFIRGetDlyLine_64fc(AmLMSFIRState_64fc *state, Am64fc *dst);

// src : Pointer to array equal to the size of the FIR filter. Overwrites the current delay
//       line of the FIR filter.
int AmLMSFIRSetDlyLine_64fc(AmLMSFIRState_64fc *state, const Am64fc *src);

// src : Input waveform. This waveform will be adapted to fit the reference waveform.
// ref : Reference waveform.
// dst : Output over adaptation steps.
// len : Length of input array.
// mu : Adaptation/convergence rate.
// e : If not NULL, must be an array of length 'len'. Will be set to the error term
//     at each input value.
int AmLMSFIR_64fc(AmLMSFIRState_64fc *state,
                  const Am64fc *src,
                  const Am64fc *ref,
                  Am64fc *dst,
                  int32_t len,
                  double mu,
                  Am64fc *e);

#endif // LMS_ADAPTIVE_FIR_FILTER_H

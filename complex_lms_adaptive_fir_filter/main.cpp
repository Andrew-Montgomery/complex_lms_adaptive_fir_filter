#include <vector>

#include "complex_lms_adaptive_fir_filter.h"

int main()
{
    LMSFIRState_64fc *state = NULL;
    // Filter length
    const int FIRLEN = 5;
    // Input array length
    const int N = 2048;

    // Query the memory requirements for the state, allocate the memory,
    //   then initialize the state.
    int bufferSize = 0;
    LMSFIRGetStateSize_64fc(FIRLEN, &bufferSize);
    std::vector<uint8_t> buffer(bufferSize);
    LMSFIRInit(&state, NULL, FIRLEN, NULL, &buffer[0]);

    // Inputs/outputs
    std::vector<Cplx64> input(N), ref(N), dst(N), e(N);

    // Reference filter
    Cplx64 w[FIRLEN] = {
        { 0.2, 0.0 },
        { 0.0, 0.4 },
        { 0.6, 0.0 },
        { 0.0, 0.8 },
        { 1.0, 0.0 }
    };

    // Random input
    for(int i = 0; i < N; i++) {
        input[i].re = (double)rand() / (double)RAND_MAX;
        input[i].im = (double)rand() / (double)RAND_MAX;
    }

    // Filter input and store as reference.
    for(int i = 0; i < N; i++) {
        Cplx64 sum = { 0.0, 0.0 };
        for(int j = 0; j < FIRLEN; j++) {
            int k = i - j;
            if(k >= 0) {
                Cplx64 c = {
                    input[k].re * w[j].re - input[k].im * w[j].im,
                    input[k].re * w[j].im + input[k].im * w[j].re
                };
                sum.re += c.re;
                sum.im += c.im;
            }
        }
        ref[i] = sum;
    }

    // Adapt step.
    LMSFIR_64fc(state, &input[0], &ref[0], &dst[0], N, 1.0e-2, &e[0]);

    // Request adapted filter taps, should match reference filter.
    Cplx64 wAdapted[FIRLEN];
    LMSFIRGetTaps_64fc(state, wAdapted);

    return 0;
}

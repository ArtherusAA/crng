//
// Created by daniil on 20.12.2021.
//
#include <inc/nist.h>

#define sqrt_2 1.41421356237
#define int64_size 64
#define erfc_const 2.174


double abs(double x) { return x > 0 ? x : -x; }

double sqrt(double x) {
    if (x == 1.0 || x == 0.0) {
        return x;
    }

    double to = x, from, ret;
    if ( x < 1 ) {
        ret = x + (1 - x) / 2;
        from = x;
        to = 1;
    } else {
        ret = 1.0 + (x - 1) / 2;
        from = 1;
    }

    for (int k = 0; k < 32; k++) {
        double z = ret * ret;
        if (z == x) break;
        if (z < x) from = ret;
        else to = ret;
        ret = (from + to) / 2;
    }
    return ret;
}

///First and basic test. If this test fails, the likelihood of other tests failing is high.
bool frequency_test(unsigned n) {
    unsigned iteration_count = n / int64_size + 1;
    int S_n = 0;
    for (unsigned i = 0; i < iteration_count; i++) {
        uint64_t sequence = secure_urand64_doom(); //sequence of bits
        for (unsigned j = 0; j < int64_size; j++) {
            unsigned char bit = sequence & 1; //current bit of sequence
            if ( bit ) {
                S_n += 1;
            } else {
                S_n -= 1;
            }
            sequence = sequence >> 1;
        }
    }
    //P-value = erfc(S_obs/sqrt(2))
    double S_obs = abs(S_n) / sqrt(n);
    S_obs /= sqrt_2;

    return S_obs <= erfc_const ? 1 : 0;
}

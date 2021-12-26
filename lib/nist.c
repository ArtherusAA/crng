//
// Created by daniil on 20.12.2021.
//
#include <inc/nist.h>

#define sqrt_2 1.41421356237
#define int64_size 64

bool frequency_test(unsigned n) {
    unsigned iteration_count = n / int64_size + 1;
    int S_n = 0;
    for (unsigned i = 0; i < iteration_count; i++) {
        uint64_t sequence = secure_urand64_doom();
        for (unsigned j = 0; j < int64_size; j++) {
            unsigned char bit = sequence & 1;
            if ( bit ) {
                S_n += 1;
            } else {
                S_n -= 1;
            }
            sequence = sequence >> 1;
        }
    }
    //P-value = erfc(S_obs/sqrt(2))
    double abs_res, sqrt_res, S_obs, res;
    abs_func(S_n, &abs_res);
    sqrt_func(n, &sqrt_res);

    S_obs = abs_res / sqrt_res;
    S_obs /= sqrt_2;

    erfc_func(S_obs, &res);
    return res >= 0.01 ? true : false;
}

bool frequency_block_test(unsigned n, unsigned M) {
    unsigned block_quantity = n / M;
    double ksi_2 = 0;
    for (unsigned block_count = 0; block_count < block_quantity; block_count++) {
        unsigned bit_count = 0;
        double pi_i = 0;
        uint64_t sequence = secure_urand64_doom();
        while (bit_count < M) {
            if (bit_count % int64_size == 0) {
                sequence = secure_urand64_doom();
            }
            unsigned char bit = sequence & 1;
            sequence = sequence >> 1;
            if ( bit ) {
                pi_i += 1;
            }
            bit_count += 1;
        }
        double term = (pi_i / M - 1./2);
        ksi_2 += term * term;
    }
    ksi_2 *= 4 * M;
    double res;
    gamma_func(block_quantity / 2., ksi_2 / 2, &res);
    return res > 0.01 ? true : false;
}

bool runs_test(unsigned n) {
    unsigned iteration_count = n / int64_size + 1;
    unsigned V_n = 1;
    double _pi = 0;
    unsigned char prev_bit;
    char first_time = 1;
    for (unsigned i = 0; i < iteration_count; i++) {
        uint64_t sequence = secure_urand64_doom();
        for (unsigned j = 0; j < int64_size; j++) {
            unsigned char bit = sequence & 1;
            if ( bit ) {
                _pi += 1;
            }
            if ( !first_time ) {
                if (bit != prev_bit) {
                    V_n += 1;
                }
            } else { first_time = 0; }

            prev_bit = bit;
            sequence = sequence >> 1;
        }
    }
    _pi /= n;
    double sqrt_res, abs_res;
    abs_func(_pi - 1./2, &abs_res);
    sqrt_func(n, &sqrt_res);
    if ( abs_res >= (2 / sqrt_res) ) { //frequency test is not passed
        return false;
    }

    double pi_one_pi = _pi * (1 - _pi), res, ret;
    sqrt_func(2 * n, &sqrt_res);
    abs_func(V_n - 2 * n * pi_one_pi, &abs_res);
    res = abs_res / (2 * sqrt_res * pi_one_pi);

    erfc_func(res, &ret);
    return ret >= 0.01 ? true : false;
}

bool longest_run_of_ones_test(unsigned n, unsigned M) {
    unsigned v[] = {0, 0, 0, 0, 0, 0};
    double probabilities[] = {0.2148, 0.3672, 0.2305, 0.1875, 0, 0};
    unsigned N = n / M, K = 3;

    if (M == 128) {
        K = 5;
        probabilities[0] = 0.1174; probabilities[1] = 0.2430; probabilities[2] = 0.2493;
        probabilities[3] = 0.1752; probabilities[4] = 0.1027; probabilities[5] = 0.1124;
    }

    for (unsigned block_count = 0; block_count < N; block_count++) {
        unsigned bit_count = 0, curr_len = 0, max_len = 0;
        uint64_t sequence = secure_urand64_doom();
        while (bit_count < M) {
            if (bit_count % int64_size == 0) {
                sequence = secure_urand64_doom();
            }
            unsigned char bit = sequence & 1;

            if ( bit ) {
                curr_len += 1;
            } else {
                if (curr_len > max_len) {
                    max_len = curr_len;
                }
                curr_len = 0;
            }

            sequence = sequence >> 1;
            bit_count += 1;
        }
        if (M == 8) {
            if (max_len >= 4) {
                v[3] += 1;
            } else if (max_len <= 1){
                v[0] += 1;
            } else {
                v[max_len - 1] += 1;
            }
        } else { //M = 128
            if (max_len >= 9) {
                v[5] += 1;
            } else if (max_len <= 4) {
                v[0] += 1;
            } else {
                v[max_len - 4] += 1;
            }
        }
    }

    double ksi_2 = 0;
    for (int i = 0; i < K; i++) {
        double pi_i = probabilities[i];
        double temp = v[i] - N * pi_i;
        ksi_2 += (temp * temp) / (N * pi_i);
    }
    double res;
    gamma_func(K / 2., ksi_2 / 2, &res);
    return res > 0.01 ? true : false;
}

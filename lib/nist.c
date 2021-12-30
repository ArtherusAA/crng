//
// Created by daniil on 20.12.2021.
//
#include <inc/nist.h>

#define sqrt_2 1.41421356237
#define int64_size 64

bool frequency_test(unsigned n, unsigned not_used, uint64_t (*rand_func)()) {
    unsigned iteration_count = n / int64_size + 1;
    int S_n = 0;
    for (unsigned i = 0; i < iteration_count; i++) {
        uint64_t sequence = rand_func();
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

bool frequency_block_test(unsigned n, unsigned M, uint64_t (*rand_func)()) {
    unsigned block_quantity = n / M;
    double ksi_2 = 0;
    for (unsigned block_count = 0; block_count < block_quantity; block_count++) {
        unsigned bit_count = 0;
        double pi_i = 0;
        uint64_t sequence = rand_func();
        while (bit_count < M) {
            if (bit_count % int64_size == 0) {
                sequence = rand_func();
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

bool runs_test(unsigned n, unsigned not_used, uint64_t (*rand_func)()) {
    unsigned iteration_count = n / int64_size + 1;
    unsigned V_n = 1;
    double _pi = 0;
    unsigned char prev_bit;
    char first_time = 1;
    for (unsigned i = 0; i < iteration_count; i++) {
        uint64_t sequence = rand_func();
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

bool longest_run_of_ones_test(unsigned n, unsigned M, uint64_t (*rand_func)()) {
    unsigned v[] = {0, 0, 0, 0, 0, 0, 0};
    double probabilities[] = {0.2148, 0.3672, 0.2305, 0.1875, 0, 0, 0};
    unsigned N = n / M, K = 3;

    if (M == 128) {
        K = 5;
        probabilities[0] = 0.1174; probabilities[1] = 0.2430; probabilities[2] = 0.2493;
        probabilities[3] = 0.1752; probabilities[4] = 0.1027; probabilities[5] = 0.1124;
    }
    if (M == 10000) {
        K = 6;
        probabilities[0] = 0.0882; probabilities[1] = 0.2092; probabilities[2] = 0.2483;
        probabilities[3] = 0.1933; probabilities[4] = 0.1208; probabilities[5] = 0.0675;
        probabilities[6] = 0.0727;
    }

    for (unsigned block_count = 0; block_count < N; block_count++) {
        unsigned bit_count = 0, curr_len = 0, max_len = 0;
        uint64_t sequence = rand_func();
        while (bit_count < M) {
            if (bit_count % int64_size == 0) {
                sequence = rand_func();
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
        } else if (M == 128){ //M = 128
            if (max_len >= 9) {
                v[5] += 1;
            } else if (max_len <= 4) {
                v[0] += 1;
            } else {
                v[max_len - 4] += 1;
            }
        } else { //M = 10000
            if (max_len >= 16) {
                v[6] += 1;
            } else if (max_len <= 10) {
                v[0] += 1;
            } else {
                v[max_len - 10] += 1;
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

#define matrix_size 32
uint32_t matrix[matrix_size];

//Check 32x32 matrix's rank
int matrix_rank(uint64_t (*rand_func)()) {
    uint64_t sequence = rand_func();;
    for (int i = 0; i < matrix_size; i++) {
        if (i % 2 == 0) { //обновляем число каждую вторую итерацию
            sequence = rand_func();
        }
        matrix[i] = (uint32_t)sequence;
        sequence = sequence >> (int64_size / 2);
    }
    for (int row = 0; row < matrix_size; row++) {
        uint32_t seq = matrix[row], mask = 1 << (int64_size / 2 - 1);
        int first_one = -1;
        for (int i = 0; i <= int64_size / 2; i++) {
            if (seq & mask) {
                first_one = int64_size / 2 - i;
                break;
            }
            mask = mask >> 1;
        }
        mask = 1 << (first_one - 1);
        for (int inner_row = 0; inner_row < matrix_size; inner_row++) {
            if ((matrix[inner_row] & mask) && inner_row != row) {
                uint32_t res = matrix[inner_row] ^ matrix[row]; //xor
                matrix[inner_row] = res;
            }
        }
    }
    int rank = 0;
    for (int row = 0; row < matrix_size; row++) {
        if (matrix[row]) {
            rank += 1;
        }
    }
    return rank;
}

///The focus of the test is the rank of disjoint sub-matrices of the entire sequence.
bool binary_matrix_rank_test(unsigned no_used, unsigned not_used, uint64_t (*rand_func)()) {
    unsigned N = 38, F_M = 0, F_M_1 = 0, F_other = 0;
    for (int i = 0; i < N; i++) {
        int rank = matrix_rank(rand_func);
        switch (rank) {
        case 32: F_M++; break;
        case 31: F_M_1++; break;
        default: F_other++;
        }
    }
    double temp1 = F_M - 0.2888 * N, temp2 = F_M_1 - 0.5776 * N, temp3 = F_other - 0.1336 * N;
    double ksi2 = (temp1 * temp1) / (0.2888 * N) + (temp2 * temp2) / (0.5776 * N) + (temp3 * temp3) / (0.1336 * N);

    double res;
    gamma_func(1., ksi2 / 2, &res);
    return res > 0.01 ? true : false;
}
#undef matrix_size

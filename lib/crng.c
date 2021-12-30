//
// Created by daniil on 20.12.2021.
//
#define bool _Bool
#include <stdint.h>
#include <inc/rand_isaac.h>
#include <inc/crng.h>

#define PRIME_SIZE 1000000

extern bool InternalX86RdRand32(uint32_t *Rand); 
extern bool InternalX86RdRand64(uint64_t *Rand);
void get_prime_numbers(uint32_t *first, uint32_t *second);

struct isaac_state isaac_state;
bool isaac_initialized = 0;

volatile uint64_t s_entropy[32];
volatile size_t s_entropy_begin = 0, s_entropy_end = 0;

void initialize_isaac(void) {
    for (int i = 0; i < ISAAC_WORDS; i++) {
        isaac_state.m[i] = secure_rand64_rdrand(); 
    }
    isaac_seed(&isaac_state);
    isaac_initialized = 1;
}

uint64_t secure_rand64_rdrand(void) {
    uint64_t res;
    if (!InternalX86RdRand64(&res)) {
        res = secure_rand64_doom();
    }
    return res;
}

uint32_t secure_rand32_rdrand(void) {
    return (uint32_t)secure_rand64_rdrand();
}

uint64_t secure_urand64_rdrand(void) {
    isaac_word arr[ISAAC_WORDS];
    if (!isaac_initialized) {
        initialize_isaac();
    }
    isaac_refill(&isaac_state, arr);
    return arr[0];
}

uint32_t secure_urand32_rdrand(void) {
    return (uint32_t)secure_urand64_rdrand();
}

uint64_t secure_urand64_doom(void) {
    static int first_call = 0;
    static uint64_t x = 0;
    static uint64_t M;
    if (!first_call) {
        uint32_t p, q;
        get_prime_numbers(&p, &q);
        M = p * q;
        x = secure_rand64_doom();
        first_call = 1;
    }
    uint64_t result = x & 1U;
    for (int pow = 1; pow < 64; pow++) {
        x = (x * x) % M;
        result = (result << 1U) | (x & 1U);
    }
    return result;
}

uint64_t secure_rand64_doom(void) {
    static int iter = 0;
    uint64_t res = 0;
    for (int i = 0; i < 8; i++) {
        res <<= 8;
        res += s_entropy[iter++] % 256u;
        iter %= 32;
    }
    return res;
}

uint32_t secure_urand32_doom(void) {
    return (uint32_t)secure_urand64_doom();
}

uint32_t secure_rand32_doom(void) {
    return (uint32_t)secure_rand64_doom();
}

unsigned char is_prime[PRIME_SIZE + 1];

void get_prime_numbers(uint32_t *first, uint32_t *second) {
    unsigned limit = PRIME_SIZE, sqr_lim;
    int x2, y2;
    int n;

    sqr_lim = 1000;
    for (int i = 0; i <= limit; ++i)
        is_prime[i] = 0;
    is_prime[2] = 1; is_prime[3] = 1;

    x2 = 0;
    for (int i = 1; i <= sqr_lim; ++i) {
        x2 += 2 * i - 1;
        y2 = 0;
        for (int j = 1; j <= sqr_lim; ++j) {
            y2 += 2 * j - 1;

            n = 4 * x2 + y2;
            if ((n <= limit) && (n % 12 == 1 || n % 12 == 5))
                is_prime[n] = !is_prime[n];

            n -= x2; // n = 3 * x2 + y2;
            if ((n <= limit) && (n % 12 == 7))
                is_prime[n] = !is_prime[n];

            n -= 2 * y2; // n = 3 * x2 - y2;
            if ((i > j) && (n <= limit) && (n % 12 == 11))
                is_prime[n] = !is_prime[n];
        }
    }

    for (int i = 5; i <= sqr_lim; ++i) {
        if (is_prime[i]) {
            n = i * i;
            for (int j = n; j <= limit; j += n)
                is_prime[j] = 0;
        }
    }
    is_prime[0] = is_prime[1] = is_prime[2] = is_prime[3] = 0;
    uint32_t res1 = secure_rand64_doom() % limit, res2 = secure_rand64_doom() % limit;
    while ( !is_prime[res1] || (res1 % 4 != 3)) {
        res1 = (res1 + 1) % limit;
    }
    while ( !is_prime[res2] || (res2 % 4 != 3)) {
        res2 = (res2 + 1) % limit;
    }
    *first = res1;
    *second = res2;
}


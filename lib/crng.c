//
// Created by daniil on 20.12.2021.
//
#define bool _Bool
#include <stdint.h>
#include <inc/rand_isaac.h>
#include <inc/crng.h>
#include <inc/x86.h>

extern bool InternalX86RdRand32(uint32_t *Rand); 
extern bool InternalX86RdRand64(uint64_t *Rand);

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
    uint32_t rdrand_enabled;
    static bool first_call = true;
    cpuid(0, NULL, NULL, &rdrand_enabled, NULL);
    rdrand_enabled >>= 29u;
    rdrand_enabled &= 1u;
    if (!rdrand_enabled || !InternalX86RdRand64(&res)) {
        if (first_call) {
            res = secure_rand64_doom();
            first_call = false;
        } else {
            res = secure_urand64_doom();
        }
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
        uint32_t p = 1215752191, q = 1215752173;
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
    uint64_t res = 0;
    while (s_entropy_end - s_entropy_begin < 16) {}
    for (int i = 0; i < 16; i++) {
        res <<= 4;
        res += s_entropy[s_entropy_begin++] % 16u;
    }
    return res;
}

uint32_t secure_urand32_doom(void) {
    return (uint32_t)secure_urand64_doom();
}

uint32_t secure_rand32_doom(void) {
    return (uint32_t)secure_rand64_doom();
}


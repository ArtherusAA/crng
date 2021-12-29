//
// Created by daniil on 20.12.2021.
//
#define bool _Bool
#include <stdint.h>
#include <inc/rand_isaac.h>
#include <inc/crng.h>

#define ENTROPY_SEED 100
#define PRIME_SIZE 1000000

extern bool InternalX86RdRand32(uint32_t *Rand); 
extern bool InternalX86RdRand64(uint64_t *Rand);
uint64_t make_entropy_doom();
void get_prime_numbers(uint32_t *first, uint32_t *second);

struct isaac_state isaac_state;
bool isaac_initialized = 0;

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

static unsigned char rndtable[] = {
        109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66 ,
        74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36 ,
        95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188 ,
        52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224 ,
        149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242 ,
        145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0 ,
        175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235 ,
        25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113 ,
        94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75 ,
        136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196 ,
        135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113 ,
        80, 250, 108,7, 255, 237
};

uint64_t secure_rand64_doom(void) {
    static uint64_t ind = ENTROPY_SEED;
    //ind += pmtimer_get_timeval();
    uint64_t result = 0;
    for (int k = 0; k < sizeof(uint64_t); k++) {
        ind += 3;
        uint64_t new_byte = rndtable[(k + ind) % sizeof(rndtable)];
        result = result | (new_byte << k * 8);
    }
    return result;
}

uint32_t secure_urand32_doom(void) {
    return (uint32_t)secure_urand64_doom();
}

uint32_t secure_rand32_doom(void) {
    return (uint32_t)secure_rand64_doom();
}

uint64_t make_entropy_doom() {
    static uint64_t ind = ENTROPY_SEED;
    //ind += pmtimer_get_timeval();
    uint64_t result = 0;
    for (int k = 0; k < sizeof(uint64_t); k++) {
        ind += 3;
        uint64_t new_byte = rndtable[(k + ind) % sizeof(rndtable)];
        result = result | (new_byte << k * 8);
    }
    return result;
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


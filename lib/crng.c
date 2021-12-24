//
// Created by daniil on 20.12.2021.
//
#define bool _Bool
#include <stdint.h>
#include <inc/crng.h>

extern bool InternalX86RdRand32(uint32_t *Rand); 
extern bool InternalX86RdRand64(uint64_t *Rand);

// uint64_t secure_rand64_rdrand(void);
// uint32_t secure_rand32_rdrand(void);

// uint64_t secure_urand64_rdrand(void);
// uint32_t secure_urand32_rdrand(void);


// uint64_t secure_rand64_doom(void);
// uint32_t secure_rand32_doom(void);

// uint64_t secure_urand64_doom(void);
// uint32_t secure_urand32_doom(void);


// uint64_t get_entropy_rdrand(void);
// uint64_t get_entropy_doom(void);

int64_t secure_rand64_rdrand(void) {
    int64_t res;
    if (!InternalX86RdRand64((uint64_t *)(&res))) {
        return secure_rand64_doom();
    }
    return res;
}

int32_t secure_rand32_rdrand(void) {
    int32_t res;
    if (!InternalX86RdRand32((uint32_t *)(&res))) {
        return secure_rand32_doom();
    }
    return res;
}

uint64_t secure_urand64_rdrand(void) {
    uint64_t res;
    if (!InternalX86RdRand64(&res)) {
        return secure_urand64_doom();
    }
    return res;
}

uint32_t secure_urand32_rdrand(void) {
    uint32_t res;
    if (!InternalX86RdRand32(&res)) {
        return secure_urand32_doom();
    }
    return res;
}


int64_t secure_rand64_doom(void) {
    return 0;
}

int32_t secure_rand32_doom(void) {
    return 0;
}


uint64_t secure_urand64_doom(void) {
    return 0;
}

uint32_t secure_urand32_doom(void) {
    return 0;
}


uint64_t get_entropy_rdrand(void) {
    return 0;
}

uint64_t get_entropy_doom(void) {
    return 0;
}

//
// Created by daniil on 20.12.2021.
//


#ifndef OSCOURSE_CRNG_H
#define OSCOURSE_CRNG_H
#include <stdint.h>

int64_t secure_rand64_rdrand(void);
int32_t secure_rand32_rdrand(void);

uint64_t secure_urand64_rdrand(void);
uint32_t secure_urand32_rdrand(void);


int64_t secure_rand64_doom(void);
int32_t secure_rand32_doom(void);

uint64_t secure_urand64_doom(void);
uint32_t secure_urand32_doom(void);

#endif // OSCOURSE_CRNG_H

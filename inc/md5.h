//
// Created by daniil on 02.01.2022.
//

#ifndef CRNG_MD5_H
#define CRNG_MD5_H

#include <stddef.h>

#define HASHSIZE       16

#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
typedef uint32_t WORD32;
#else
/* static assert that int equal or greater than 32bit. */
typedef char static_assert_sizeof_int
        [sizeof(unsigned int) >= 4 ? 1 : -1];
typedef unsigned int WORD32;
#endif

typedef struct md5_s {
    WORD32 d[4];
    size_t len;
} md5_t;

void md5_init(md5_t* m);
int  md5_update(md5_t* m, const char* message, size_t len);
void md5_finish(md5_t* m, char output[HASHSIZE]);
void md5(const char* message, size_t len, char output[HASHSIZE]);

#endif // CRNG_MD5_H

//
// Created by daniil on 02.01.2022.
//

#ifndef CRNG_CURVE_H
#define CRNG_CURVE_H

#include<inc/bn.h>
#include<inc/string.h>

typedef struct curve_s curve;
typedef struct bignum_curve_s bignum_curve_t;
typedef struct pnt_s point;


void ellip_curve_init(bignum_curve_t* ellip_curve, curve* ellip);
void elliptic_init_zero(point* p);
void elliptic_mul(point* x, bignum* kt, bignum* a, bignum* pt, point* result);
void elliptic_add(point* p1, point* p2, point* p3, bignum* a, bignum* p);
int elliptic_point_eq(point* p1, point* p2);
void neg_elliptic_point(point* src, bignum* p, point* dst);

struct pnt_s
{
    bignum x;
    bignum y;
    int zero_flag;
};

struct curve_s
{
    const char* p;
    const char* n;
    const char* SEED;
    const char* c;
    const char* b;
    const char* Gx;
    const char* Gy;
};

struct bignum_curve_s
{
    bignum p;
    bignum n;
    bignum SEED;
    bignum c;
    bignum b;
    bignum Gx;
    bignum Gy;
};

#endif // CRNG_CURVE_H

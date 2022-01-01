//
// Created by daniil on 20.12.2021.
//

#ifndef OSCOURSE_ECDSA_H
#define OSCOURSE_ECDSA_H
#include <inc/bn.h>
#include <inc/string.h>
#include <inc/md5.h>
#include <inc/curve.h>
#include <inc/crng.h>

void ecdsa_public_key(bignum *da, curve *ellip, point *ha);
void ecdsa_sign(
        curve *ellip, // curve, IN
        bignum *z, // hash, IN
        bignum *da, // private key, IN
        bignum *r, // r, OUT
        bignum *s // s, OUT
);
int ecdsa_verify(
        bignum *z, // z, IN
        bignum *r, // r, IN
        bignum *s, // s, IN
        curve *ellip, // curve, IN
        point *ha // HA, IN
);


#endif // OSCOURSE_ECDSA_H

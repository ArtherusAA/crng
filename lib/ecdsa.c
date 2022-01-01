//
// Created by daniil on 20.12.2021.
//
#include <inc/ecdsa.h>

extern curve p_192;

void bignum_gen_mod(bignum* k, bignum* n, uint32_t (*rand_func) (void));

//y^2 ≡ x^3 – 3x + b (mod p) //a = -3

void ecdsa_public_key(bignum *da, curve *ellip, point *ha) {

    bignum_curve_t ellip_curve;
    ellip_curve_init(&ellip_curve, ellip);
    point G;
    G.zero_flag = 0;
    bignum_copy(&G.x, &ellip_curve.Gx);
    bignum_copy(&G.y, &ellip_curve.Gy);

    bignum a;
    bignum_from_int(&a, 3);
    bignum_negate(&a, &ellip_curve.p); // a = -3 mod p

    elliptic_mul(&G, da, &a, &ellip_curve.p, ha);
}

void ecdsa_sign(
        curve *ellip, // curve, IN
        bignum *z, // hash, IN
        bignum *da, // private key, IN
        bignum *r, // r, OUT
        bignum *s // s, OUT
) {
    bignum k;
    bignum zero;
    bignum_from_int(&zero, 0);

    bignum_curve_t ellip_curve;
    ellip_curve_init(&ellip_curve, ellip);
    bignum_mod(z, &ellip_curve.n, z);

    bignum a;
    bignum_from_int(&a, 3);
    bignum_negate(&a, &ellip_curve.p); // a = -3 mod p
    point P, G;
    G.zero_flag = 0;
    bignum_copy(&G.x, &ellip_curve.Gx);
    bignum_copy(&G.y, &ellip_curve.Gy);

kgen:
    bignum_gen_mod(&k, &ellip_curve.n, secure_urand32_rdrand);

    elliptic_mul(&G, &k, &a, &ellip_curve.p, &P); // P = kG
    bignum_mod(&P.x, &ellip_curve.n, r); // r = Px mod n

    bignum tmp, tmp2;
    bignum_mul_mod(r, da, &tmp, &ellip_curve.n); // r * da mod n
    bignum_add_mod(z, &tmp, &tmp2, &ellip_curve.n); // z + r * da mod n
    bignum_reverse(&tmp, &k, &ellip_curve.n); // k ^ -1 mod n

    bignum_mul_mod(&tmp, &tmp2, s, &ellip_curve.n); // k ^ -1 *  (z + r * da) mod n

    if (bignum_cmp(r, &zero) == EQUAL || bignum_cmp(s, &zero) == EQUAL)
        goto kgen;
}

int ecdsa_verify(
        bignum *z, // z, IN
        bignum *r, // r, IN
        bignum *s, // s, IN
        curve *ellip, // curve, IN
        point *ha // HA, IN
) {
    bignum u1, u2, rev_s;

    bignum_curve_t ellip_curve;
    ellip_curve_init(&ellip_curve, ellip);

    point G;
    G.zero_flag = 0;
    bignum_copy(&G.x, &ellip_curve.Gx);
    bignum_copy(&G.y, &ellip_curve.Gy);

    bignum a;
    bignum_from_int(&a, 3);
    bignum_negate(&a, &ellip_curve.p); // a = -3 mod p

    bignum_reverse(&rev_s, s, &ellip_curve.n); // s ^ -1
    bignum_mul_mod(&rev_s, z, &u1, &ellip_curve.n); // u1 = s ^ -1 * z mod n
    bignum_mul_mod(&rev_s, r, &u2, &ellip_curve.n); // u2 = s ^ -1 * r mod n

    point P, uG, uH;
    elliptic_mul(&G, &u1, &a, &ellip_curve.p, &uG); // u1 * G
    elliptic_mul(ha, &u2, &a, &ellip_curve.p, &uH); // u2 * HA
    elliptic_add(&uG, &uH, &P, &a, &ellip_curve.p); // P = u1 * G + u2 * HA

    bignum tmp;
    bignum_mod(&P.x, &ellip_curve.p, &tmp);

    return bignum_cmp(r, &tmp) == EQUAL;
}

void bignum_gen_mod(bignum* k, bignum *n, uint32_t (*rand_func) (void))
{
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        k->array[i] = rand_func();
    bignum temp;
    bignum_copy(&temp, k);
    bignum_mod(&temp, n, k);
}





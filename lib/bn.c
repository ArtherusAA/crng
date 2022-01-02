/*

Big number library - arithmetic on multiple-precision unsigned integers.

This library is an implementation of arithmetic on arbitrarily large integers.

The difference between this and other implementations, is that the data structure
has optimal memory utilization (i.e. a 1024 bit integer takes up 128 bytes RAM),
and all memory is allocated statically: no dynamic allocation for better or worse.

Primary goals are correctness, clarity of code and clean, portable implementation.
Secondary goal is a memory footprint small enough to make it suitable for use in
embedded applications.


The current state is correct functionality and adequate performance.
There may well be room for performance-optimizations and improvements.

*/

#include <inc/stdio.h>
#include <inc/bn.h>
#include <inc/string.h>


/* Functions for shifting number in-place. */
static void _lshift_one_bit(struct bn* a);
static void _rshift_one_bit(struct bn* a);
static void _lshift_word(struct bn* a, int nwords);
static void _rshift_word(struct bn* a, int nwords);



/* Public / Exported functions. */
void bignum_init(struct bn* n)
{
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        n->array[i] = 0;
    }
}


void bignum_from_int(struct bn* n, DTYPE_TMP i)
{
    bignum_init(n);

    /* Endianness issue if machine is not little-endian? */
#ifdef WORD_SIZE
#if (WORD_SIZE == 1)
    n->array[0] = (i & 0x000000ff);
    n->array[1] = (i & 0x0000ff00) >> 8;
    n->array[2] = (i & 0x00ff0000) >> 16;
    n->array[3] = (i & 0xff000000) >> 24;
#elif (WORD_SIZE == 2)
    n->array[0] = (i & 0x0000ffff);
    n->array[1] = (i & 0xffff0000) >> 16;
#elif (WORD_SIZE == 4)
    n->array[0] = i;
    DTYPE_TMP num_32 = 32;
    DTYPE_TMP tmp = i >> num_32; /* bit-shift with U64 operands to force 64-bit results */
    n->array[1] = tmp;
#endif
#endif
}


int bignum_to_int(struct bn* n)
{

    int ret = 0;

    /* Endianness issue if machine is not little-endian? */
#if (WORD_SIZE == 1)
    ret += n->array[0];
    ret += n->array[1] << 8;
    ret += n->array[2] << 16;
    ret += n->array[3] << 24;
#elif (WORD_SIZE == 2)
    ret += n->array[0];
    ret += n->array[1] << 16;
#elif (WORD_SIZE == 4)
    ret += n->array[0];
#endif

    return ret;
}


void bignum_copy(struct bn* n, const struct bn* m)
{
    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        n->array[i] = m->array[i];
}

void bn_mod(struct bn* r, const struct bn* a, const struct bn* b)
{
    struct bn ta,  tb;
    bignum_copy(&ta, a);
    bignum_copy(&tb, b);
    while (bignum_cmp(&ta, &tb) >= 0)
    {
        struct bn temp;
        bignum_sub(&temp, &ta, &tb);
        bignum_copy(&ta, &temp);
    }
}

void bignum_dec(struct bn* n)
{

    DTYPE tmp; /* copy of n */
    DTYPE res;

    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp = n->array[i];
        res = tmp - 1;
        n->array[i] = res;

        if (!(res > tmp))
        {
            break;
        }
    }
}


void bignum_inc(struct bn* n)
{

    DTYPE res;
    DTYPE_TMP tmp; /* copy of n */

    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp = n->array[i];
        res = tmp + 1;
        n->array[i] = res;

        if (res > tmp)
        {
            break;
        }
    }
}


void bignum_add(struct bn* a, struct bn* b, struct bn* c)
{

    DTYPE_TMP tmp;
    int carry = 0;
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp = (DTYPE_TMP)a->array[i] + b->array[i] + carry;
        carry = (tmp > MAX_VAL);
        c->array[i] = (tmp & MAX_VAL);
    }
}


void bignum_sub(struct bn* a, struct bn* b, struct bn* c)
{

    DTYPE_TMP res;
    DTYPE_TMP tmp1;
    DTYPE_TMP tmp2;
    int borrow = 0;
    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        tmp1 = (DTYPE_TMP)a->array[i] + (MAX_VAL + 1); /* + number_base */
        tmp2 = (DTYPE_TMP)b->array[i] + borrow;;
        res = (tmp1 - tmp2);
        c->array[i] = (DTYPE)(res & MAX_VAL); /* "modulo number_base" == "% (number_base - 1)" if number_base is 2^N */
        borrow = (res <= MAX_VAL);
    }
}


void bignum_mul(struct bn* a, struct bn* b, struct bn* c)
{


    struct bn row;
    struct bn tmp;
    int i, j;

    bignum_init(c);

    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        bignum_init(&row);

        for (j = 0; j < BN_ARRAY_SIZE; ++j)
        {
            if (i + j < BN_ARRAY_SIZE)
            {
                bignum_init(&tmp);
                DTYPE_TMP intermediate = ((DTYPE_TMP)a->array[i] * (DTYPE_TMP)b->array[j]);
                bignum_from_int(&tmp, intermediate);
                _lshift_word(&tmp, i + j);
                bignum_add(&tmp, &row, &row);
            }
        }
        bignum_add(c, &row, c);
    }
}


void bignum_div(struct bn* a, struct bn* b, struct bn* c)
{


    struct bn current;
    struct bn denom;
    struct bn tmp;

    bignum_from_int(&current, 1);               // int current = 1;
    bignum_assign(&denom, b);                   // denom = b
    bignum_assign(&tmp, a);                     // tmp   = a

    const DTYPE_TMP half_max = 1 + (DTYPE_TMP)(MAX_VAL / 2);
    bool overflow = false;
    while (bignum_cmp(&denom, a) != LARGER)     // while (denom <= a) {
    {
        if (denom.array[BN_ARRAY_SIZE - 1] >= half_max)
        {
            overflow = true;
            break;
        }
        _lshift_one_bit(&current);                //   current <<= 1;
        _lshift_one_bit(&denom);                  //   denom <<= 1;
    }
    if (!overflow)
    {
        _rshift_one_bit(&denom);                  // denom >>= 1;
        _rshift_one_bit(&current);                // current >>= 1;
    }
    bignum_init(c);                             // int answer = 0;

    while (!bignum_is_zero(&current))           // while (current != 0)
    {
        if (bignum_cmp(&tmp, &denom) != SMALLER)  //   if (dividend >= denom)
        {
            bignum_sub(&tmp, &denom, &tmp);         //     dividend -= denom;
            bignum_or(c, &current, c);              //     answer |= current;
        }
        _rshift_one_bit(&current);                //   current >>= 1;
        _rshift_one_bit(&denom);                  //   denom >>= 1;
    }                                           // return answer;
}


void bignum_lshift(struct bn* a, struct bn* b, int nbits)
{


    bignum_assign(b, a);
    /* Handle shift in multiples of word-size */
    const int nbits_pr_word = (WORD_SIZE * 8);
    int nwords = nbits / nbits_pr_word;
    if (nwords != 0)
    {
        _lshift_word(b, nwords);
        nbits -= (nwords * nbits_pr_word);
    }

    if (nbits != 0)
    {
        int i;
        for (i = (BN_ARRAY_SIZE - 1); i > 0; --i)
        {
            b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * WORD_SIZE) - nbits));
        }
        b->array[i] <<= nbits;
    }
}


void bignum_rshift(struct bn* a, struct bn* b, int nbits)
{


    bignum_assign(b, a);
    /* Handle shift in multiples of word-size */
    const int nbits_pr_word = (WORD_SIZE * 8);
    int nwords = nbits / nbits_pr_word;
    if (nwords != 0)
    {
        _rshift_word(b, nwords);
        nbits -= (nwords * nbits_pr_word);
    }

    if (nbits != 0)
    {
        int i;
        for (i = 0; i < (BN_ARRAY_SIZE - 1); ++i)
        {
            b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * WORD_SIZE) - nbits));
        }
        b->array[i] >>= nbits;
    }

}


void bignum_mod(struct bn* a, struct bn* b, struct bn* c)
{
    /*
      Take divmod and throw away div part
    */


    struct bn tmp;

    bignum_divmod(a,b,&tmp,c);
}

void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d)
{
    /*
      Puts a%b in d
      and a/b in c

      mod(a,b) = a - ((a / b) * b)

      example:
        mod(8, 3) = 8 - ((8 / 3) * 3) = 2
    */

    struct bn tmp;

    /* c = (a / b) */
    bignum_div(a, b, c);

    /* tmp = (c * b) */
    bignum_mul(c, b, &tmp);

    /* c = a - tmp */
    bignum_sub(a, &tmp, d);
}


void bignum_and(struct bn* a, struct bn* b, struct bn* c)
{


    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        c->array[i] = (a->array[i] & b->array[i]);
    }
}


void bignum_or(struct bn* a, struct bn* b, struct bn* c)
{


    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        c->array[i] = (a->array[i] | b->array[i]);
    }
}


void bignum_xor(struct bn* a, struct bn* b, struct bn* c)
{

    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        c->array[i] = (a->array[i] ^ b->array[i]);
    }
}


int bignum_cmp(struct bn* a, struct bn* b)
{


    int i = BN_ARRAY_SIZE;
    do
    {
        i -= 1; /* Decrement first, to start with last array element */
        if (a->array[i] > b->array[i])
        {
            return LARGER;
        }
        else if (a->array[i] < b->array[i])
        {
            return SMALLER;
        }
    }
    while (i != 0);

    return EQUAL;
}


int bignum_is_zero(struct bn* n)
{

    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        if (n->array[i])
        {
            return 0;
        }
    }

    return 1;
}


void bignum_pow(struct bn* a, struct bn* b, struct bn* c)
{

    struct bn tmp;

    bignum_init(c);

    if (bignum_cmp(b, c) == EQUAL)
    {
        /* Return 1 when exponent is 0 -- n^0 = 1 */
        bignum_inc(c);
    }
    else
    {
        struct bn bcopy;
        bignum_assign(&bcopy, b);

        /* Copy a -> tmp */
        bignum_assign(&tmp, a);

        bignum_dec(&bcopy);

        /* Begin summing products: */
        while (!bignum_is_zero(&bcopy))
        {

            /* c = tmp * tmp */
            bignum_mul(&tmp, a, c);
            /* Decrement b by one */
            bignum_dec(&bcopy);

            bignum_assign(&tmp, c);
        }

        /* c = tmp */
        bignum_assign(c, &tmp);
    }
}

void bignum_isqrt(struct bn *a, struct bn* b)
{


    struct bn low, high, mid, tmp;

    bignum_init(&low);
    bignum_assign(&high, a);
    bignum_rshift(&high, &mid, 1);
    bignum_inc(&mid);

    while (bignum_cmp(&high, &low) > 0)
    {
        bignum_mul(&mid, &mid, &tmp);
        if (bignum_cmp(&tmp, a) > 0)
        {
            bignum_assign(&high, &mid);
            bignum_dec(&high);
        }
        else
        {
            bignum_assign(&low, &mid);
        }
        bignum_sub(&high,&low,&mid);
        _rshift_one_bit(&mid);
        bignum_add(&low,&mid,&mid);
        bignum_inc(&mid);
    }
    bignum_assign(b,&low);
}


void bignum_assign(struct bn* dst, struct bn* src)
{


    int i;
    for (i = 0; i < BN_ARRAY_SIZE; ++i)
    {
        dst->array[i] = src->array[i];
    }
}


/* Private / Static functions. */
static void _rshift_word(struct bn* a, int nwords)
{
    /* Naive method: */


    int i;
    if (nwords >= BN_ARRAY_SIZE)
    {
        for (i = 0; i < BN_ARRAY_SIZE; ++i)
        {
            a->array[i] = 0;
        }
        return;
    }

    for (i = 0; i < BN_ARRAY_SIZE - nwords; ++i)
    {
        a->array[i] = a->array[i + nwords];
    }
    for (; i < BN_ARRAY_SIZE; ++i)
    {
        a->array[i] = 0;
    }
}


static void _lshift_word(struct bn* a, int nwords)
{

    int i;
    /* Shift whole words */
    for (i = (BN_ARRAY_SIZE - 1); i >= nwords; --i)
    {
        a->array[i] = a->array[i - nwords];
    }
    /* Zero pad shifted words. */
    for (; i >= 0; --i)
    {
        a->array[i] = 0;
    }
}


static void _lshift_one_bit(struct bn* a)
{

    int i;
    for (i = (BN_ARRAY_SIZE - 1); i > 0; --i)
    {
        a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * WORD_SIZE) - 1));
    }
    a->array[0] <<= 1;
}


static void _rshift_one_bit(struct bn* a)
{

    int i;
    for (i = 0; i < (BN_ARRAY_SIZE - 1); ++i)
    {
        a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * WORD_SIZE) - 1));
    }
    a->array[BN_ARRAY_SIZE - 1] >>= 1;
}




//ADDITION

void bignum_from_str(bignum* n, const char* src, int64_t len)
{
    //p = 1;
    bignum p;
    bignum_from_int(n, 0);
    bignum_from_int(&p, 1);
    for (int64_t i = len - 2; i >= 0; --i)
    //for (int64_t i = 0; i < len - 1; ++i)
    {
        //n += src[i] * p;
        //p *= 16;
        bignum temp, temp2;

        bignum_from_int(&temp, (src[i] - '0'));

        bignum_mul(&temp, &p, &temp2);
        bignum_copy(&temp, n);
        bignum_add(&temp, &temp2, n);

        bignum_from_int(&temp, 10);
        bignum_copy(&temp2, &p);
        bignum_mul(&temp, &temp2, &p);
    }
}

void bignum_from_str_dex(bignum* n, const char* src, int64_t len)
{
    //p = 1;
    bignum p;
    bignum_from_int(n, 0);
    bignum_from_int(&p, 1);
    for (int64_t i = len - 2; i >= 0; --i)
    //for (int64_t i = 0; i < len - 1; ++i)
    {
        //n += src[i] * p;
        //p *= 16;
        bignum temp, temp2;
        switch (src[i])
        {
        case 'F':
        case 'f':
            bignum_from_int(&temp, (15));
            break;
        case 'E':
        case 'e':
            bignum_from_int(&temp, (14));
            break;
        case 'D':
        case 'd':
            bignum_from_int(&temp, (13));
            break;
        case 'C':
        case 'c':
            bignum_from_int(&temp, (12));
            break;
        case 'B':
        case 'b':
            bignum_from_int(&temp, (11));
            break;
        case 'A':
        case 'a':
            bignum_from_int(&temp, (10));
            break;
        default:
            bignum_from_int(&temp, (src[i] - '0'));
            break;
        }
        bignum_mul(&temp, &p, &temp2);
        bignum_copy(&temp, n);
        bignum_add(&temp, &temp2, n);

        bignum_from_int(&temp, 16);
        bignum_copy(&temp2, &p);
        bignum_mul(&temp, &temp2, &p);
    }
}

void bignum_euc(const bignum* a, bignum* c, bignum* b, bignum* d)
{
    bignum u, v, c1, c2, d1, d2;
    bignum_copy(&u, a);
    bignum_copy(&v, b);
    if (bignum_cmp(&u, &v) == SMALLER)
    {
        bignum_from_int(&c2, 0);
        bignum_from_int(&c1, 1);
        bignum_from_int(&d2, 1);
        bignum_from_int(&d1, 0);
    }
    else
    {
        bignum_from_int(&c2, 1);
        bignum_from_int(&c1, 0);
        bignum_from_int(&d2, 0);
        bignum_from_int(&d1, 1);
    }
    bignum r;
    bignum_from_int(&r, 0);
    bignum zero;
    bignum_from_int(&zero, 0);
    while (bignum_cmp(&v, &zero) != EQUAL)
    {
        //r = u % v;
        bignum_mod(&u, &v, &r);
        //int q = (u-r) / v;
        bignum q, temp;
        bignum_sub(&u, &r, &temp);
        bignum_div(&temp, &v, &q);
        // c = c2 - q * c1;
        bignum_mul(&q, &c1, &temp);
        bignum_sub(&c2, &temp, c);
        // d = d2 - q * d1;
        bignum_mul(&q, &d1, &temp);
        bignum_sub(&d2, &temp, d);
        bignum_copy(&u, &v); bignum_copy(&v, &r);
        bignum_copy(&c2, &c1); bignum_copy(&c1, c);
        bignum_copy(&d2, &d1); bignum_copy(&d1, d);
    }
    bignum_copy(c, &d2);
    bignum_copy(d, &c2);
}


// x * b = 1 (mod m);
void bignum_reverse(bignum* x, bignum* b, bignum* m)
{
    bignum trash;
    bignum_euc(b, x, m, &trash);
    if (bignum_cmp(x, m) == LARGER)
    {
        bignum temp;
        bignum_add(x, m, &temp);
        bignum_copy(x, &temp);
    }
}

void bignum_negate(bignum* x, bignum* n) {
    bignum zero;
    bignum_from_int(&zero, 0);
    if (bignum_cmp(x, &zero) != EQUAL) {
        bignum tmp;
        bignum_sub(n, x, &tmp);
        bignum_copy(x, &tmp);
    }
}

void bignum_add_mod(bignum* a, bignum* b, bignum* c, bignum* n) {
    bignum tmp;
    bignum_add(a, b, &tmp);
    bignum_mod(&tmp, n, c);
}

void bignum_sub_mod(bignum* a, bignum* b, bignum* c, bignum* n) {
    bignum tmp, tmp2;
    bignum_copy(&tmp2, b);
    bignum_negate(&tmp2, n);
    bignum_add(a, &tmp2, &tmp);
    bignum_mod(&tmp, n, c);
}

void bignum_mul_mod(bignum* a, bignum* b, bignum* c, bignum* n) {
    bignum tmp;
    bignum_mul(a, b, &tmp);
    bignum_mod(&tmp, n, c);
}





void convert_from_md5_to_bignum(bignum* dst, const char* src){
    uint32_t a1, a2, a3, a4;
    memcpy((void*)(&a1), (void*)(src),                        sizeof(uint32_t));
    memcpy((void*)(&a2), (void*)(src +     sizeof(uint32_t)), sizeof(uint32_t));
    memcpy((void*)(&a3), (void*)(src + 2 * sizeof(uint32_t)), sizeof(uint32_t));
    memcpy((void*)(&a4), (void*)(src + 3 * sizeof(uint32_t)), sizeof(uint32_t));
    bignum_from_int(dst, 0);
    dst->array[0] = a1;
    dst->array[1] = a2;
    dst->array[2] = a3;
    dst->array[3] = a4;
}

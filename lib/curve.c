#include <inc/curve.h>

curve test_curve =
        {
                "97",
                "1488",
                "aboba",
                "aboba",
                "3",
                "11",
                "A"
};

curve p_192 = {
        "6277101735386680763835789423207666416083908700390324961279", //p
        "6277101735386680763835789423176059013767194773182842284081", //n
        "3045ae6fc8422f64ed579528d38120eae12196d5",                   //SEED
        "3099d2bbbfcb2538542dcd5fb078b6ef5f3d6fe2c745de65",           //c
        "64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1",           //b
        "188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",           //Gx
        "07192b95ffc8da78631011ed6b24cdd573f977a11e794811"            //Gy
};

curve p_224 = {
        "26959946667150639794667015087019630673557916260026308143510066298881", //p
        "26959946667150639794667015087019625940457807714424391721682722368061", //n
        "bd71344799d5c7fcdc45b59fa3b9ab8f6a948bc5",                             //SEED
        "5b056c7e11dd68f40469ee7f3c7a7d74f7d121116506d031218291fb",             //c
        "b4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4",             //b
        "b70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",             //Gx
        "bd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34"              //Gy
};

curve p_256 = {
        "115792089210356248762697446949407573530086143415290314195533631308867097853951", //p
        "115792089210356248762697446949407573529996955224135760342422259061068512044369", //n
        "c49d360886e704936a6678e1139d26b7819f7e90",                                       //SEED
        "7efba1662985be9403cb055c75d4f7e0ce8d84a9c5114abcaf3177680104fa0d",               //c
        "5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",               //b
        "6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",               //Gx
        "4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5"                //Gy
};

curve p_384 = {
        "3940200619639447921227904010014361380507973927046544666794"
        "8293404245721771496870329047266088258938001861606973112319", //p

        "3940200619639447921227904010014361380507973927046544666794"
        "6905279627659399113263569398956308152294913554433653942643", //n

        "a335926aa319a27a1d00896a6773a4827acdac73",                   //SEED

        "79d1e655f868f02fff48dcdee14151ddb80643c1406d0ca1"
        "0dfe6fc52009540a495e8042ea5f744f6e184667cc722483",           //c

        "b3312fa7e23ee7e4988e056be3f82d19181d9c6efe814112"
        "0314088f5013875ac656398d8a2ed19d2a85c8edd3ec2aef",           //b

        "aa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b98"
        "59f741e082542a385502f25dbf55296c3a545e3872760ab7",           //Gx

        "3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147c"
        "e9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f"            //Gy
};


void neg_elliptic_point(point* src, bignum* p, point* dst)
{
    bignum_copy(&dst->x, &src->x);
    bignum_copy(&dst->y, p);
    bignum temp;
    bignum_sub(&dst->y, &src->y, &temp);
    bignum_copy(&dst->y, &temp);
}

int elliptic_point_eq(point* p1, point* p2)
{
    if (p1->zero_flag != p2->zero_flag)
        return 0;
    if (bignum_cmp(&p1->x, &p2->x) != EQUAL)
        return 0;
    if (bignum_cmp(&p1->y, &p2->y) != EQUAL)
        return 0;
    return 1;
}

void elliptic_add(point* p1, point* p2, point* p3, bignum* a, bignum* p)
{
    bignum neg_y;
    bignum_copy(&neg_y, &p1->y);
    bignum_negate(&neg_y, p);
    if (p1->zero_flag == 1) {
        bignum_copy(&p3->x, &p2->x);
        bignum_copy(&p3->y, &p2->y);
        p3->zero_flag = p2->zero_flag;
    }
    else if (p2->zero_flag == 1) {
        bignum_copy(&p3->x, &p1->x);
        bignum_copy(&p3->y, &p1->y);
        p3->zero_flag = p1->zero_flag;
    }
    else if (bignum_cmp(&p1->x, &p2->x) == EQUAL && bignum_cmp(&neg_y, &p2->y) == EQUAL) {
        p3->zero_flag = 1;
    }
    else {
        p3->zero_flag = 0;
        bignum m;
        if (bignum_cmp(&p1->x, &p2->x) == EQUAL && bignum_cmp(&p1->y, &p2->y) == EQUAL) {
            bignum tmp, tmp2, tree, two;
            bignum_from_int(&tree, 3);
            bignum_from_int(&two, 2);
            bignum_mul_mod(&p1->x, &p1->x, &tmp, p); // Px ^ 2
            bignum_mul_mod(&tmp, &tree, &tmp2, p); // 3 Px ^ 2
            bignum_add_mod(&tmp2, a, &m, p); // (3 Px ^ 2 + a)
            bignum_mul_mod(&two, &p1->y, &tmp, p); // 2 Py
            bignum_reverse(&tmp2, &tmp, p); // (2 Py) ^ -1
            bignum_mul_mod(&m, &tmp2, &tmp, p); // (3 Px ^ 2 + a) (2 Py) ^ -1
            bignum_copy(&m, &tmp);
        }
        else {
            bignum tmp, tmp2;
            bignum_copy(&tmp, &p2->y);
            bignum_copy(&tmp2, &p2->x);
            bignum_negate(&tmp, p); // -Qy
            bignum_negate(&tmp2, p); // -Qx
            bignum_add_mod(&p1->y, &tmp, &m, p); // Py - Qy
            bignum_add_mod(&p1->x, &tmp2, &tmp, p); // Px - Qx
            bignum_reverse(&tmp2, &tmp, p); // (Px - Qx) ^ -1
            bignum_mul_mod(&m, &tmp2, &tmp, p);  // (Py - Qy) (Px - Qx) ^ -1
            bignum_copy(&m, &tmp);
        }
        bignum neg_px, neg_qx, tmp, tmp2;
        bignum_copy(&neg_px, &p1->x); // Px
        bignum_copy(&neg_qx, &p2->x); // Qx
        bignum_negate(&neg_px, p); // -Px
        bignum_negate(&neg_qx, p); // -Qx
        bignum_mul_mod(&m, &m, &tmp, p); // m ^ 2
        bignum_add_mod(&tmp, &neg_px, &tmp2, p); // m ^ 2 - Px
        bignum_add_mod(&tmp2, &neg_qx, &p3->x, p); // m ^ 2 - Px - Qx

        bignum_add_mod(&p3->x, &neg_px, &tmp, p); // Rx - Px
        bignum_mul_mod(&m, &tmp, &tmp2, p); // m * (Rx - Px)
        bignum_add_mod(&p1->y, &tmp2, &p3->y, p); // Py + m * (Rx - Px)
        bignum_negate(&p3->y, p); // -(Py + m * (Rx - Px))
    }
}




int inline binary(bignum* k, uint64_t i)
{

    //uint64_t result = (*k & (1ULL << i));
    //*k -= result;
    //return (result) && (1);

    bignum temp, temp2;
    bignum_from_int(&temp, 1);
    bignum_lshift(&temp, &temp2, i);
    bignum_and(k, &temp2, &temp);

    bignum_sub(k, &temp, &temp2);
    bignum_copy(k, &temp2);

    for (int i = 0; i < BN_ARRAY_SIZE; ++i)
        if (temp.array[i] != 0)
            return 1;
    return 0;
}

void elliptic_mul(point* x, bignum* kt, bignum* a, bignum* pt, point* result)
{
    bignum_from_int(&result->x, 0);
    bignum_from_int(&result->y, 0);
    result->zero_flag = 1;

    point temp2;
    point temp;
    point temp3;
    bignum_copy(&temp3.x, &x->x);       //temp3  = x;
    bignum_copy(&temp3.y, &x->y);
    temp3.zero_flag = x->zero_flag;

    bignum k;
    bignum_copy(&k, kt);

    bignum p;
    bignum_copy(&p, pt);

    for (int i = 0; !bignum_is_zero(&k); ++i)
    {

        if (binary(&k, i))
        {
            //result += temp3;
            bignum_copy(&temp.x, &temp3.x);    //temp  = temp3;
            bignum_copy(&temp.y, &temp3.y);
            temp.zero_flag = temp3.zero_flag;
            bignum_copy(&temp2.x, &result->x); //temp2 = result;
            bignum_copy(&temp2.y, &result->y);
            temp2.zero_flag = result->zero_flag;
            elliptic_add(&temp, &temp2, result, a, &p); //result = result + temp3

            /*bignum_to_string(&result->x, out_buf, 1024);
            std::cout << "tempx: " << out_buf << '\n';
            bignum_to_string(&result->y, out_buf, 1024);
            std::cout << "tempy: " << out_buf << '\n';*/

        }
        //temp3 = temp3 + temp3;
        bignum_copy(&temp.x, &temp3.x);       //temp  = temp3;
        bignum_copy(&temp.y, &temp3.y);
        temp.zero_flag = temp3.zero_flag;
        bignum_copy(&temp2.x, &temp3.x);       //temp2  = temp3;
        bignum_copy(&temp2.y, &temp3.y);
        temp2.zero_flag = temp3.zero_flag;
        elliptic_add(&temp, &temp2, &temp3, a, &p);   //temp3 = temp + temp2;
    }


}

void elliptic_init_zero(point* p)
{
    p->zero_flag = 0;
}

void ellip_curve_init(bignum_curve_t* ellip_curve, curve* ellip)
{
    bignum_from_str(&ellip_curve->p, ellip->p, strlen(ellip->p) + 1);
    bignum_from_str(&ellip_curve->n, ellip->n, strlen(ellip->n) + 1);
    bignum_from_str_dex(&ellip_curve->b, ellip->b, strlen(ellip->b) + 1);
    bignum_from_str_dex(&ellip_curve->c, ellip->c, strlen(ellip->c) + 1);
    bignum_from_str_dex(&ellip_curve->SEED, ellip->SEED, strlen(ellip->SEED) + 1);
    bignum_from_str_dex(&ellip_curve->Gx, ellip->Gx, strlen(ellip->Gx) + 1);
    bignum_from_str_dex(&ellip_curve->Gy, ellip->Gy, strlen(ellip->Gy) + 1);
}
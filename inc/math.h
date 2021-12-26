//
// Created by daniil on 26.12.2021.
//

#ifndef CRNG_MATH_H
#define CRNG_MATH_H

#include <stdint.h>
#include <stddef.h>

typedef void(*ptr_integral_func)(double x, int param_count, const double *const_params, double *res);

///Culc abs value
extern void abs_func(double x, double *res);

///Culc sqrt
extern void sqrt_func(double x, double *res);

///Numerical trapezodial integration of {func} from {from} to {to}
extern void integral_func(double from, double to, double eps, ptr_integral_func func, int param_count, const double *const_params, double *res);

///Calc Gamma(a, z)
extern void gamma_func(double a, double z, double *res);

///Calc log2(x)
extern void log2_func(double x, double *res);

///Calc log(x)
extern void log_func(double x, double *res);

///Calc log(1 + eps)
extern void log_1_eps_func(double eps, double *res);

///Calc exp(eps) | abs(eps) less 1.0
extern void exp_eps_func(double eps, double *res);

///Calc x^n
extern void pow_n_func(double x, int32_t n, double *res);

///Calc e^n
extern void exp_n_func(int32_t n, double *res);

///Calc e^st
extern void exp_func(double st, double *res);

///Calc erfc
extern void erfc_func(double x, double *res);

#endif // CRNG_MATH_H

//
// Created by daniil on 26.12.2021.
//
#include <inc/math.h>

void abs_func(double x, double *res) {
    *res = (x > 0) ? x : -x;
}

//log(1 + x) ~= x - x^2/2 + x^3/3 - x^4/4
void log_1_eps_func(double eps, double *res) {
    double x = eps;
    double x2 = x * x;
    double x3 = x2 * x;
    double x4 = x2 * x2;
    *res = x - x2 / 2 + x3 / 3 - x4 / 4;
}

void log2_func(double x, double *res) {
    // x>0
    static _Bool first = 1;
#define precision 12
#define size (1 << precision)
    static double log_table[size];
    static double const c_log2 = 1.442695040888963407359924681;
    if (first) {
        //for example: log(1 + 2/4096) = log((4097/4096) * (4098/4097)) = log(1 + 1/4096) + log(1 + 1/4097);
        // log(1 + k/size) = pred_log + log(1 + 1/(size + k - 1));  | pred_log = log(1 + (k-1)/size);
        log_table[0] = 0;
        double pred_log = log_table[0];
        for (int k = 1; k < size; k++) { //k_1 = k - 1
            double eps = 1 / ((double)k + size);
            double log_1;
            log_1_eps_func(eps, &log_1);
            pred_log = pred_log + log_1;
            log_table[k] = pred_log * c_log2;
        }
        first = 0;
    }

    uint64_t t = *(uint64_t *)&x;
    int exp = (t >> 52) - 0x3ff;
    int mantissa = (t >> (52 - precision)) & (size - 1);
    *res = exp + log_table[mantissa];
#undef size
#undef precision
}

void log_func(double x, double *res) {
    static double const c_log = 0.6931471805599453;
    double log2;
    log2_func(x, &log2);
    *res = c_log * log2;
}

void exp_eps_func(double eps, double *res) {
    _Bool less_zero = (eps < 0);
    if (less_zero)eps = -eps;

    double ret = 1.0;

    if (eps > 0.5) {
        ret *= 1.64872127070012814684865;//eps(0.5)
        eps -= 0.5;
    }
    double x = eps;
    double x2 = x * x;
    double x3 = x2 * x;
    double x4 = x2 * x2;
    double x5 = x4 * x;
    ret *= 1.0 + x + x2 / 2.0 + x3 / 6.0 + x4 / 24.0 + x5 / 120.0;

    *res = (less_zero) ? (1.0 / ret) : ret;
}

void pow_n_func(double x, int32_t n, double *res) {
    _Bool less_zero = (n < 0);
    if (less_zero)n = -n;
    double ret = 1;
    for (; n; n >>= 1, x *= x)if (n & 1)ret *= x;
    *res = (less_zero) ? (1.0 / ret) : ret;
}

void exp_n_func(int32_t n, double *res) {
    double power;
    pow_n_func(2.71828182845904523536, n, &power);
    *res = power;
}

void exp_func(double st, double *res) {
    _Bool less_zero = (st < 0);
    if (less_zero)st = -st;

    int32_t entire_to = (int32_t)st;
    double eps = st - entire_to;
    //exp(n + eps) = exp(n)*exp(eps)
    double entire_exp;
    exp_n_func(entire_to, &entire_exp);
    double eps_exp;
    exp_eps_func(eps, &eps_exp);

    double ret = entire_exp * eps_exp;

    *res = (less_zero) ? (1.0 / ret) : ret;
}

void sqrt_func(double x, double *res) {
    if (x < 0)
        *res = x;
    if (x == 1.0 || x == 0.0)
        *res = x;

    double from = 0;
    double to = x;
    double ret = 0;
    _Bool less = (x <= 1);
    if (less) {
        ret = x + (1 - x) / 2;
        from = x;
        to = 1;
    } else {
        ret = 1.0 + (x - 1) / 2;
        from = 1;
    }

    const int enough = 32;

    for (int k = 0; k < enough; k++) {
        double z = ret * ret;
        if (z == x) break;
        if (z < x) from = ret;
        else to = ret;
        ret = (from + to) / 2;
    }
    *res = ret;
}

void integral_func(double from, double to, double eps, ptr_integral_func func, int param_count, const double *const_params, double *res) {
    if (!func)
        *res = 0;
    if (eps == 0)eps = 1E-5;// 1E-6;
    _Bool sg = 0;
    if (from > to) {
        double temp = to;
        to = from;
        from = temp;
        sg = 1;
    }

    double I = 0;
    double I_back = 0;

    uint64_t n = 16;
    double delta = to - from;
    double func_in_from, func_in_to;
    func(from, param_count, const_params, &func_in_from);
    func(to, param_count, const_params, &func_in_to);

    uint32_t max_do = 20;

    do {
        max_do--;
        n *= 2;
        I_back = I;
        I = 0;
        double delta_n = delta / n;
        double delta_n_2 = delta / (2 * n);
        double f_i_1 = func_in_from;//f(x[i-1])
        for (uint64_t i = 1; i < n; i++) {
            double x_i = from + i * delta_n;
            double f_i;
            func(x_i, param_count, const_params, &f_i);
            I += (f_i + f_i_1) * delta_n_2;
            f_i_1 = f_i;
        }
        I += (func_in_to - f_i_1) * delta_n_2;
    } while (((I - I_back) > eps) && (max_do != 0));
    *res = sg ? -I : I;
}

void gamma_def(double x, int len, const double *a_ptr, double *res) {
    if (len < 1)
        *res = 0;
    double a = a_ptr[0];
    double log_x;
    log_func(x, &log_x);
    double st = log_x * (a - 1) - x;
    if (st < -20)
        *res = 0;
    exp_func(st, res);
}

void gamma_func(double a, double z, double *res) {
    double from = z, to = 5 + a * 5;
    if (-1.0 < from && from < 0.0)
        from = 0.0;
    if (from > to)
        *res = 0;
    integral_func(from, to, 0, gamma_def, 1, &a, res);
}

void erf_def(double x, int not_used, const double *no_used, double *res) {
    if (x > 6)
        *res = 0.0;
    exp_func(-(x * x), res);
}

void erf_func(double x, double *res) {
    if (x > 3.5)
        *res = 1.0;
    double loc_res;
    integral_func(0, x, 0, erf_def, 0, NULL, &loc_res);
    double ret = 1.1283791670955125738961589 * loc_res;
    if (ret > 1.0)
        ret = 1.0;
    *res = ret;
}

void erfc_func(double x, double *res) {
    double loc_res;
    erf_func(x, &loc_res);
    *res = 1.0 - loc_res;
}

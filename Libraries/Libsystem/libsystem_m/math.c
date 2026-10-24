/*
 * Minimal libm for ravynOS
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * IMPORTANT: implementations in this file must never rely on a compiler
 * builtin that the compiler lowers into a call to the *same* function
 * (e.g. "__builtin_exp" -> "call exp").  Doing that turns every entry point
 * into an infinitely recursive function.  All real work is therefore done by
 * the private __rm_* helpers below, using only bit manipulation, inline
 * assembly and self-contained polynomial / series evaluations.
 */

#include <math.h>
#include <errno.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif

/* ------------------------------------------------------------------ bits */

typedef union { double d; uint64_t u; } __rm_b64;
typedef union { float  f; uint32_t u; } __rm_b32;

static inline uint64_t __rm_d2u(double __d) { __rm_b64 __b; __b.d = __d; return __b.u; }
static inline double   __rm_u2d(uint64_t __u) { __rm_b64 __b; __b.u = __u; return __b.d; }
static inline uint32_t __rm_f2u(float __f) { __rm_b32 __b; __b.f = __f; return __b.u; }
static inline float    __rm_u2f(uint32_t __u) { __rm_b32 __b; __b.u = __u; return __b.f; }

#define __RM_SIGN 0x8000000000000000ULL
#define __RM_EXPM 0x7ff0000000000000ULL
#define __RM_ABSBITS(__u) ((__u) & ~__RM_SIGN)
#define __RM_INF  (__rm_u2d(__RM_EXPM))
#define __RM_NAN  (__rm_u2d(0x7ff8000000000000ULL))
#define __RM_NANF (__rm_u2f(0x7fc00000U))

static inline int __rm_isnan(double __x) { return __RM_ABSBITS(__rm_d2u(__x)) > __RM_EXPM; }
static inline int __rm_isinf(double __x) { return __RM_ABSBITS(__rm_d2u(__x)) == __RM_EXPM; }
static inline int __rm_isfinite(double __x) { return (__rm_d2u(__x) & __RM_EXPM) != __RM_EXPM; }
static inline double __rm_fabs(double __x) { return __rm_u2d(__RM_ABSBITS(__rm_d2u(__x))); }
static inline double __rm_copysign(double __x, double __y) {
    return __rm_u2d((__rm_d2u(__x) & ~__RM_SIGN) | (__rm_d2u(__y) & __RM_SIGN));
}
static inline double __rm_sqrt(double __x) {
    double __r;
    __asm__ ("sqrtsd %1, %0" : "=x"(__r) : "x"(__x));
    return __r;
}

/* forward declaration: used by the trig argument reduction below */
static double __rm_fmod(double __x, double __y);

/* ------------------------------------------------------------- constants */

static const double __rm_LN2     = 6.93147180559945309417e-01;
static const double __rm_LN2HI   = 6.93147180369123816490e-01;
static const double __rm_LN2LO   = 1.90821492927058770002e-10;
static const double __rm_INVLN2  = 1.44269504088896338700e+00;
static const double __rm_LN10    = 2.30258509299404568402e+00;
static const double __rm_INVLN10 = 4.34294481903251827651e-01;
static const double __rm_LOG2E   = 1.44269504088896340736e+00;
static const double __rm_2OPI    = 6.36619772367581382433e-01;
static const double __rm_SQRT1_2 = 7.07106781186547524401e-01;

/* -------------------------------------------------- rounding primitives */

/* truncate toward zero (exact for every finite double) */
static double __rm_trunc(double __x) {
    uint64_t __u = __rm_d2u(__x);
    int __e = (int)((__u >> 52) & 0x7ff) - 1023;
    if (__e < 0) return __rm_copysign(0.0, __x);
    if (__e >= 52) return __x;
    return __rm_u2d(__u & ~((1ULL << (52 - __e)) - 1));
}
static double __rm_floor(double __x) {
    double __t = __rm_trunc(__x);
    return (__t > __x) ? __t - 1.0 : __t;
}
static double __rm_ceil(double __x) {
    double __t = __rm_trunc(__x);
    return (__t < __x) ? __t + 1.0 : __t;
}
/* round to nearest, ties to even */
static double __rm_rint(double __x) {
    double __t = __rm_trunc(__x);
    double __f = __x - __t;
    if (__f > 0.5 || (__f == 0.5 && (((long long)__t) & 1)))
        __t += 1.0;
    else if (__f < -0.5 || (__f == -0.5 && (((long long)__t) & 1)))
        __t -= 1.0;
    return __t;
}
/* round to nearest, ties away from zero */
static double __rm_round(double __x) {
    double __t = __rm_trunc(__x);
    double __f = __x - __t;
    if (__f >= 0.5) __t += 1.0;
    else if (__f <= -0.5) __t -= 1.0;
    return __t;
}
/* x * 2^__n */
static double __rm_scalbn(double __x, int __n) {
    if (__x == 0.0 || !__rm_isfinite(__x) || __n == 0) return __x;
    while (__n > 1023) {
        __x *= __rm_u2d((uint64_t)2046 << 52);
        __n -= 1023;
        if (!__rm_isfinite(__x)) return __x;
    }
    while (__n < -1022) {
        __x *= 0x1p-1022;
        __n += 1022;
        if (__x == 0.0) return __x;
    }
    return __x * __rm_u2d((uint64_t)(1023 + __n) << 52);
}
static double __rm_frexp(double __x, int *__e) {
    uint64_t __u = __rm_d2u(__x);
    int __ex = (int)((__u >> 52) & 0x7ff);
    if (__ex == 0) {
        if (__x == 0.0) { *__e = 0; return __x; }
        __x *= 18014398509481984.0;              /* 2^54 */
        __u = __rm_d2u(__x);
        __ex = (int)((__u >> 52) & 0x7ff) - 54;
    } else if (__ex == 0x7ff) {
        *__e = 0;
        return __x;
    }
    *__e = __ex - 0x3fe;
    return __rm_u2d((__u & 0x800fffffffffffffULL) | 0x3fe0000000000000ULL);
}

/* ------------------------------------------------------- exp / log / pow */

static double __rm_exp(double __x) {
    double __k, __r, __t, __s;
    int __i;
    if (__rm_isnan(__x)) return __x;
    if (__x > 709.782712893384) return __RM_INF;
    if (__x < -745.1332191019411) return 0.0;
    __k = __rm_rint(__x * __rm_INVLN2);
    __r = (__x - __k * __rm_LN2HI) - __k * __rm_LN2LO;
    __t = 1.0;
    __s = 1.0;
    for (__i = 1; __i <= 14; __i++) {
        __t *= __r / (double)__i;
        __s += __t;
    }
    return __rm_scalbn(__s, (int)__k);
}

static double __rm_log(double __x) {
    int __e = 0, __i;
    double __m, __s, __s2, __term, __acc;
    if (__rm_isnan(__x)) return __x;
    if (__x < 0.0) return __RM_NAN;
    if (__x == 0.0) return -__RM_INF;
    if (__rm_isinf(__x)) return __x;
    __m = __rm_frexp(__x, &__e);                 /* m in [0.5,1) */
    if (__m < __rm_SQRT1_2) { __m += __m; __e -= 1; }   /* m in [sqrt(1/2), sqrt(2)) */
    __s = (__m - 1.0) / (__m + 1.0);             /* |s| <= 0.1716 */
    __s2 = __s * __s;
    __term = __s;
    __acc = 0.0;
    for (__i = 1; __i <= 12; __i++) {
        __acc += __term / (double)(2 * __i - 1);
        __term *= __s2;
    }
    return 2.0 * __acc + (double)__e * __rm_LN2;
}

static double __rm_pow(double __x, double __y) {
    int __neg = 0;
    double __r, __yi;
    if (__y == 0.0) return 1.0;
    if (__rm_isnan(__x) || __rm_isnan(__y)) return __RM_NAN;
    if (__x == 1.0) return 1.0;
    if (__y == 1.0) return __x;
    if (__y == 2.0) return __x * __x;
    if (__rm_isinf(__y)) {
        double __a = __rm_fabs(__x);
        if (__a == 1.0) return 1.0;
        return ((__a > 1.0) == (__y > 0.0)) ? __RM_INF : 0.0;
    }
    if (__x == 0.0) return (__y > 0.0) ? 0.0 : __RM_INF;
    if (__rm_isinf(__x)) {
        if (__y > 0.0) return __RM_INF;
        return 0.0;
    }
    if (__y == 0.5 && __x > 0.0) return __rm_sqrt(__x);
    if (__x < 0.0) {
        __yi = __rm_rint(__y);
        if (__yi != __y) return __RM_NAN;
        __neg = (int)(((long long)__yi) & 1);
        __x = -__x;
    }
    __r = __rm_exp(__y * __rm_log(__x));
    return __neg ? -__r : __r;
}

/* ---------------------------------------------------------------- trig */

/* return r in [-pi/4, pi/4] and quadrant q, x = q*(pi/2) + r */
static void __rm_sincos_reduce(double __x, double *__r, int *__q) {
    double __n = __rm_rint(__x * __rm_2OPI);
    *__q = (int)(((long long)__n) & 3);
    *__r = ((__x - __n * 1.570796326794896557998982)
            - __n * 6.123233995736766035868821e-17)
           - __n * (-1.4973849048591697612e-33);
}
static double __rm_sin_k(double __r) {
    double __r2 = __r * __r, __t = __r, __s = __r;
    int __i;
    for (__i = 1; __i <= 8; __i++) {
        __t *= -__r2 / ((double)(2 * __i) * (double)(2 * __i + 1));
        __s += __t;
    }
    return __s;
}
static double __rm_cos_k(double __r) {
    double __r2 = __r * __r, __t = 1.0, __s = 1.0;
    int __i;
    for (__i = 1; __i <= 9; __i++) {
        __t *= -__r2 / ((double)(2 * __i - 1) * (double)(2 * __i));
        __s += __t;
    }
    return __s;
}
static double __rm_sin(double __x) {
    double __r;
    int __q;
    if (__rm_isnan(__x)) return __x;
    if (__rm_isinf(__x)) return __RM_NAN;
    if (__rm_fabs(__x) > 1.0e7) __x = __rm_fmod(__x, 6.283185307179586476925287);
    __rm_sincos_reduce(__x, &__r, &__q);
    switch (__q) {
        case 0:  return __rm_sin_k(__r);
        case 1:  return __rm_cos_k(__r);
        case 2:  return -__rm_sin_k(__r);
        default: return -__rm_cos_k(__r);
    }
}
static double __rm_cos(double __x) {
    double __r;
    int __q;
    if (__rm_isnan(__x)) return __x;
    if (__rm_isinf(__x)) return __RM_NAN;
    if (__rm_fabs(__x) > 1.0e7) __x = __rm_fmod(__x, 6.283185307179586476925287);
    __rm_sincos_reduce(__x, &__r, &__q);
    switch (__q) {
        case 0:  return __rm_cos_k(__r);
        case 1:  return -__rm_sin_k(__r);
        case 2:  return -__rm_cos_k(__r);
        default: return __rm_sin_k(__r);
    }
}
/* atan series, accurate for |x| <= 0.4143 */
static double __rm_atan_series(double __x) {
    double __x2 = __x * __x, __t = __x, __s = __x;
    int __i;
    for (__i = 1; __i <= 20; __i++) {
        __t *= -__x2;
        __s += __t / (double)(2 * __i + 1);
    }
    return __s;
}
static double __rm_atan(double __x) {
    int __neg;
    double __a, __r;
    if (__rm_isnan(__x)) return __x;
    if (__rm_isinf(__x)) return __rm_copysign(M_PI_2, __x);
    __neg = (__x < 0.0);
    __a = __rm_fabs(__x);
    if (__a <= 0.2679491924311227)
        __r = __rm_atan_series(__a);
    else if (__a <= 1.0)
        __r = M_PI / 6.0 + __rm_atan_series((__a - 0.57735026918962576) /
                                            (1.0 + 0.57735026918962576 * __a));
    else if (__a <= 2.4142135623730951)
        __r = M_PI_4 + __rm_atan_series((__a - 1.0) / (1.0 + __a));
    else
        __r = M_PI_2 - __rm_atan_series(1.0 / __a);
    return __neg ? -__r : __r;
}
static double __rm_atan2(double __y, double __x) {
    double __a, __r;
    if (__rm_isnan(__x) || __rm_isnan(__y)) return __RM_NAN;
    if (__x == 0.0 && __y == 0.0) return __rm_copysign(0.0, __y);
    if (__x == 0.0) return __rm_copysign(M_PI_2, __y);
    if (__y == 0.0) return (__x > 0.0) ? __rm_copysign(0.0, __y)
                                       : __rm_copysign(M_PI, __y);
    if (__rm_isinf(__x) || __rm_isinf(__y)) {
        __a = __rm_isinf(__x) ? M_PI_4 : M_PI_2;
        if (__x > 0.0) return (__y > 0.0) ? __a : -__a;
        return (__y > 0.0) ? (M_PI - __a) : (__a - M_PI);
    }
    __a = __rm_atan(__rm_fabs(__y / __x));
    __r = (__x > 0.0) ? __a : (M_PI - __a);
    return (__y < 0.0) ? -__r : __r;
}
static double __rm_asin(double __x) {
    if (__rm_isnan(__x)) return __x;
    if (__rm_fabs(__x) > 1.0) return __RM_NAN;
    if (__x == 1.0) return M_PI_2;
    if (__x == -1.0) return -M_PI_2;
    return __rm_atan2(__x, __rm_sqrt(1.0 - __x * __x));
}
static double __rm_acos(double __x) {
    if (__rm_isnan(__x)) return __x;
    if (__rm_fabs(__x) > 1.0) return __RM_NAN;
    if (__x == 1.0) return 0.0;
    if (__x == -1.0) return M_PI;
    return __rm_atan2(__rm_sqrt(1.0 - __x * __x), __x);
}

/* ---------------------------------------------------------- classification */

bool signbit(double __x) { return (__rm_d2u(__x) >> 63) != 0ULL; }
int fpclassify(double __x) {
    uint64_t __u = __RM_ABSBITS(__rm_d2u(__x));
    if (__u == 0) return FP_ZERO;
    if (__u >= __RM_EXPM) return (__u > __RM_EXPM) ? FP_NAN : FP_INFINITE;
    if (__u < (1ULL << 52)) return FP_SUBNORMAL;
    return FP_NORMAL;
}
bool isfinite(double __x) { return __rm_isfinite(__x) != 0; }
bool isinf(double __x) { return __rm_isinf(__x) != 0; }
bool isnan(double __x) { return __rm_isnan(__x) != 0; }
bool isnormal(double __x) {
    uint64_t __u = __rm_d2u(__x) & __RM_EXPM;
    return (__u != 0) && (__u != __RM_EXPM);
}
bool isgreater(double __x, double __y) { return !__rm_isnan(__x) && !__rm_isnan(__y) && __x > __y; }
bool isgreaterequal(double __x, double __y) { return !__rm_isnan(__x) && !__rm_isnan(__y) && __x >= __y; }
bool isless(double __x, double __y) { return !__rm_isnan(__x) && !__rm_isnan(__y) && __x < __y; }
bool islessequal(double __x, double __y) { return !__rm_isnan(__x) && !__rm_isnan(__y) && __x <= __y; }
bool islessgreater(double __x, double __y) { return !__rm_isnan(__x) && !__rm_isnan(__y) && (__x < __y || __x > __y); }
bool isunordered(double __x, double __y) { return __rm_isnan(__x) || __rm_isnan(__y); }

/* ------------------------------------------------------------ abs / sign */

float fabsf(float __x) { return __rm_u2f(__rm_f2u(__x) & 0x7fffffffU); }
double fabs(double __x) { return __rm_fabs(__x); }
long double fabsl(long double __x) { return (__x < 0.0L) ? -__x : __x; }

double copysign(double __x, double __y) { return __rm_copysign(__x, __y); }
double copysignf(float __x, float __y) { return __rm_copysign((double)__x, (double)__y); }
long double copysignl(long double __x, long double __y) {
    long double __a = (__x < 0.0L) ? -__x : __x;
    return (__y < 0.0L) ? -__a : __a;
}

/* --------------------------------------------------------------- sqrt */

float sqrtf(float __x) {
    float __r;
    __asm__ ("sqrtss %1, %0" : "=x"(__r) : "x"(__x));
    return __r;
}
double sqrt(double __x) { return __rm_sqrt(__x); }
long double sqrtl(long double __x) { return (long double)__rm_sqrt((double)__x); }

/* ------------------------------------------------------------ rounding */

float truncf(float __x) { return (float)__rm_trunc((double)__x); }
double trunc(double __x) { return __rm_trunc(__x); }
long double truncl(long double __x) { return (long double)__rm_trunc((double)__x); }

float floorf(float __x) { return (float)__rm_floor((double)__x); }
double floor(double __x) { return __rm_floor(__x); }
long double floorl(long double __x) { return (long double)__rm_floor((double)__x); }

float ceilf(float __x) { return (float)__rm_ceil((double)__x); }
double ceil(double __x) { return __rm_ceil(__x); }
long double ceill(long double __x) { return (long double)__rm_ceil((double)__x); }

float rintf(float __x) { return (float)__rm_rint((double)__x); }
double rint(double __x) { return __rm_rint(__x); }
long double rintl(long double __x) { return (long double)__rm_rint((double)__x); }

float nearbyintf(float __x) { return (float)__rm_rint((double)__x); }
double nearbyint(double __x) { return __rm_rint(__x); }
long double nearbyintl(long double __x) { return (long double)__rm_rint((double)__x); }

float roundf(float __x) { return (float)__rm_round((double)__x); }
double round(double __x) { return __rm_round(__x); }
long double roundl(long double __x) { return (long double)__rm_round((double)__x); }

long lrintf(float __x) { return (long)__rm_rint((double)__x); }
long lrint(double __x) { return (long)__rm_rint(__x); }
long lrintl(long double __x) { return (long)__rm_rint((double)__x); }

long long llrintf(float __x) { return (long long)__rm_rint((double)__x); }
long long llrint(double __x) { return (long long)__rm_rint(__x); }
long long llrintl(long double __x) { return (long long)__rm_rint((double)__x); }

long lroundf(float __x) { return (long)__rm_round((double)__x); }
long lround(double __x) { return (long)__rm_round(__x); }
long lroundl(long double __x) { return (long)__rm_round((double)__x); }

long long llroundf(float __x) { return (long long)__rm_round((double)__x); }
long long llround(double __x) { return (long long)__rm_round(__x); }
long long llroundl(long double __x) { return (long long)__rm_round((double)__x); }

/* ------------------------------------------------------- exp / log / pow */

float expf(float __x) { return (float)__rm_exp((double)__x); }
double exp(double __x) { return __rm_exp(__x); }
long double expl(long double __x) { return (long double)__rm_exp((double)__x); }

float exp2f(float __x) { return (float)__rm_exp((double)__x * __rm_LN2); }
double exp2(double __x) { return __rm_exp(__x * __rm_LN2); }
long double exp2l(long double __x) { return (long double)__rm_exp((double)__x * __rm_LN2); }

float expm1f(float __x) { return (float)expm1((double)__x); }
double expm1(double __x) {
    if (__rm_fabs(__x) < 1.0e-5) return __x + 0.5 * __x * __x;
    return __rm_exp(__x) - 1.0;
}
long double expm1l(long double __x) { return (long double)expm1((double)__x); }

float logf(float __x) { return (float)__rm_log((double)__x); }
double log(double __x) { return __rm_log(__x); }
long double logl(long double __x) { return (long double)__rm_log((double)__x); }

float log2f(float __x) { return (float)(__rm_log((double)__x) * __rm_LOG2E); }
double log2(double __x) { return __rm_log(__x) * __rm_LOG2E; }
long double log2l(long double __x) { return (long double)(__rm_log((double)__x) * __rm_LOG2E); }

float log10f(float __x) { return (float)(__rm_log((double)__x) * __rm_INVLN10); }
double log10(double __x) { return __rm_log(__x) * __rm_INVLN10; }
long double log10l(long double __x) { return (long double)(__rm_log((double)__x) * __rm_INVLN10); }

float log1pf(float __x) { return (float)log1p((double)__x); }
double log1p(double __x) {
    if (__rm_fabs(__x) < 1.0e-4)
        return __x - 0.5 * __x * __x + __x * __x * __x / 3.0;
    return __rm_log(1.0 + __x);
}
long double log1pl(long double __x) { return (long double)log1p((double)__x); }

float powf(float __x, float __y) { return (float)__rm_pow((double)__x, (double)__y); }
double pow(double __x, double __y) { return __rm_pow(__x, __y); }
long double powl(long double __x, long double __y) { return (long double)__rm_pow((double)__x, (double)__y); }

/* ---------------------------------------------------------------- fmod */

static double __rm_fmod(double __x, double __y) {
    double __a, __b, __d;
    if (__y == 0.0 || __rm_isnan(__x) || __rm_isnan(__y) || __rm_isinf(__x))
        return __RM_NAN;
    if (__rm_isinf(__y)) return __x;
    __a = __rm_fabs(__x);
    __b = __rm_fabs(__y);
    if (__a < __b) return __x;
    while (__a >= __b) {
        __d = __b;
        while (__a >= __d + __d && __rm_isfinite(__d + __d)) __d += __d;
        __a -= __d;
    }
    return __rm_copysign(__a, __x);
}

float fmodf(float __x, float __y) { return (float)__rm_fmod((double)__x, (double)__y); }
double fmod(double __x, double __y) { return __rm_fmod(__x, __y); }
long double fmodl(long double __x, long double __y) { return (long double)__rm_fmod((double)__x, (double)__y); }

float remainderf(float __x, float __y) { return (float)remainder((double)__x, (double)__y); }
double remainder(double __x, double __y) {
    if (__y == 0.0 || __rm_isnan(__x) || __rm_isnan(__y) || __rm_isinf(__x))
        return __RM_NAN;
    if (__rm_isinf(__y)) return __x;
    return __x - __rm_rint(__x / __y) * __y;
}
long double remainderl(long double __x, long double __y) { return (long double)remainder((double)__x, (double)__y); }

float remquof(float __x, float __y, int *__z) { return (float)remquo((double)__x, (double)__y, __z); }
double remquo(double __x, double __y, int *__z) {
    double __r = remainder(__x, __y);
    if (__z) *__z = (int)__rm_rint(__x / __y);
    return __r;
}
long double remquol(long double __x, long double __y, int *__z) {
    return (long double)remquo((double)__x, (double)__y, __z);
}

/* ------------------------------------------------------------ scalbn,modf */

float frexpf(float __x, int *__e) { return (float)__rm_frexp((double)__x, __e); }
double frexp(double __x, int *__e) { return __rm_frexp(__x, __e); }
long double frexpl(long double __x, int *__e) { return (long double)__rm_frexp((double)__x, __e); }

float ldexpf(float __x, int __e) { return (float)__rm_scalbn((double)__x, __e); }
double ldexp(double __x, int __e) { return __rm_scalbn(__x, __e); }
long double ldexpl(long double __x, int __e) { return (long double)__rm_scalbn((double)__x, __e); }

float scalbnf(float __x, int __y) { return (float)__rm_scalbn((double)__x, __y); }
double scalbn(double __x, int __y) { return __rm_scalbn(__x, __y); }
long double scalbnl(long double __x, int __y) { return (long double)__rm_scalbn((double)__x, __y); }

float scalblnf(float __x, long __y) { return (float)__rm_scalbn((double)__x, (int)__y); }
double scalbln(double __x, long __y) { return __rm_scalbn(__x, (int)__y); }
long double scalblnl(long double __x, long __y) { return (long double)__rm_scalbn((double)__x, (int)__y); }

float modff(float __x, float *__y) {
    double __ip;
    double __fp = modf((double)__x, &__ip);
    *__y = (float)__ip;
    return (float)__fp;
}
double modf(double __x, double *__y) {
    double __t = __rm_trunc(__x);
    *__y = __t;
    return __rm_isinf(__x) ? __rm_copysign(0.0, __x) : (__x - __t);
}
long double modfl(long double __x, long double *__y) {
    double __ip;
    double __fp = modf((double)__x, &__ip);
    *__y = (long double)__ip;
    return (long double)__fp;
}

int ilogbf(float __x) { return (int)ilogb((double)__x); }
double ilogb(double __x) {
    int __e = 0;
    if (__rm_isnan(__x) || __rm_isinf(__x)) return 2147483647.0;
    if (__x == 0.0) return -2147483648.0;
    __rm_frexp(__x, &__e);
    return (double)(__e - 1);
}
int ilogbl(long double __x) { return (int)ilogb((double)__x); }

float logbf(float __x) { return (float)logb((double)__x); }
double logb(double __x) {
    int __e = 0;
    if (__rm_isnan(__x)) return __x;
    if (__rm_isinf(__x)) return __RM_INF;
    if (__x == 0.0) return -__RM_INF;
    __rm_frexp(__x, &__e);
    return (double)(__e - 1);
}
long double logbl(long double __x) { return (long double)logb((double)__x); }

static double __rm_nextafter(double __x, double __y) {
    uint64_t __u;
    int __up;
    if (__rm_isnan(__x) || __rm_isnan(__y)) return __RM_NAN;
    if (__x == __y) return __y;
    if (__x == 0.0) return __rm_copysign(__rm_u2d(1ULL), __y);
    __u = __rm_d2u(__x);
    __up = (__y > __x);
    if (__x > 0.0) __u += __up ? 1 : -1;
    else __u += __up ? -1 : 1;
    return __rm_u2d(__u);
}
static float __rm_nextafterf(float __x, float __y) {
    uint32_t __u;
    int __up;
    if ((__rm_f2u(__x) & 0x7fffffffU) > 0x7f800000U) return __RM_NANF;
    if ((__rm_f2u(__y) & 0x7fffffffU) > 0x7f800000U) return __RM_NANF;
    if (__x == __y) return __y;
    if (__x == 0.0f) return __rm_u2f((__y > 0.0f) ? 1U : 0x80000001U);
    __u = __rm_f2u(__x);
    __up = (__y > __x);
    if (__x > 0.0f) __u += __up ? 1U : (uint32_t)-1;
    else __u += __up ? (uint32_t)-1 : 1U;
    return __rm_u2f(__u);
}

float nextafterf(float __x, float __y) { return __rm_nextafterf(__x, __y); }
double nextafter(double __x, double __y) { return __rm_nextafter(__x, __y); }
long double nextafterl(long double __x, long double __y) {
    return (long double)__rm_nextafter((double)__x, (double)__y);
}

float nexttowardf(float __x, long double __y) { return __rm_nextafterf(__x, (float)__y); }
double nexttoward(double __x, long double __y) { return __rm_nextafter(__x, (double)__y); }
long double nexttowardl(long double __x, long double __y) {
    return (long double)__rm_nextafter((double)__x, (double)__y);
}

/* ------------------------------------------------------- hypot, fdim, fma */

float hypotf(float __x, float __y) { return (float)hypot((double)__x, (double)__y); }
double hypot(double __x, double __y) {
    double __r;
    __x = __rm_fabs(__x);
    __y = __rm_fabs(__y);
    if (__x < __y) { double __t = __x; __x = __y; __y = __t; }
    if (__rm_isinf(__x)) return __RM_INF;
    if (__x == 0.0) return 0.0;
    __r = __y / __x;
    return __x * __rm_sqrt(1.0 + __r * __r);
}
long double hypotl(long double __x, long double __y) { return (long double)hypot((double)__x, (double)__y); }

float fdimf(float __x, float __y) { return (float)fdim((double)__x, (double)__y); }
double fdim(double __x, double __y) { return (__x > __y) ? (__x - __y) : 0.0; }
long double fdiml(long double __x, long double __y) { return (long double)fdim((double)__x, (double)__y); }

float fmaf(float __x, float __y, float __z) {
    return (float)((double)__x * (double)__y + (double)__z);
}
double fma(double __x, double __y, double __z) { return __x * __y + __z; }
long double fmal(long double __x, long double __y, long double __z) { return __x * __y + __z; }

float fmaxf(float __x, float __y) { return (float)fmax((double)__x, (double)__y); }
double fmax(double __x, double __y) {
    if (__rm_isnan(__x)) return __y;
    if (__rm_isnan(__y)) return __x;
    return (__x > __y) ? __x : __y;
}
long double fmaxl(long double __x, long double __y) { return (long double)fmax((double)__x, (double)__y); }

float fminf(float __x, float __y) { return (float)fmin((double)__x, (double)__y); }
double fmin(double __x, double __y) {
    if (__rm_isnan(__x)) return __y;
    if (__rm_isnan(__y)) return __x;
    return (__x < __y) ? __x : __y;
}
long double fminl(long double __x, long double __y) { return (long double)fmin((double)__x, (double)__y); }

/* ---------------------------------------------------------------- trig */

float sinf(float __x) { return (float)__rm_sin((double)__x); }
double sin(double __x) { return __rm_sin(__x); }
long double sinl(long double __x) { return (long double)__rm_sin((double)__x); }

float cosf(float __x) { return (float)__rm_cos((double)__x); }
double cos(double __x) { return __rm_cos(__x); }
long double cosl(long double __x) { return (long double)__rm_cos((double)__x); }

static double __rm_tan(double __x) {
    double __s = __rm_sin(__x), __c = __rm_cos(__x);
    if (__c == 0.0) return __rm_copysign(__RM_INF, __s);
    return __s / __c;
}
float tanf(float __x) { return (float)__rm_tan((double)__x); }
double tan(double __x) { return __rm_tan(__x); }
long double tanl(long double __x) { return (long double)__rm_tan((double)__x); }

float asinf(float __x) { return (float)__rm_asin((double)__x); }
double asin(double __x) { return __rm_asin(__x); }
long double asinl(long double __x) { return (long double)__rm_asin((double)__x); }

float acosf(float __x) { return (float)__rm_acos((double)__x); }
double acos(double __x) { return __rm_acos(__x); }
long double acosl(long double __x) { return (long double)__rm_acos((double)__x); }

float atanf(float __x) { return (float)__rm_atan((double)__x); }
double atan(double __x) { return __rm_atan(__x); }
long double atanl(long double __x) { return (long double)__rm_atan((double)__x); }

float atan2f(float __y, float __x) { return (float)__rm_atan2((double)__y, (double)__x); }
double atan2(double __y, double __x) { return __rm_atan2(__y, __x); }
long double atan2l(long double __y, long double __x) {
    return (long double)__rm_atan2((double)__y, (double)__x);
}

/* ----------------------------------------------------------- hyperbolic */

float sinhf(float __x) { return (float)sinh((double)__x); }
double sinh(double __x) {
    double __a = __rm_fabs(__x), __e, __r;
    if (__a < 1.0e-9) return __x;
    if (__rm_isinf(__x)) return __x;
    __e = __rm_exp(__a);
    __r = 0.5 * (__e - 1.0 / __e);
    return (__x < 0.0) ? -__r : __r;
}
long double sinhl(long double __x) { return (long double)sinh((double)__x); }

float coshf(float __x) { return (float)cosh((double)__x); }
double cosh(double __x) {
    double __e;
    if (__rm_isnan(__x)) return __x;
    if (__rm_isinf(__x)) return __RM_INF;
    __e = __rm_exp(__rm_fabs(__x));
    return 0.5 * (__e + 1.0 / __e);
}
long double coshl(long double __x) { return (long double)cosh((double)__x); }

float tanhf(float __x) { return (float)tanh((double)__x); }
double tanh(double __x) {
    double __a = __rm_fabs(__x), __e, __r;
    if (__rm_isnan(__x)) return __x;
    if (__a > 20.0) return (__x < 0.0) ? -1.0 : 1.0;
    if (__a < 1.0e-9) return __x;
    __e = __rm_exp(2.0 * __a);
    __r = (__e - 1.0) / (__e + 1.0);
    return (__x < 0.0) ? -__r : __r;
}
long double tanhl(long double __x) { return (long double)tanh((double)__x); }

float asinhf(float __x) { return (float)asinh((double)__x); }
double asinh(double __x) {
    double __a = __rm_fabs(__x), __r;
    if (__rm_isnan(__x) || __rm_isinf(__x)) return __x;
    __r = __rm_log(__a + __rm_sqrt(__a * __a + 1.0));
    return (__x < 0.0) ? -__r : __r;
}
long double asinhl(long double __x) { return (long double)asinh((double)__x); }

float acoshf(float __x) { return (float)acosh((double)__x); }
double acosh(double __x) {
    if (__rm_isnan(__x)) return __x;
    if (__x < 1.0) return __RM_NAN;
    if (__rm_isinf(__x)) return __x;
    return __rm_log(__x + __rm_sqrt(__x * __x - 1.0));
}
long double acoshl(long double __x) { return (long double)acosh((double)__x); }

float atanhf(float __x) { return (float)atanh((double)__x); }
double atanh(double __x) {
    double __a = __rm_fabs(__x), __r;
    if (__rm_isnan(__x)) return __x;
    if (__a > 1.0) return __RM_NAN;
    if (__a == 1.0) return __rm_copysign(__RM_INF, __x);
    __r = 0.5 * __rm_log((1.0 + __a) / (1.0 - __a));
    return (__x < 0.0) ? -__r : __r;
}
long double atanhl(long double __x) { return (long double)atanh((double)__x); }

/* ---------------------------------------------------------------- cbrt */

float cbrtf(float __x) { return (float)cbrt((double)__x); }
double cbrt(double __x) {
    double __r;
    if (__x == 0.0 || !__rm_isfinite(__x)) return __x;
    __r = __rm_pow(__rm_fabs(__x), 1.0 / 3.0);
    return (__x < 0.0) ? -__r : __r;
}
long double cbrtl(long double __x) { return (long double)cbrt((double)__x); }

/* -------------------------------------------------------- erf and gamma */

float erff(float __x) { return (float)erf((double)__x); }
double erf(double __x) {
    double __a = __rm_fabs(__x), __t, __y;
    if (__rm_isnan(__x)) return __x;
    if (__a > 6.0) return (__x < 0.0) ? -1.0 : 1.0;
    __t = 1.0 / (1.0 + 0.3275911 * __a);
    __y = 1.0 - (((((1.061405429 * __t - 1.453152027) * __t) + 1.421413741) * __t
                  - 0.284496736) * __t + 0.254829592) * __t * __rm_exp(-__a * __a);
    return (__x < 0.0) ? -__y : __y;
}
long double erfl(long double __x) { return (long double)erf((double)__x); }

float erfcf(float __x) { return (float)erfc((double)__x); }
double erfc(double __x) {
    if (__rm_isnan(__x)) return __x;
    if (__x > 6.0) return 0.0;
    if (__x < -6.0) return 2.0;
    return 1.0 - erf(__x);
}
long double erfcl(long double __x) { return (long double)erfc((double)__x); }

static const double __rm_lg[9] = {
    0.99999999999980993, 676.5203681218851, -1259.1392167224028,
    771.32342877765313, -176.61502916214059, 12.507343278686905,
    -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7
};

static double __rm_lgamma_pos(double __x) {
    double __a, __t;
    int __i;
    if (__x < 0.5)
        return __rm_log(M_PI / __rm_sin(M_PI * __x)) - __rm_lgamma_pos(1.0 - __x);
    __x -= 1.0;
    __a = __rm_lg[0];
    __t = __x + 7.5;
    for (__i = 1; __i < 9; __i++) __a += __rm_lg[__i] / (__x + (double)__i);
    return 0.5 * __rm_log(2.0 * M_PI) + (__x + 0.5) * __rm_log(__t) - __t
           + __rm_log(__a);
}

float lgammaf(float __x) { return (float)lgamma((double)__x); }
double lgamma(double __x) {
    if (__rm_isnan(__x)) return __x;
    if (__rm_isinf(__x)) return __RM_INF;
    if (__x <= 0.0 && __x - __rm_rint(__x) == 0.0) return __RM_INF;
    if (__x < 0.5)
        return __rm_log(M_PI / __rm_fabs(__rm_sin(M_PI * __x)))
               - __rm_lgamma_pos(1.0 - __x);
    return __rm_lgamma_pos(__x);
}
long double lgammal(long double __x) { return (long double)lgamma((double)__x); }

float tgammaf(float __x) { return (float)tgamma((double)__x); }
double tgamma(double __x) {
    if (__rm_isnan(__x)) return __x;
    if (__x <= 0.0 && __x - __rm_rint(__x) == 0.0) return __RM_NAN;
    if (__rm_isinf(__x)) return __x;
    if (__x < 0.5) return M_PI / (__rm_sin(M_PI * __x) * tgamma(1.0 - __x));
    return __rm_exp(__rm_lgamma_pos(__x));
}
long double tgammal(long double __x) { return (long double)tgamma((double)__x); }

/* ------------------------------------------------------------------ nan */

float nanf(const char *__s) { (void)__s; return __RM_NANF; }
double nan(const char *__s) { (void)__s; return __RM_NAN; }
long double nanl(const char *__s) { (void)__s; return (long double)__RM_NAN; }

/* ---------------------------------------------------- Darwin/libm extras */

double __exp10(double __x) { return __rm_pow(10.0, __x); }

typedef struct { double sin; double cos; } __rm_sc64_t;
typedef float __rm_v4sf __attribute__((vector_size(16)));

__rm_sc64_t __sincos_stret(double __x) {
    __rm_sc64_t __r;
    __r.sin = __rm_sin(__x);
    __r.cos = __rm_cos(__x);
    return __r;
}

__rm_v4sf __sincosf_stret(float __x) {
    __rm_v4sf __v;
    __v[0] = (float)__rm_sin((double)__x);
    __v[1] = (float)__rm_cos((double)__x);
    __v[2] = 0.0f;
    __v[3] = 0.0f;
    return __v;
}

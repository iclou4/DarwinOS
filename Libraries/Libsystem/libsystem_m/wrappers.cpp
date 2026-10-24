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

#include <cmath>

double log(double x) { return __builtin_log(x); }
double pow(double x, double y) { return __builtin_pow(x, y); }
double fabs(double x) { return __builtin_fabs(x); }
double fmod(double x, double y) { return __builtin_fmod(x, y); }
double modf(double x, double* iptr) { return __builtin_modf(x, iptr); }
double floor(double x) { return __builtin_floor(x); }
double ceil(double x) { return __builtin_ceil(x); }
float expf(float x) { return __builtin_expf(x); }
double cos(double x) { return __builtin_cos(x); }
double sin(double x) { return __builtin_sin(x); }
double tan(double x) { return __builtin_tan(x); }
double asin(double x) { return __builtin_asin(x); }
double atan(double x) { return __builtin_atan(x); }
double sqrt(double x) { return __builtin_sqrt(x); }
double atan2(double x, double y) { return __builtin_atan2(x, y); }
double round(double x) { return __builtin_round(x); }
bool signbit(double x) { return __builtin_signbit(x); }
bool isfinite(double x) { return __builtin_isfinite(x); }

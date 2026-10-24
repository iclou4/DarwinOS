#include <math.h>

struct __sincos_stret_t {
    double sinval;
    double cosval;
};

struct __sincos_stret_t __sincos_stret(double x) {
    struct __sincos_stret_t res;
    res.sinval = sin(x);
    res.cosval = cos(x);
    return res;
}

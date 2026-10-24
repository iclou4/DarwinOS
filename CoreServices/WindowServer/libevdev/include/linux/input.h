#ifdef __linux__
#include "linux/input.h"
#elif defined(__FreeBSD__)
#include "freebsd/input.h"
#else
#include "freebsd/input.h"
#endif

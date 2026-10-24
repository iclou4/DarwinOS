#include <mach/kmod.h>
extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);
extern kern_return_t pthread_start(kmod_info_t *ki, void *data);
extern kern_return_t pthread_stop(kmod_info_t *ki, void *data);
__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(com.apple.kec.pthread, "", _start, _stop)
__private_extern__ kmod_start_func_t *_realmain = pthread_start;
__private_extern__ kmod_stop_func_t *_antimain = pthread_stop;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;

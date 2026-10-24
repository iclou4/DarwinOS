#include <mach/kmod.h>
extern kern_return_t _start(kmod_info_t *ki, void *data);
extern kern_return_t _stop(kmod_info_t *ki, void *data);
extern kern_return_t cc_kext_start(kmod_info_t *ki, void *data);
extern kern_return_t cc_kext_stop(kmod_info_t *ki, void *data);
__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL(com.apple.kec.corecrypto, "1.0.0", _start, _stop)
__private_extern__ kmod_start_func_t *_realmain = cc_kext_start;
__private_extern__ kmod_stop_func_t *_antimain = cc_kext_stop;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;

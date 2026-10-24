MK_WERROR = no
PROG = dsc_extractor
SRCS =  launch-cache/dsc_extractor.cpp \
	launch-cache/dsc_iterator.cpp
MAN =

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DBUILDING_CACHE_BUILDER \
        -I${.CURDIR}/dyld3 -I${RAVYN_SDKROOT}/usr/include/Private \
	-I${.CURDIR}/dyld3/shared-cache 

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD} -lc++ \
        -Wl,-exported_symbol,_dyld_shared_cache_extract_dylibs_progress

.include <bsd.prog.mk>


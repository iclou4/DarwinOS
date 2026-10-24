MK_WERROR = no
SUBDIR =
.PATH: ${.CURDIR}
LIB = dsc
.PATH: ${.CURDIR:H}
SRCS =  launch-cache/dsc_iterator.cpp \
	launch-cache/dsc_extractor.cpp

CXXSTD = c++14
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -I${.CURDIR:H}/dyld3 -I${RAVYN_SDKROOT}/usr/include/Private \
        -I${.CURDIR:H}/dyld3/shared-cache -I${.CURDIR:H}/../Libsystem/private

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

.include <bsd.lib.mk>


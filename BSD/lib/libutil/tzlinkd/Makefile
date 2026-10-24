MK_WERROR = no

PROG = tzlinkd
SRCS = tzlinkd.c
MAN =

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -I${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch \
        -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern -I${.CURDIR:H}
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

.include <bsd.prog.mk>


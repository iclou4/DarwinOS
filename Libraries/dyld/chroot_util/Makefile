MK_WERROR = no
SUBDIR =
PROG = chroot_util
.PATH: ${.CURDIR:H}
SRCS =  dyld3/Diagnostics.cpp \
	dyld3/MachOFile.cpp \
	chroot_util.cpp
MAN =

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DTARGET_OS_BRIDGE=0 -I${.CURDIR:H}/dyld3/shared-cache \
        -I${.CURDIR:H}/dyld3 -I${.CURDIR:H}/../Libsystem/private -I${.CURDIR:H}

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD} -lc++

.include <bsd.prog.mk>


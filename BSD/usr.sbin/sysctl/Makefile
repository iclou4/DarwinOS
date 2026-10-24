.include <src.opts.mk>

SUBDIR =
CFLAGS = -Wno-int-conversion

PROG=	sysctl

MK_WERROR = no
MAN=	sysctl.8
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -Wno-nullability-completeness
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

HAS_TESTS=

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/sbin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/sbin/

.include <bsd.prog.mk>

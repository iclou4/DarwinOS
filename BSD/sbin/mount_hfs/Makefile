SUBDIR = 
MK_WERROR = no

PROG = mount_hfs
SRCS = mount_hfs.c #optical.c
MAN = mount_hfs.8

LIBUTIL = ${ROOT_SOURCE_DIR}/BSD/lib/libutil
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -I${.CURDIR} -I${.CURDIR}/include -I${LIBUTIL}
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD} -L${ROOT_BINARY_DIR}${LIBUTIL} -lutil

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/sbin
	cp -f ${PROG} ${SYSROOT_DIR}/sbin/


.include <bsd.prog.mk>

SUBDIR =
MK_WERROR = no
PROG = grep
MAN = grep.1 
SRCS = file.c grep.c queue.c util.c

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-nullability-completeness -DWITHOUT_LZMA -DWITHOUT_FASTMATCH \
	-DWITHOUT_NLS -I${ROOT_SOURCE_DIR}/Libraries/zlib \
        -I${ROOT_SOURCE_DIR}/BSD/lib/bzip2/src
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-fuse-ld=${LD} -L${SYSROOT_DIR}/usr/lib -lbz2 -lz

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/

.include <bsd.prog.mk>

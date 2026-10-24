MK_WERROR = no
SUBDIR =
PROG = printf

CFLAGS += -D__DARWIN_64_BIT_INO_T -D_DARWIN_C_SOURCE \
        -mmacos-version-min=${MACOS_VERSION_MIN}

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin/

.include <bsd.prog.mk>


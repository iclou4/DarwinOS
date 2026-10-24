SUBDIR =
PROG=	touch
CFLAGS += -D_DARWIN_C_SOURCE

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin

.include <bsd.prog.mk>

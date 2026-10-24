SUBDIR =
PROG=	sed
SRCS=	compile.c main.c misc.c process.c


all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin/

.include <bsd.prog.mk>

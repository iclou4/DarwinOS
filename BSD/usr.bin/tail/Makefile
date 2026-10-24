SUBDIR =
PROG=	tail
SRCS=	forward.c misc.c read.c reverse.c tail.c

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin

.include <bsd.prog.mk>

SUBDIR =
PROG=	env
SRCS=	env.c envopts.c

HAS_TESTS=

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin/


.include <bsd.prog.mk>

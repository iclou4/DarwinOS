.include <src.opts.mk>

SUBDIR =
MK_WERROR = no
PROG=	ls
SRCS=	cmp.c ls.c print.c util.c

HAS_TESTS=

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/

.include <bsd.prog.mk>

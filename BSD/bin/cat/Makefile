.include <src.opts.mk>

SUBDIR =
PROG=	cat

HAS_TESTS=

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/


.include <bsd.prog.mk>

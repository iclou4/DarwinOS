.include <src.opts.mk>

SUBDIR =
PROG=	echo

HAS_TESTS=
SUBDIR.${MK_TESTS}+= tests

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/


.include <bsd.prog.mk>

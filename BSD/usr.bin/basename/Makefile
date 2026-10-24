SUBDIR =
PROG=	basename
MLINKS=	basename.1 dirname.1

HAS_TESTS=
SUBDIR.${MK_TESTS}+= tests

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin/

.include <bsd.prog.mk>

SUBDIR =
PROG=	rm

MK_WERROR = no
LINKS=	${BINDIR}/rm ${BINDIR}/unlink
MLINKS=	rm.1 unlink.1

HAS_TESTS=

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/


.include <bsd.prog.mk>

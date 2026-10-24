PROG = killall
MAN = killall.1
SUBDIR =

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/

.include <bsd.prog.mk>

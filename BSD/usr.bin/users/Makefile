SUBDIR =
MK_WERROR = no
PROG = users
CSTD = c99
CFLAGS += -Wno-cast-qual

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${PROG} ${SYSROOT_DIR}/usr/bin

.include <bsd.prog.mk>


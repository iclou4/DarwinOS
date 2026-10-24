MK_WERROR = no

SUBDIR =
PROG = ps
SRCS = fmt.c keyword.c nlist.c print.c ps.c tasks.c

CFLAGS += -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/libproc

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/


.include <bsd.prog.mk>

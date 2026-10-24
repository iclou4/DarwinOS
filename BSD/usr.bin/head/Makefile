.include <src.opts.mk>

SUBDIR =
PROG=	head
CFLAGS += -I${ROOT_SOURCE_DIR}/BSD/lib/libutil
LDFLAGS += -L${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/BSD/lib/libutil -lutil

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/


.include <bsd.prog.mk>

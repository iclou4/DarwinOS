.include <src.opts.mk>

MK_WERROR = no
SUBDIR =
PROG=	find
SRCS=	find.c function.c ls.c main.c misc.c operator.c option.c printf.c \
	getdate.y
YFLAGS=
CFLAGS += -D__DARWIN_64_BIT_INO_T -D_DARWIN_C_SOURCE -Wno-nullability-completeness
LDFLAGS += -mmacos-version-min=${MACOS_VERSION_MIN} -L${SYSROOT_DIR}${LIBDIR}
LIBS += -lbz2
#CFLAGS.clang+=	-Werror=undef

NO_WMISSING_VARIABLE_DECLARATIONS=

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/

.include <bsd.prog.mk>

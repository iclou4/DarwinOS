.PATH: ${DEVEL}/Default.xctoolchain/cctools/libmacho

SUBDIR =
MK_WERROR = no
LIB = macho
SRCS = arch.c getsecbyname.c getsegbyname.c swap.c
CFLAGS += --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-I${DEVEL}/Default.xctoolchain/dtrace_ctf/include \
	-I${.OBJDIR} -Wno-nullability-completeness \
	-DCPU_TYPE_VEO=255 -DCPU_SUBTYPE_VEO_1=1 -DCPU_SUBTYPE_VEO_2=2 \
	-DCPU_SUBTYPE_VEO_3=3 -DCPU_SUBTYPE_VEO_4=4 -DCPU_SUBTYPE_VEO_ALL=2
LDFLAGS += --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-fuse-ld=${LD}


all: lib${LIB}.dylib

prep:
	cp -Rf ${DEVEL}/Default.xctoolchain/include/stuff ${.OBJDIR}
	cp -Rf ${DEVEL}/Default.xctoolchain/include/mach-o ${.OBJDIR}

lib${LIB}.dylib: prep lib${LIB}.a
	${CC} -o ${.TARGET} --sysroot=${RAVYN_SDKROOT} -umbrella System \
	  -mmacos-version-min=${MACOS_VERSION_MIN} -shared -dylib \
	  -Wl,-install_name,/usr/lib/system/libmacho.dylib ${LDFLAGS} \
	  -Wl,-current_version,949.0.1 -Wl,-compatibility_version,1.0.0 \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a -lSystem
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.OBJDIR}/${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.OBJDIR}/${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

.include <bsd.lib.mk>

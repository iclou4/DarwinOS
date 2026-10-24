SUBDIR =
MK_WERROR = no
LIB = system_trace
SRCS = log.c signpost.c init.c

SLF = /System/Library/Frameworks
CFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT} \
	-ObjC -x objective-c -fblocks \
	-DNUL=\'\0\' -Wno-nullability-completeness \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${ROOT_SOURCE_DIR}/Frameworks \
	-I${RAVYN_SDKROOT}/usr/include \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/PrivateHeaders


LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wl,-install_name,/usr/lib/system/libsystem_trace.dylib \
	-Wl,-current_version,1147.0.3 -Wl,-compatibility_version,1 \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-umbrella System -Wl,-upward-lobjc -lsystem_c -lsystem_platform \
        -ldispatch -ldyld

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	mkdir -p ${RAVYN_SDKROOT}/usr/lib/system ${SYSROOT_DIR}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system
	mkdir -p ${RAVYN_SDKROOT}/usr/include/os
	cp -f ${.CURDIR}/log.h ${RAVYN_SDKROOT}/usr/include/os

.include <bsd.lib.mk>

SUBDIR =
MK_WERROR = no

LIB = util
SRCS =  ExtentManager.cpp \
	getmntopts.c \
	humanize_number.c \
	pidfile.c \
	realhostname.c \
	reexec_to_match_kernel.c \
	trimdomain.c \
	tzlink.c \
	wipefs.cpp

MAN = getmntopts.3 \
      humanize_number.3 \
      pidfile.3 \
      realhostname.3 \
      realhostname_sa.3 \
      reexec_to_match_kernel.3 \
      trimdomain.3 \
      wipefs.3


CFLAGS = -ObjC --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -I${.CURDIR} -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
        -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern ${CFLAGS.${.TARGET}}
CXXFLAGS = -ObjC++ --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -I${.CURDIR} -I${ROOT_SOURCE_DIR}/Kernel/xnu/libsyscall/wrappers/spawn \
        -I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

CFLAGS.tzlink.o = -I${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch \
        -include${RAVYN_SDKROOT}/usr/include/os/object.h

#SUBDIR = tzlinkd

.include <bsd.lib.mk>
.include <bsd.subdir.mk>

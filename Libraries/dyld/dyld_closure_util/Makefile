MK_WERROR = no
PROG = dyld_closure_util
.PATH: ${.CURDIR:H}
SRCS =  dyld3/shared-cache/dyld_closure_util.cpp \
	dyld3/shared-cache/FileUtils.cpp \
	dyld3/Diagnostics.cpp \
	dyld3/PathOverrides.cpp \
	dyld3/shared-cache/DyldSharedCache.cpp \
	dyld3/ClosureFileSystemPhysical.cpp \
	dyld3/Closure.cpp \
	dyld3/ClosureWriter.cpp \
	dyld3/ClosureBuilder.cpp \
	dyld3/MachOFile.cpp \
	dyld3/MachOLoaded.cpp \
	dyld3/MachOAnalyzer.cpp \
	dyld3/ClosurePrinter.cpp
MAN =

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DTARGET_OS_BRIDGE=0 -DWITHOUT_SANDBOX \
        -Wno-non-pod-varargs -Wno-nullability-completeness \
        -I${.CURDIR:H}/dyld3 \
        -I${RAVYN_SDKROOT}/usr/include/Private \
        -I${.CURDIR:H}/dyld3/shared-cache \
        -I${.CURDIR:H}/../Libsystem/private \
        -I${.CURDIR:H}/../Libsystem/liblaunch \
        -I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD} -lc++ -L${.OBJDIR:H}/libslc_builder -lslc_builder

.include <bsd.prog.mk>

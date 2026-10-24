MK_WERROR = no
PROG = dyld_shared_cache_util
SRCS =  dyld3/ClosureFileSystemPhysical.cpp \
	dyld3/Diagnostics.cpp \
	dyld3/Closure.cpp \
	dyld3/shared-cache/DyldSharedCache.cpp \
	dyld3/MachOFile.cpp \
	dyld3/MachOAnalyzer.cpp \
	launch-cache/dsc_iterator.cpp \
	dyld3/MachOLoaded.cpp \
	launch-cache/dyld_shared_cache_util.cpp


CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DBUILDING_SHARED_CACHE_UTIL -I${.CURDIR:H}/Libsystem/private

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

.include <bsd.prog.mk>

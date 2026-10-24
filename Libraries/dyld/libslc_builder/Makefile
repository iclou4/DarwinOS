MK_WERROR = no
SUBDIR =
.PATH: ${.CURDIR:H}
LIB = slc_builder
SRCS =  dyld3/shared-cache/DyldSharedCache.cpp \
	dyld3/ClosureFileSystemNull.cpp \
	dyld3/shared-cache/FileUtils.cpp \
	dyld3/shared-cache/OptimizerLinkedit.cpp \
	dyld3/ClosureWriter.cpp \
	dyld3/shared-cache/AdjustDylibSegments.cpp \
	dyld3/ClosureBuilder.cpp \
	dyld3/shared-cache/CacheBuilder.cpp \
	dyld3/MachOAnalyzer.cpp \
	dyld3/PathOverrides.cpp \
	dyld3/Diagnostics.cpp \
	dyld3/shared-cache/OptimizerObjC.cpp \
	dyld3/MachOFile.cpp \
	dyld3/MachOLoaded.cpp \
	dyld3/Closure.cpp \
	dyld3/shared-cache/SharedCacheBuilder.cpp \
	dyld3/shared-cache/mrm_shared_cache_builder.cpp \
	dyld3/shared-cache/OptimizerBranches.cpp

CXXSTD = none
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DBUILDING_CACHE_BUILDER -DTARGET_OS_BRIDGE=0 \
        -I${.CURDIR:H}/dyld3 -I${RAVYN_SDKROOT}/usr/include/Private \
        -I${.CURDIR:H}/dyld3/shared-cache -I${.CURDIR:H}/../Libsystem/private \
        -I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include \
        ${CFLAGS.${.TARGET:T}}

CFLAGS.OptimizerObjC.o = -std=c++14

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

.include <bsd.lib.mk>


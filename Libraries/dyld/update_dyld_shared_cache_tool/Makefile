MK_WERROR = no
PROG = update_dyld_shared_cache_tool
.PATH: ${.CURDIR:H}
SRCS =  dyld3/PathOverrides.cpp \
	dyld3/ClosureFileSystemNull.cpp \
	dyld3/Closure.cpp \
	dyld3/ClosureWriter.cpp \
	dyld3/ClosureFileSystemPhysical.cpp \
	dyld3/ClosureBuilder.cpp \
	dyld3/MachOFile.cpp \
	dyld3/MachOLoaded.cpp \
	dyld3/MachOAnalyzer.cpp \
	dyld3/shared-cache/update_dyld_shared_cache.cpp \
	dyld3/shared-cache/DyldSharedCache.cpp \
	dyld3/shared-cache/CacheBuilder.cpp \
	dyld3/shared-cache/AdjustDylibSegments.cpp \
	dyld3/shared-cache/FileUtils.cpp \
	dyld3/Diagnostics.cpp \
	dyld3/shared-cache/SharedCacheBuilder.cpp \
	dyld3/shared-cache/OptimizerObjC.cpp \
	dyld3/shared-cache/OptimizerBranches.cpp \
	dyld3/shared-cache/OptimizerLinkedit.cpp 
MAN =

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DTARGET_OS_BRIDGE=0 -DWITHOUT_SANDBOX -DDEBUG \
        -DBUILDING_CACHE_BUILDER -DBUILDING_UPDATE_DYLD_CACHE_BUILDER \
        -Wno-elaborated-enum-base \
        -I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include \
        -I${.CURDIR:H}/dyld3/shared-cache -I${.CURDIR:H}/dyld3 \
        -I${ROOT_SOURCE_DIR}/Kernel/IOKitUser/stubs/sandbox \
        -I${.CURDIR:H}/../Libsystem/private -I${.CURDIR} \
        -I${RAVYN_SDKROOT}/usr/include/Private

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

.include <bsd.prog.mk>

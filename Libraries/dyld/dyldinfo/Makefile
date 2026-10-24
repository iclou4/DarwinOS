MK_WERROR = no
PROG = dyldinfo
.PATH: ${.CURDIR:H}
SRCS =  dyld3/shared-cache/dyldinfo.cpp \
	dyld3/MachOFile.cpp \
	dyld3/MachOLoaded.cpp \
	dyld3/MachOAnalyzer.cpp \
	dyld3/ClosureFileSystemPhysical.cpp \
	dyld3/Diagnostics.cpp
MAN = # man page is in cctools/ld64

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DTARGET_OS_BRIDGE=0 -DWITHOUT_SANDBOX -DBUILDING_DYLDINFO -DDEBUG \
        -I${.CURDIR:H}/dyld3 -I${.CURDIR:H} \
        -I${ROOT_SOURCE_DIR}/Kernel/Extensions/corecrypto/include \
        -I${.CURDIR:H}/../Libsystem/private

LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD} -lc++ 

.include <bsd.prog.mk>

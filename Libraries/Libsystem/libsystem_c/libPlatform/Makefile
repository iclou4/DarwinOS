.PATH: ${.CURDIR:H}
SUBDIR = 
LIB = Platform
SRCS = darwin/forceLibcToBuild.c darwin/kvm.c
ACFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT}

.if "${CpuArch}" == "x86_64"
SRCS += x86_64/gen/mcount.s
.endif

.include <bsd.lib.mk>

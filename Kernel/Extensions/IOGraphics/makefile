MK_WERROR = no
SUBDIR =
KEXT = IOGOPFramebuffer
KERNEL_PRIVATE = yes
INFO_PLIST = Info-IOGOPFramebuffer.plist
BUNDLE_IDENTIFIER = com.ravynos.iokit.IOGOPFramebuffer
BUNDLE_VERSION = 1.0.0
MAIN_FUNCTION = IOGraphicsFamilyModuleStart

CSTD = gnu11
CXXSTD = gnu++11

.PATH: ${.CURDIR}/IOGraphicsFamily
.PATH: ${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/lzvndecode
SRCS = IOGOPFramebuffer.cpp \
        IOGraphicsDevice.cpp \
        IOBootFramebuffer.cpp \
        IOFramebuffer.cpp \
        IOI2CInterface.cpp \
        IODisplay.cpp \
        IODisplayWrangler.cpp \
        IOBacklightDisplay.cpp \
        IOKit/graphics/tl/osmemory.cpp \
        IOFramebufferUserClient.cpp \
        lzvn_decode.c

SLF = /System/Library/Frameworks
CFLAGS = -DKERNEL -DPRIVATE \
        -Wno-implicit-int-conversion -Wno-sign-conversion \
	-Wno-c++11-narrowing \
        -I${.CURDIR}/IOGraphicsFamily \
        -I${.CURDIR}/IOGraphicsFamily/IOKit/graphics/tl \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/iokit \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/osfmk \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern \
	-I${.CURDIR:H}/IOPCIFamily \
	-I${.CURDIR:H}/IOACPIFamily/include \
        -I${.CURDIR}/GTrace/Kernel -I${.CURDIR}/GMetric -I${.CURDIR} \
        -I${.CURDIR}/IONDRVSupport \
        -I${ROOT_SOURCE_DIR}/Developer/Default.xctoolchain/lzvndecode \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/osfmk \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/libkern \
	-I${ROOT_BINARY_DIR}${ROOT_SOURCE_DIR}/Kernel/xnu/EXPORT_HDRS/san \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/EXTERNAL_HEADERS
CFLAGS += -include${.CURDIR}/shims.h

#SUBDIR = IOGraphicsFamily IONDRVSupport

.include <rvn.kext.mk>
.include <bsd.subdir.mk>

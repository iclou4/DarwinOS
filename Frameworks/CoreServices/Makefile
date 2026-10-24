MK_WERROR = no
FRAMEWORK = CoreServices
FMWK_VERSION = A
SRCS = CoreServices.c
HEADERS = CoreServices.h 

RESOURCES=Info.plist English.lproj

CFLAGS = -I${.CURDIR:H}
SUBDIR =

fmwk-install-hook:
	mkdir -p ${RAVYN_SDKROOT}/System/Library/Frameworks \
	  ${SYSROOT_DIR}/System/Library/Frameworks
	cp -Rf ${.OBJDIR}/${FRAMEWORK}.framework ${RAVYN_SDKROOT}/System/Library/Frameworks
	cp -Rf ${.OBJDIR}/${FRAMEWORK}.framework ${SYSROOT_DIR}/System/Library/Frameworks

#SUBDIR = LaunchServices

.include <rvn.framework.mk>
.include <bsd.subdir.mk>

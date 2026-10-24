APP=	        LoginWindow
SRCS=           LoginWindow.m \
                DesktopWindow.m \
                LoginBox.m \
                Label.m 
RELEASE_LOGO=	hyena.png
RESOURCES=      ${SRCTOP}/Logo_Assets/darwinos-full.png \
		${SRCTOP}/Logo_Assets/${RELEASE_LOGO}

MK_WERROR=	no
MK_PIE=		no
WARNS=	        1
OBJCFLAGS= -fobjc-runtime=gnustep-2.0 -fobjc-nonfragile-abi -fPIC \
	   -Wno-missing-method-return-type -Wno-macro-redefined
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR} \
		--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
		${OBJCFLAGS} -I${SRCTOP}/Frameworks -D__MACH__ \
		-I${MAKEOBJDIR}/../SystemUIServer -D__DARWINOS__ \
		-I${SRCTOP}/Frameworks/AppKit/Headers -I${SRCTOP}/CoreServices \
		-I${OBJTOP}/Frameworks/Foundation/Headers \
		-I${SRCTOP}/gnu/lib/libobjc2 \
		-DRELEASE_LOGO="@\"${RELEASE_LOGO}\""
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' -L${OBJTOP}/tmp/usr/lib \
		-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-L${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/CoreGraphics.framework/Versions/A \
                -lAppKit -lFoundation -lCoreGraphics -lobjc -lSystem -lcrypt -lm

.include <rvn.app.mk>
${OBJS}: obj

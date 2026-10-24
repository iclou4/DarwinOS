APP=Install_darwinOS
SRCS=Source/main.m Source/AppDelegate.m Source/GSGeomDisk.m
MK_DEBUG_FILES=no
RESOURCES=${.CURDIR}/MarmosetLogo.tiff ${.CURDIR}/header.rtf ${.CURDIR}/terms.rtf \
        ${.CURDIR}/English.lproj ${.CURDIR}/Icon.png
DARWINOS_VERSION!= head -1 ${.CURDIR}/../../version.txt

MK_WERROR= no
WARNS= 1
CFLAGS= -g -DDARWINOS_VERSION=\"${DARWINOS_VERSION}\" \
	--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
	-Wno-missing-method-return-type \
	-I${OBJTOP}/CoreServices/WindowServer/SystemUIServer \
	-I${SRCTOP}/Frameworks -I${OBJTOP}/Frameworks/Foundation/Headers \
	-I${SRCTOP}/Frameworks/AppKit/Headers
LDFLAGS= -L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
	 -L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
	 -L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework \
	 -L${OBJTOP}/Frameworks/Cocoa/Cocoa.framework \
	 -Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
	 -Wl,-R/System/Library/Frameworks/Cocoa.framework/Versions/A \
	 -Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/A \
	 -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
	 -lCocoa -lAppKit -lFoundation -lobjc -lSystem

clean:
	rm -rf ${APP_DIR} "${APP_DIR:S/_/ /}"
	rm -f Source/*.o

makedirs:
	mkdir -p ${MAKEOBJDIR}/Source

build: clean makedirs obj all
	rm -f ${APP_DIR}/${APP}
	ln -sf "Contents/darwinOS/${APP:S/_/ /}" "${APP_DIR}/${APP:S/_/ /}"
	mv -f ${APP_DIR}/Contents/darwinOS/${APP} "${APP_DIR}/Contents/darwinOS/${APP:S/_/ /}"
	mv -f ${APP_DIR} "${APP_DIR:S/_/ /}"

.include <rvn.app.mk>

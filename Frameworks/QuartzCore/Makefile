FRAMEWORK=QuartzCore
MK_WERROR=no

SRCS= \
	CAAnimation.m \
	CAAnimationGroup.m \
	CABase.m \
	CABasicAnimation.m \
	CALayer.m \
	CALayerContext.m \
	CAMediaTimingFunction.m \
	CAPropertyAnimation.m \
	CARenderer.m \
	CATransaction.m \
	CATransactionGroup.m \
	CATransform3D.m \
	CATransition.m \
	CAValueFunction.m \
	CAWindowOpenGLContext.m \
	CIAffineTransform.m \
	CIBloom.m \
	CIBoxBlur.m \
	CIColor.m \
	CIContext.m \
	CIFilter.m \
	CIImage.m \
	CILinearGradient.m \
	CIVector.m \
	CAMetalLayer.m

HEADERS= \
	CAAction.h \
	CAAnimation.h \
	CAAnimationGroup.h \
	CABase.h \
	CABasicAnimation.h \
	CALayer.h \
	CALayerContext.h \
	CAMediaTiming.h \
	CAMediaTimingFunction.h \
	CAPropertyAnimation.h \
	CIBloom.h \
	CARenderer.h \
	CATransaction.h \
	CATransactionGroup.h \
	CATransform3D.h \
	CATransition.h \
	CAValueFunction.h \
	CAWindowOpenGLContext.h \
	CIAffineTransform.h \
	CIBoxBlur.h \
	CIColor.h \
	CIContext.h \
	CIFilter.h \
	CIImage.h \
	CILinearGradient.h \
	CIVector.h \
	CAMetalLayer.h \
	CoreAnimation.h \
	CoreImage.h \
	CoreVideo.h \
	QuartzCore.h 

RESOURCES=Info.plist English.lproj
CFLAGS+= -I${.CURDIR}/.. -I${.CURDIR} \
	-I${ROOT_SOURCE_DIR}/Frameworks \
	-I${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers \
	-I${ROOT_SOURCE_DIR}/Frameworks/AppKit/Headers \
	-I${ROOT_SOURCE_DIR}/Frameworks/Metal/include \
	-I${ROOT_SOURCE_DIR}/Frameworks/OpenGL/include \
	-I${ROOT_SOURCE_DIR}/Frameworks/Onyx2D \
	-F${RAVYN_SDKROOT}/System/Library/Frameworks \
	-fblocks -Wno-unused-parameter -Wno-nullability-completeness -Wno-strict-prototypes \
	-Wno-error=implicit-function-declaration -Wno-implicit-function-declaration \
	-Wno-error=deprecated-objc-isa-usage -Wno-deprecated-objc-isa-usage
LDFLAGS+= ${FMWK_LDFLAGS} \
	-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework \
	-F${RAVYN_SDKROOT}/System/Library/Frameworks \
	-framework Metal -framework Foundation -framework CoreFoundation -framework CoreGraphics -framework Onyx2D -framework OpenGL -lobjc \
	-Wl,-undefined,dynamic_lookup

INSTALL_NAME_DIR = /System/Library/Frameworks/${FRAMEWORK}.framework/Versions/${FMWK_VERSION}

fmwk-install-hook:
.for dir in ${RAVYN_SDKROOT}/System/Library/Frameworks/ ${SYSROOT_DIR}/System/Library/Frameworks/
	mkdir -p ${dir}
	cp -Rf ${.OBJDIR}/${FRAMEWORK}.framework ${dir}
.endfor

.include <rvn.framework.mk>

FRAMEWORK = Metal
SUBDIR =
MK_WERROR = no

SRCS = \
	src/mtl_raster.c \
	src/mtl_core.c \
	src/mtl_command.c \
	src/_MTLDevice.m \
	src/MTLSoftwareDevice.m \
	src/MTLSoftwareCommandQueue.m \
	src/MTLSoftwareCommandBuffer.m \
	src/MTLSoftwareRenderCommandEncoder.m \
	src/MTLSoftwareBuffer.m \
	src/MTLSoftwareTexture.m \
	src/MTLSoftwareSampler.m \
	src/MTLSoftwareLibrary.m \
	src/MTLSoftwareRenderPipeline.m \
	src/MTLSoftwareDrawable.m \
	src/MTLSoftwareDescriptors.m \
	src/MTLSoftwareDepthStencil.m \
	src/MTLIOAccelDevice.m \
	src/MTLIOAccelCommandQueue.m \
	src/MTLIOAccelCommandBuffer.m \
	src/MTLIOAccelRenderCommandEncoder.m \
	src/MTLIOAccelResource.m \
	src/MTLIOAccelBuffer.m \
	src/MTLIOAccelTexture.m

HEADERS = \
	include/Metal/Metal.h \
	include/Metal/MTLDefines.h \
	include/Metal/MTLTypes.h \
	include/Metal/MTLPixelFormat.h \
	include/Metal/MTLResource.h \
	include/Metal/MTLBuffer.h \
	include/Metal/MTLTexture.h \
	include/Metal/MTLSampler.h \
	include/Metal/MTLLibrary.h \
	include/Metal/MTLVertexDescriptor.h \
	include/Metal/MTLRenderPipeline.h \
	include/Metal/MTLComputePipeline.h \
	include/Metal/MTLDepthStencil.h \
	include/Metal/MTLRenderPass.h \
	include/Metal/MTLDrawable.h \
	include/Metal/MTLCommandEncoder.h \
	include/Metal/MTLRenderCommandEncoder.h \
	include/Metal/MTLComputeCommandEncoder.h \
	include/Metal/MTLBlitCommandEncoder.h \
	include/Metal/MTLCommandBuffer.h \
	include/Metal/MTLCommandQueue.h \
	include/Metal/MTLDevice.h \
	include/Metal/MTLFunctionConstantValues.h \
	include/Metal/MTLShaders.h \
	include/Metal/MTLSoftware.h \
	include/Metal/_MTLDevice.h \
	include/Metal/MTLIOAccelDevice.h \
	include/Metal/MTLIOAccelCommandQueue.h \
	include/Metal/MTLIOAccelCommandBuffer.h \
	include/Metal/MTLIOAccelRenderCommandEncoder.h \
	include/Metal/MTLIOAccelResource.h \
	include/Metal/MTLIOAccelBuffer.h \
	include/Metal/MTLIOAccelTexture.h \
	include/Metal/metal_stdlib.h

RESOURCES = Info.plist

CFLAGS += -I${.CURDIR}/include -I${.CURDIR}/src \
	-I${ROOT_SOURCE_DIR}/Frameworks -I${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers \
	-fblocks -Wno-unused-parameter -Wno-nullability-completeness -Wno-objc-missing-super-calls

LDFLAGS += -framework Foundation -framework CoreFoundation -lm \
	-Wl,-undefined,dynamic_lookup

INSTALL_NAME_DIR = /System/Library/Frameworks/${FRAMEWORK}.framework/Versions/${FMWK_VERSION}

fmwk-install-hook:
.for dir in ${RAVYN_SDKROOT}/System/Library/Frameworks/ ${SYSROOT_DIR}/System/Library/Frameworks/
	mkdir -p ${dir}
	cp -Rf ${.OBJDIR}/${FRAMEWORK}.framework ${dir}
.endfor

.include <rvn.framework.mk>

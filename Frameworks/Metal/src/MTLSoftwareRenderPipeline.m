/*
 * MTLSoftwareRenderPipeline.m - ravynOS Metal: render pipeline implementation
 */

#import "MTLSoftwareRenderPipeline.h"
#import "MTLSoftwareLibrary.h"

@implementation MTLSoftwareRenderPipelineState

@synthesize label = _label;
@synthesize device = _device;
@synthesize cPipeline = _cPipeline;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLRenderPipelineDescriptor *)descriptor
{
	self = [super init];
	if (self) {
		_device = device;
		_label = [descriptor.label copy];

		mtl_render_pipeline_descriptor cDesc;
		memset(&cDesc, 0, sizeof(cDesc));

		if ([descriptor.vertexFunction isKindOfClass:[MTLSoftwareFunction class]]) {
			cDesc.vertex_function = ((MTLSoftwareFunction *)descriptor.vertexFunction).vertexFunction;
		}
		if ([descriptor.fragmentFunction isKindOfClass:[MTLSoftwareFunction class]]) {
			cDesc.fragment_function = ((MTLSoftwareFunction *)descriptor.fragmentFunction).fragmentFunction;
		}

		MTLRenderPipelineColorAttachmentDescriptor *color0 = descriptor.colorAttachments[0];
		if (color0) {
			cDesc.color_attachment_format = color0.pixelFormat;
			cDesc.color_attachment.blending_enabled = color0.isBlendingEnabled;
			cDesc.color_attachment.source_rgb = color0.sourceRGBBlendFactor;
			cDesc.color_attachment.destination_rgb = color0.destinationRGBBlendFactor;
			cDesc.color_attachment.operation_rgb = color0.rgbBlendOperation;
			cDesc.color_attachment.source_alpha = color0.sourceAlphaBlendFactor;
			cDesc.color_attachment.destination_alpha = color0.destinationAlphaBlendFactor;
			cDesc.color_attachment.operation_alpha = color0.alphaBlendOperation;
			cDesc.color_attachment.write_mask = (unsigned int)color0.writeMask;
		}
		cDesc.depth_attachment_format = descriptor.depthAttachmentPixelFormat;

		_cPipeline = mtl_device_new_render_pipeline(NULL, &cDesc);
	}
	return self;
}

- (void)dealloc
{
	if (_cPipeline) {
		mtl_render_pipeline_release(_cPipeline);
	}
}

@end

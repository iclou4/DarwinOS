/*
 * MTLSoftwareRenderPipeline.h - ravynOS Metal: render pipeline implementation
 */

#ifndef MTL_SOFTWARE_RENDER_PIPELINE_H
#define MTL_SOFTWARE_RENDER_PIPELINE_H

#import <Foundation/Foundation.h>
#import <Metal/MTLRenderPipeline.h>
#import "mtl_internal.h"

@interface MTLSoftwareRenderPipelineState : NSObject <MTLRenderPipelineState> {
@public
	id<MTLDevice> _device;
	NSString *_label;
	mtl_render_pipeline_t *_cPipeline;
}

@property (readonly) mtl_render_pipeline_t *cPipeline;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLRenderPipelineDescriptor *)descriptor;

@end

#endif /* MTL_SOFTWARE_RENDER_PIPELINE_H */

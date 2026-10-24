/*
 * MTLSoftwareRenderCommandEncoder.h - ravynOS Metal: render command encoder
 */

#ifndef MTL_SOFTWARE_RENDER_COMMAND_ENCODER_H
#define MTL_SOFTWARE_RENDER_COMMAND_ENCODER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLRenderCommandEncoder.h>
#import <Metal/MTLCommandBuffer.h>
#import "mtl_internal.h"

@interface MTLSoftwareRenderCommandEncoder : NSObject <MTLRenderCommandEncoder> {
@public
	id<MTLCommandBuffer> _commandBuffer;
	NSString *_label;
	mtl_render_encoder_t *_cEncoder;
}

@property (readonly) mtl_render_encoder_t *cEncoder;

- (instancetype)initWithCommandBuffer:(id<MTLCommandBuffer>)commandBuffer descriptor:(MTLRenderPassDescriptor *)descriptor cEncoder:(mtl_render_encoder_t *)cEncoder;

@end

#endif /* MTL_SOFTWARE_RENDER_COMMAND_ENCODER_H */

/*
 * MTLSoftwareDepthStencil.h - ravynOS Metal: depth stencil state header
 */

#ifndef MTL_SOFTWARE_DEPTH_STENCIL_H
#define MTL_SOFTWARE_DEPTH_STENCIL_H

#import <Metal/MTLDepthStencil.h>

@interface MTLSoftwareDepthStencilState : NSObject <MTLDepthStencilState> {
@public
	id<MTLDevice> _device;
	NSString *_label;
	MTLCompareFunction _compareFunction;
	BOOL _depthWriteEnabled;
}
@property (nullable, readonly) NSString *label;
@property (readonly) id<MTLDevice> device;
@property (readonly) MTLCompareFunction compareFunction;
@property (readonly) BOOL depthWriteEnabled;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLDepthStencilDescriptor *)descriptor;
@end

#endif /* MTL_SOFTWARE_DEPTH_STENCIL_H */

/*
 * MTLSoftwareSampler.h - ravynOS Metal: sampler state implementation
 */

#ifndef MTL_SOFTWARE_SAMPLER_H
#define MTL_SOFTWARE_SAMPLER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLSampler.h>
#import "mtl_internal.h"

@interface MTLSoftwareSamplerState : NSObject <MTLSamplerState> {
@public
	id<MTLDevice> _device;
	NSString *_label;
	mtl_sampler_t *_cSampler;
}

@property (readonly) mtl_sampler_t *cSampler;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLSamplerDescriptor *)descriptor;

@end

#endif /* MTL_SOFTWARE_SAMPLER_H */

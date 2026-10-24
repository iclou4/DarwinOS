/*
 * MTLComputeCommandEncoder.h - ravynOS Metal: compute encoder protocol
 */

#ifndef MTL_COMPUTE_COMMAND_ENCODER_H
#define MTL_COMPUTE_COMMAND_ENCODER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLCommandEncoder.h>
#import <Metal/MTLTypes.h>

@protocol MTLBuffer;
@protocol MTLTexture;
@protocol MTLSamplerState;
@protocol MTLComputePipelineState;

@protocol MTLComputeCommandEncoder <MTLCommandEncoder>

- (void)setComputePipelineState:(id<MTLComputePipelineState>)state;

- (void)setBuffer:(nullable id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index;
- (void)setTexture:(nullable id<MTLTexture>)texture atIndex:(NSUInteger)index;
- (void)setSamplerState:(nullable id<MTLSamplerState>)sampler atIndex:(NSUInteger)index;

- (void)dispatchThreadgroups:(MTLSize)threadgroupsPerGrid
       threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup;

- (void)dispatchThreads:(MTLSize)threadsPerGrid
  threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup;
@end

#endif /* MTL_COMPUTE_COMMAND_ENCODER_H */

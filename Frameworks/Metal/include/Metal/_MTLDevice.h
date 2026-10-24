/*
 * _MTLDevice.h - ravynOS Metal: internal base device class
 */

#ifndef _MTL_DEVICE_INTERNAL_H
#define _MTL_DEVICE_INTERNAL_H

#import <Foundation/Foundation.h>
#import <Metal/MTLDevice.h>
#import <Metal/MTLCommandQueue.h>
#import <Metal/MTLBuffer.h>
#import <Metal/MTLTexture.h>
#import <Metal/MTLSampler.h>
#import <Metal/MTLLibrary.h>
#import <Metal/MTLRenderPipeline.h>
#import <Metal/MTLComputePipeline.h>
#import <Metal/MTLDepthStencil.h>

@interface _MTLDevice : NSObject <MTLDevice> {
@protected
	NSString *_name;
	uint64_t _registryID;
	MTLSize _maxThreadsPerThreadgroup;
	BOOL _hasUnifiedMemory;
	BOOL _lowPower;
	BOOL _headless;
	BOOL _removable;
	uint64_t _recommendedMaxWorkingSetSize;
	MTLDeviceLocation _location;
	NSUInteger _locationNumber;
	NSUInteger _maxBufferLength;
}

@property (readonly) NSString *name;
@property (readonly) uint64_t registryID;
@property (readonly) MTLSize maxThreadsPerThreadgroup;
@property (readonly, getter=isLowPower) BOOL lowPower;
@property (readonly, getter=isHeadless) BOOL headless;
@property (readonly, getter=isRemovable) BOOL removable;
@property (readonly) BOOL hasUnifiedMemory;
@property (readonly) uint64_t recommendedMaxWorkingSetSize;
@property (readonly) MTLDeviceLocation location;
@property (readonly) NSUInteger locationNumber;
@property (readonly) NSUInteger maxBufferLength;

- (BOOL)supportsFeatureSet:(MTLFeatureSet)featureSet;
- (BOOL)supportsFamily:(MTLGPUFamily)gpuFamily;

- (nullable id<MTLCommandQueue>)newCommandQueue;
- (nullable id<MTLCommandQueue>)newCommandQueueWithMaxCommandBufferCount:(NSUInteger)maxCommandBufferCount;

- (nullable id<MTLBuffer>)newBufferWithLength:(NSUInteger)length options:(MTLResourceOptions)options;
- (nullable id<MTLBuffer>)newBufferWithBytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options;
- (nullable id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor;
- (nullable id<MTLSamplerState>)newSamplerStateWithDescriptor:(MTLSamplerDescriptor *)descriptor;

- (nullable id<MTLLibrary>)newDefaultLibrary;
- (nullable id<MTLLibrary>)newLibraryWithFile:(NSString *)filepath error:(__autoreleasing NSError **)error;
- (nullable id<MTLLibrary>)newLibraryWithSource:(NSString *)source options:(nullable MTLCompileOptions *)options error:(__autoreleasing NSError **)error;

- (nullable id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)descriptor error:(__autoreleasing NSError **)error;
- (nullable id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)descriptor options:(MTLPipelineOption)options reflection:(MTLComputePipelineReflection * _Nullable * _Nullable)reflection error:(__autoreleasing NSError **)error;
- (nullable id<MTLComputePipelineState>)newComputePipelineStateWithFunction:(id<MTLFunction>)computeFunction error:(__autoreleasing NSError **)error;
- (nullable id<MTLDepthStencilState>)newDepthStencilStateWithDescriptor:(MTLDepthStencilDescriptor *)descriptor;

@end

#endif /* _MTL_DEVICE_INTERNAL_H */

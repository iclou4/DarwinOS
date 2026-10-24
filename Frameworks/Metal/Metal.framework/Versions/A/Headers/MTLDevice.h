/*
 * MTLDevice.h - ravynOS Metal: device protocol and factory function
 */

#ifndef MTL_DEVICE_H
#define MTL_DEVICE_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLTypes.h>
#import <Metal/MTLResource.h>
#import <Metal/MTLRenderPipeline.h>

typedef NS_ENUM(NSUInteger, MTLFeatureSet) {
	MTLFeatureSet_macOS_GPUFamily1_v1 = 10000,
	MTLFeatureSet_macOS_GPUFamily1_v2 = 10001,
	MTLFeatureSet_macOS_GPUFamily1_v3 = 10002,
	MTLFeatureSet_macOS_GPUFamily1_v4 = 10003,
	MTLFeatureSet_macOS_GPUFamily2_v1 = 10004,
};

typedef NS_ENUM(NSUInteger, MTLDeviceLocation) {
	MTLDeviceLocationBuiltIn = 0,
	MTLDeviceLocationSlot = 1,
	MTLDeviceLocationExternal = 2,
	MTLDeviceLocationUnspecified = 0xFFFFFFFF,
};

typedef NS_ENUM(NSInteger, MTLGPUFamily) {
	MTLGPUFamilyApple1 = 1001,
	MTLGPUFamilyApple2 = 1002,
	MTLGPUFamilyApple3 = 1003,
	MTLGPUFamilyApple4 = 1004,
	MTLGPUFamilyApple5 = 1005,
	MTLGPUFamilyApple6 = 1006,
	MTLGPUFamilyApple7 = 1007,
	MTLGPUFamilyMac1 = 2001,
	MTLGPUFamilyMac2 = 2002,
	MTLGPUFamilyCommon1 = 3001,
	MTLGPUFamilyCommon2 = 3002,
	MTLGPUFamilyCommon3 = 3003,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLCommandQueue;
@protocol MTLBuffer;
@protocol MTLTexture;
@protocol MTLSamplerState;
@protocol MTLLibrary;
@protocol MTLFunction;
@protocol MTLRenderPipelineState;
@protocol MTLComputePipelineState;
@protocol MTLDepthStencilState;

@class MTLTextureDescriptor;
@class MTLSamplerDescriptor;
@class MTLRenderPipelineDescriptor;
@class MTLComputePipelineDescriptor;
@class MTLDepthStencilDescriptor;
@class MTLCompileOptions;
@class MTLComputePipelineReflection;

@protocol MTLDevice <NSObject>
@property (readonly) NSString *name;
@property (readonly) uint64_t registryID;
@property (readonly) MTLSize maxThreadsPerThreadgroup;
@property (readonly, getter=isLowPower) BOOL lowPower;
@property (readonly, getter=isHeadless) BOOL headless;
@property (readonly, getter=isRemovable) BOOL removable;
@property (readonly) BOOL hasUnifiedMemory;
@property (readonly) uint64_t recommendedMaxWorkingSetSize;

- (nullable id<MTLCommandQueue>)newCommandQueue;
- (nullable id<MTLCommandQueue>)newCommandQueueWithMaxCommandBufferCount:(NSUInteger)maxCommandBufferCount;

- (nullable id<MTLBuffer>)newBufferWithLength:(NSUInteger)length options:(MTLResourceOptions)options;
- (nullable id<MTLBuffer>)newBufferWithBytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options;
- (nullable id<MTLBuffer>)newBufferWithBytesNoCopy:(void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options deallocator:(nullable void (^)(void *pointer, NSUInteger length))deallocator;

- (nullable id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor;

- (nullable id<MTLSamplerState>)newSamplerStateWithDescriptor:(MTLSamplerDescriptor *)descriptor;

- (nullable id<MTLLibrary>)newDefaultLibrary;
- (nullable id<MTLLibrary>)newLibraryWithFile:(NSString *)filepath error:(__autoreleasing NSError **)error;
- (nullable id<MTLLibrary>)newLibraryWithURL:(NSURL *)url error:(__autoreleasing NSError **)error;
- (nullable id<MTLLibrary>)newLibraryWithData:(NSData *)data error:(__autoreleasing NSError **)error;
- (nullable id<MTLLibrary>)newLibraryWithSource:(NSString *)source options:(nullable MTLCompileOptions *)options error:(__autoreleasing NSError **)error;

- (nullable id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)descriptor error:(__autoreleasing NSError **)error;

- (nullable id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)descriptor options:(MTLPipelineOption)options reflection:(MTLComputePipelineReflection * _Nullable * _Nullable)reflection error:(__autoreleasing NSError **)error;
- (nullable id<MTLComputePipelineState>)newComputePipelineStateWithFunction:(id<MTLFunction>)computeFunction error:(__autoreleasing NSError **)error;

- (nullable id<MTLDepthStencilState>)newDepthStencilStateWithDescriptor:(MTLDepthStencilDescriptor *)descriptor;

- (BOOL)supportsFeatureSet:(MTLFeatureSet)featureSet;
@end

#ifdef __cplusplus
extern "C" {
#endif

MTL_EXPORT id<MTLDevice> _Nullable MTLCreateSystemDefaultDevice(void);
MTL_EXPORT NSArray * _Nonnull MTLCopyAllDevices(void);

#ifdef __cplusplus
}
#endif

#endif /* __OBJC__ */

#endif /* MTL_DEVICE_H */

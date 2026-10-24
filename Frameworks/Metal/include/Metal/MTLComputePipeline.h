/*
 * MTLComputePipeline.h - ravynOS Metal: compute pipeline descriptor and state
 */

#ifndef MTL_COMPUTE_PIPELINE_H
#define MTL_COMPUTE_PIPELINE_H

#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLFunction;

@interface MTLComputePipelineDescriptor : NSObject <NSCopying>
@property (nullable, copy, nonatomic) NSString *label;
@property (nullable, readwrite, strong, nonatomic) id<MTLFunction> computeFunction;
@property (readwrite, nonatomic) BOOL threadGroupSizeIsMultipleOfThreadExecutionWidth;
- (void)reset;
@end

@protocol MTLComputePipelineState <NSObject>
@property (nullable, readonly) NSString *label;
@property (readonly) id<MTLDevice> device;
@property (readonly) NSUInteger maxTotalThreadsPerThreadgroup;
@property (readonly) NSUInteger threadExecutionWidth;
@end

#endif /* MTL_COMPUTE_PIPELINE_H */

/*
 * MTLDepthStencil.h - ravynOS Metal: depth and stencil descriptors and states
 */

#ifndef MTL_DEPTH_STENCIL_H
#define MTL_DEPTH_STENCIL_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLTypes.h>

typedef NS_ENUM(NSUInteger, MTLCompareFunction) {
	MTLCompareFunctionNever        = 0,
	MTLCompareFunctionLess         = 1,
	MTLCompareFunctionEqual        = 2,
	MTLCompareFunctionLessEqual    = 3,
	MTLCompareFunctionGreater      = 4,
	MTLCompareFunctionNotEqual     = 5,
	MTLCompareFunctionGreaterEqual = 6,
	MTLCompareFunctionAlways       = 7,
};

typedef NS_ENUM(NSUInteger, MTLStencilOperation) {
	MTLStencilOperationKeep           = 0,
	MTLStencilOperationZero           = 1,
	MTLStencilOperationReplace        = 2,
	MTLStencilOperationIncrementClamp = 3,
	MTLStencilOperationDecrementClamp = 4,
	MTLStencilOperationInvert         = 5,
	MTLStencilOperationIncrementWrap  = 6,
	MTLStencilOperationDecrementWrap  = 7,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;

@interface MTLStencilDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) MTLStencilOperation stencilFailureOperation;
@property (readwrite, nonatomic) MTLStencilOperation depthFailureOperation;
@property (readwrite, nonatomic) MTLStencilOperation depthNormalHandling;
@property (readwrite, nonatomic) MTLCompareFunction stencilCompareFunction;
@property (readwrite, nonatomic) uint32_t readMask;
@property (readwrite, nonatomic) uint32_t writeMask;
@end

@interface MTLDepthStencilDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) MTLCompareFunction depthCompareFunction;
@property (readwrite, nonatomic, getter=isDepthWriteEnabled) BOOL depthWriteEnabled;
@property (nullable, copy, nonatomic) MTLStencilDescriptor *frontFaceStencil;
@property (nullable, copy, nonatomic) MTLStencilDescriptor *backFaceStencil;
@property (nullable, copy, nonatomic) NSString *label;
@end

@protocol MTLDepthStencilState <NSObject>
@property (nullable, readonly) NSString *label;
@property (readonly) id<MTLDevice> device;
@end
#endif /* __OBJC__ */

#endif /* MTL_DEPTH_STENCIL_H */

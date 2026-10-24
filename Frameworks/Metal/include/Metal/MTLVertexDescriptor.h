/*
 * MTLVertexDescriptor.h - ravynOS Metal: vertex layout descriptions
 */

#ifndef MTL_VERTEX_DESCRIPTOR_H
#define MTL_VERTEX_DESCRIPTOR_H

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, MTLVertexFormat) {
	MTLVertexFormatInvalid = 0,
	MTLVertexFormatUChar2  = 1,
	MTLVertexFormatUChar3  = 2,
	MTLVertexFormatUChar4  = 3,
	MTLVertexFormatChar2   = 4,
	MTLVertexFormatChar3   = 5,
	MTLVertexFormatChar4   = 6,
	MTLVertexFormatFloat   = 28,
	MTLVertexFormatFloat2  = 29,
	MTLVertexFormatFloat3  = 30,
	MTLVertexFormatFloat4  = 31,
	MTLVertexFormatInt     = 32,
	MTLVertexFormatInt2    = 33,
	MTLVertexFormatInt3    = 34,
	MTLVertexFormatInt4    = 35,
};

typedef NS_ENUM(NSUInteger, MTLVertexStepFunction) {
	MTLVertexStepFunctionConstant              = 0,
	MTLVertexStepFunctionPerVertex             = 1,
	MTLVertexStepFunctionPerInstance           = 2,
	MTLVertexStepFunctionPerPatch              = 3,
	MTLVertexStepFunctionPerPatchControlPoint  = 4,
};

@interface MTLVertexBufferLayoutDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) NSUInteger stride;
@property (readwrite, nonatomic) MTLVertexStepFunction stepFunction;
@property (readwrite, nonatomic) NSUInteger stepRate;
@end

@interface MTLVertexBufferLayoutDescriptorArray : NSObject
- (MTLVertexBufferLayoutDescriptor *)objectAtIndexedSubscript:(NSUInteger)index;
- (void)setObject:(nullable MTLVertexBufferLayoutDescriptor *)bufferDesc atIndexedSubscript:(NSUInteger)index;
@end

@interface MTLVertexAttributeDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) MTLVertexFormat format;
@property (readwrite, nonatomic) NSUInteger offset;
@property (readwrite, nonatomic) NSUInteger bufferIndex;
@end

@interface MTLVertexAttributeDescriptorArray : NSObject
- (MTLVertexAttributeDescriptor *)objectAtIndexedSubscript:(NSUInteger)index;
- (void)setObject:(nullable MTLVertexAttributeDescriptor *)attributeDesc atIndexedSubscript:(NSUInteger)index;
@end

@interface MTLVertexDescriptor : NSObject <NSCopying>
@property (readonly, nonatomic) MTLVertexBufferLayoutDescriptorArray *layouts;
@property (readonly, nonatomic) MTLVertexAttributeDescriptorArray *attributes;

+ (MTLVertexDescriptor *)vertexDescriptor;
- (void)reset;
@end

#endif /* MTL_VERTEX_DESCRIPTOR_H */

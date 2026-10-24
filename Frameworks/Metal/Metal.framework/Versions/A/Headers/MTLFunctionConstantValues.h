/*
 * MTLFunctionConstantValues.h - ravynOS Metal: function constants
 */

#ifndef MTL_FUNCTION_CONSTANT_VALUES_H
#define MTL_FUNCTION_CONSTANT_VALUES_H

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, MTLDataType) {
	MTLDataTypeNone     = 0,
	MTLDataTypeStruct   = 1,
	MTLDataTypeArray    = 2,
	MTLDataTypeFloat    = 3,
	MTLDataTypeFloat2   = 4,
	MTLDataTypeFloat3   = 5,
	MTLDataTypeFloat4   = 6,
	MTLDataTypeInt      = 29,
	MTLDataTypeInt2     = 30,
	MTLDataTypeInt3     = 31,
	MTLDataTypeInt4     = 32,
	MTLDataTypeUInt     = 33,
	MTLDataTypeUInt2    = 34,
	MTLDataTypeUInt3    = 35,
	MTLDataTypeUInt4    = 36,
	MTLDataTypeBool     = 53,
};

@interface MTLFunctionConstantValues : NSObject <NSCopying>
- (void)setConstantValue:(const void *)value type:(MTLDataType)type atIndex:(NSUInteger)index;
- (void)setConstantValues:(const void *)values type:(MTLDataType)type withRange:(NSRange)range;
- (void)setConstantValue:(const void *)value type:(MTLDataType)type withName:(NSString *)name;
- (void)reset;
@end

#endif /* MTL_FUNCTION_CONSTANT_VALUES_H */

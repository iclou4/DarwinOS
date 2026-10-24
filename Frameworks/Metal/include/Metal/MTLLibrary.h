/*
 * MTLLibrary.h - ravynOS Metal: function and library protocols
 */

#ifndef MTL_LIBRARY_H
#define MTL_LIBRARY_H

#import <Metal/MTLDefines.h>

typedef NS_ENUM(NSUInteger, MTLFunctionType) {
	MTLFunctionTypeVertex   = 1,
	MTLFunctionTypeFragment = 2,
	MTLFunctionTypeKernel   = 3,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLFunction;

@protocol MTLFunction <NSObject>
@property (nullable, copy) NSString *label;
@property (readonly) id<MTLDevice> device;
@property (readonly) MTLFunctionType functionType;
@property (readonly) NSString *name;
@end

@protocol MTLLibrary <NSObject>
@property (nullable, copy) NSString *label;
@property (readonly) id<MTLDevice> device;
@property (readonly) NSArray *functionNames;

- (nullable id<MTLFunction>)newFunctionWithName:(NSString *)functionName;
@end

@interface MTLCompileOptions : NSObject <NSCopying>
@property (nullable, copy, nonatomic) NSDictionary *preprocessorMacros;
@property (readwrite, nonatomic) BOOL fastMathEnabled;
@property (readwrite, nonatomic) NSUInteger languageVersion;
@end
#endif /* __OBJC__ */

#endif /* MTL_LIBRARY_H */

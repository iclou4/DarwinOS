/*
 * MTLSoftwareLibrary.h - ravynOS Metal: library and function implementation
 */

#ifndef MTL_SOFTWARE_LIBRARY_H
#define MTL_SOFTWARE_LIBRARY_H

#import <Foundation/Foundation.h>
#import <Metal/MTLLibrary.h>
#import "mtl_internal.h"

@interface MTLSoftwareFunction : NSObject <MTLFunction> {
@public
	id<MTLDevice> _device;
	NSString *_name;
	NSString *_label;
	MTLFunctionType _type;
	mtl_vertex_function _vertexFunction;
	mtl_fragment_function _fragmentFunction;
}

@property (readonly) mtl_vertex_function vertexFunction;
@property (readonly) mtl_fragment_function fragmentFunction;

- (instancetype)initWithDevice:(id<MTLDevice>)device name:(NSString *)name type:(MTLFunctionType)type vertexFunc:(mtl_vertex_function)vf fragmentFunc:(mtl_fragment_function)ff;

@end

@interface MTLSoftwareLibrary : NSObject <MTLLibrary> {
@public
	id<MTLDevice> _device;
	NSString *_label;
	mtl_library_t *_cLibrary;
	NSMutableDictionary *_functions;
	void *_dlHandle;
}

@property (readonly) mtl_library_t *cLibrary;

- (instancetype)initWithDevice:(id<MTLDevice>)device;
- (instancetype)initWithDevice:(id<MTLDevice>)device dylibPath:(NSString *)path;
- (void)registerVertexFunction:(mtl_vertex_function)func withName:(NSString *)name;
- (void)registerFragmentFunction:(mtl_fragment_function)func withName:(NSString *)name;

@end

#endif /* MTL_SOFTWARE_LIBRARY_H */

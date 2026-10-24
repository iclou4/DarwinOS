/*
 * MTLSoftwareLibrary.m - ravynOS Metal: library and function implementation
 */

#import "MTLSoftwareLibrary.h"

@implementation MTLSoftwareFunction

@synthesize device = _device;
@synthesize name = _name;
@synthesize label = _label;
@synthesize functionType = _type;
@synthesize vertexFunction = _vertexFunction;
@synthesize fragmentFunction = _fragmentFunction;

- (instancetype)initWithDevice:(id<MTLDevice>)device name:(NSString *)name type:(MTLFunctionType)type vertexFunc:(mtl_vertex_function)vf fragmentFunc:(mtl_fragment_function)ff
{
	self = [super init];
	if (self) {
		_device = device;
		_name = [name copy];
		_type = type;
		_vertexFunction = vf;
		_fragmentFunction = ff;
	}
	return self;
}

@end

#import <dlfcn.h>

@implementation MTLSoftwareLibrary

@synthesize device = _device;
@synthesize label = _label;
@synthesize cLibrary = _cLibrary;

- (instancetype)initWithDevice:(id<MTLDevice>)device
{
	self = [super init];
	if (self) {
		_device = device;
		_cLibrary = mtl_device_new_library(NULL);
		_functions = [NSMutableDictionary new];
		_dlHandle = NULL;
	}
	return self;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device dylibPath:(NSString *)path
{
	self = [self initWithDevice:device];
	if (self && path) {
		_dlHandle = dlopen([path UTF8String], RTLD_NOW | RTLD_LOCAL);
	}
	return self;
}

- (void)dealloc
{
	if (_dlHandle) {
		dlclose(_dlHandle);
	}
	if (_cLibrary) {
		mtl_library_release(_cLibrary);
	}
}

- (void)registerVertexFunction:(mtl_vertex_function)func withName:(NSString *)name
{
	mtl_library_add_function(_cLibrary, [name UTF8String], func);
	MTLSoftwareFunction *fn = [[MTLSoftwareFunction alloc] initWithDevice:_device name:name type:MTLFunctionTypeVertex vertexFunc:func fragmentFunc:NULL];
	[_functions setObject:fn forKey:name];
}

- (void)registerFragmentFunction:(mtl_fragment_function)func withName:(NSString *)name
{
	mtl_library_add_fragment_function(_cLibrary, [name UTF8String], func);
	MTLSoftwareFunction *fn = [[MTLSoftwareFunction alloc] initWithDevice:_device name:name type:MTLFunctionTypeFragment vertexFunc:NULL fragmentFunc:func];
	[_functions setObject:fn forKey:name];
}

- (NSArray *)functionNames
{
	return [_functions allKeys];
}

- (id<MTLFunction>)newFunctionWithName:(NSString *)functionName
{
	id<MTLFunction> fn = [_functions objectForKey:functionName];
	if (fn != nil) {
		return fn;
	}

	if (_dlHandle != NULL && functionName != nil) {
		void *sym = dlsym(_dlHandle, [functionName UTF8String]);
		if (sym != NULL) {
			if ([functionName rangeOfString:@"fragment"].location != NSNotFound ||
			    [functionName rangeOfString:@"frag"].location != NSNotFound ||
			    [functionName hasPrefix:@"fs_"]) {
				[self registerFragmentFunction:(mtl_fragment_function)sym withName:functionName];
			} else {
				[self registerVertexFunction:(mtl_vertex_function)sym withName:functionName];
			}
			return [_functions objectForKey:functionName];
		}
	}

	return nil;
}

@end

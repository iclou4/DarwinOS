/*
 * _MTLDevice.m - ravynOS Metal: base device implementation
 */

#import <Metal/_MTLDevice.h>
#import "MTLSoftwareLibrary.h"
#import "MTLSoftwareRenderPipeline.h"
#import "MTLSoftwareDepthStencil.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

@implementation _MTLDevice

@synthesize name = _name;
@synthesize registryID = _registryID;
@synthesize maxThreadsPerThreadgroup = _maxThreadsPerThreadgroup;
@synthesize lowPower = _lowPower;
@synthesize headless = _headless;
@synthesize removable = _removable;
@synthesize hasUnifiedMemory = _hasUnifiedMemory;
@synthesize recommendedMaxWorkingSetSize = _recommendedMaxWorkingSetSize;
@synthesize location = _location;
@synthesize locationNumber = _locationNumber;
@synthesize maxBufferLength = _maxBufferLength;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_name = @"Apple GPU (Default)";
		_registryID = 0x10001;
		_maxThreadsPerThreadgroup = MTLSizeMake(1024, 1024, 64);
		_hasUnifiedMemory = YES;
		_lowPower = NO;
		_headless = NO;
		_removable = NO;
		_recommendedMaxWorkingSetSize = 1024ULL * 1024ULL * 1024ULL; // 1 GB
		_location = MTLDeviceLocationBuiltIn;
		_locationNumber = 0;
		_maxBufferLength = 512ULL * 1024ULL * 1024ULL; // 512 MB
	}
	return self;
}

- (BOOL)supportsFeatureSet:(MTLFeatureSet)featureSet
{
	(void)featureSet;
	return YES;
}

- (BOOL)supportsFamily:(MTLGPUFamily)gpuFamily
{
	(void)gpuFamily;
	return YES;
}

- (id<MTLCommandQueue>)newCommandQueue
{
	return [self newCommandQueueWithMaxCommandBufferCount:64];
}

- (id<MTLCommandQueue>)newCommandQueueWithMaxCommandBufferCount:(NSUInteger)maxCommandBufferCount
{
	(void)maxCommandBufferCount;
	return nil;
}

- (id<MTLBuffer>)newBufferWithLength:(NSUInteger)length options:(MTLResourceOptions)options
{
	(void)length; (void)options;
	return nil;
}

- (id<MTLBuffer>)newBufferWithBytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options
{
	(void)pointer; (void)length; (void)options;
	return nil;
}

- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor
{
	(void)descriptor;
	return nil;
}

- (id<MTLSamplerState>)newSamplerStateWithDescriptor:(MTLSamplerDescriptor *)descriptor
{
	(void)descriptor;
	return nil;
}

- (id<MTLLibrary>)newDefaultLibrary
{
	NSBundle *bundle = [NSBundle mainBundle];
	NSString *path = [bundle pathForResource:@"default" ofType:@"metallib"];
	if (!path) {
		path = @"default.metallib";
	}
	return [self newLibraryWithFile:path error:nil];
}

- (id<MTLLibrary>)newLibraryWithFile:(NSString *)filepath error:(__autoreleasing NSError **)error
{
	if (![[NSFileManager defaultManager] fileExistsAtPath:filepath]) {
		if (error) {
			*error = [NSError errorWithDomain:@"Metal" code:1 userInfo:@{NSLocalizedDescriptionKey: [NSString stringWithFormat:@"File not found: %@", filepath]}];
		}
		return nil;
	}
	return [[MTLSoftwareLibrary alloc] initWithDevice:self dylibPath:filepath];
}

- (id<MTLLibrary>)newLibraryWithSource:(NSString *)source options:(MTLCompileOptions *)options error:(__autoreleasing NSError **)error
{
	char tmpsrc[] = "/tmp/mtl_shader_XXXXXX.metal";
	char tmpout[] = "/tmp/mtl_shader_XXXXXX.metallib";
	int sfd = mkstemps(tmpsrc, 6);
	int ofd = mkstemps(tmpout, 9);
	if (sfd < 0 || ofd < 0) {
		if (sfd >= 0) { close(sfd); unlink(tmpsrc); }
		if (ofd >= 0) { close(ofd); unlink(tmpout); }
		if (error) *error = [NSError errorWithDomain:@"Metal" code:1 userInfo:@{NSLocalizedDescriptionKey: @"Failed to create temp files"}];
		return nil;
	}
	close(ofd);

	const char *srcstr = [source UTF8String];
	write(sfd, srcstr, strlen(srcstr));
	close(sfd);

	const char *metal_bin = "/Library/Developer/Toolchains/Default.xctoolchain/usr/bin/metal";
	if (access(metal_bin, X_OK) != 0) {
		metal_bin = "/usr/local/bin/metal";
		if (access(metal_bin, X_OK) != 0) {
			metal_bin = "metal";
		}
	}

	char cmd[2048];
	snprintf(cmd, sizeof(cmd), "%s \"%s\" -o \"%s\" 2>&1", metal_bin, tmpsrc, tmpout);

	FILE *fp = popen(cmd, "r");
	char logbuf[4096];
	size_t n = 0;
	if (fp) {
		n = fread(logbuf, 1, sizeof(logbuf) - 1, fp);
		logbuf[n] = '\0';
		int status = pclose(fp);
		unlink(tmpsrc);
		if (status != 0) {
			if (error) {
				NSString *diag = [NSString stringWithUTF8String:logbuf];
				*error = [NSError errorWithDomain:@"Metal" code:status userInfo:@{NSLocalizedDescriptionKey: diag ?: @"MSL compilation failed"}];
			}
			unlink(tmpout);
			return nil;
		}
	} else {
		unlink(tmpsrc);
		if (error) *error = [NSError errorWithDomain:@"Metal" code:2 userInfo:@{NSLocalizedDescriptionKey: @"Failed to invoke metal compiler"}];
		return nil;
	}

	MTLSoftwareLibrary *lib = [[MTLSoftwareLibrary alloc] initWithDevice:self dylibPath:[NSString stringWithUTF8String:tmpout]];
	unlink(tmpout);
	return lib;
}

- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)descriptor error:(__autoreleasing NSError **)error
{
	(void)error;
	return [[MTLSoftwareRenderPipelineState alloc] initWithDevice:self descriptor:descriptor];
}

- (id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)descriptor options:(MTLPipelineOption)options reflection:(MTLComputePipelineReflection **)reflection error:(__autoreleasing NSError **)error
{
	(void)descriptor; (void)options; (void)reflection; (void)error;
	return nil;
}

- (id<MTLComputePipelineState>)newComputePipelineStateWithFunction:(id<MTLFunction>)computeFunction error:(__autoreleasing NSError **)error
{
	(void)computeFunction; (void)error;
	return nil;
}

- (id<MTLDepthStencilState>)newDepthStencilStateWithDescriptor:(MTLDepthStencilDescriptor *)descriptor
{
	return [[MTLSoftwareDepthStencilState alloc] initWithDevice:self descriptor:descriptor];
}

@end

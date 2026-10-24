/*
 * MTLSoftwareTexture.m - ravynOS Metal: texture implementation
 */

#import "MTLSoftwareTexture.h"

@implementation MTLSoftwareTexture

@synthesize label = _label;
@synthesize device = _device;
@synthesize cTexture = _cTexture;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLTextureDescriptor *)descriptor
{
	self = [super init];
	if (self) {
		_device = device;
		_descriptor = [descriptor copy];

		mtl_texture_descriptor cDesc;
		memset(&cDesc, 0, sizeof(cDesc));
		cDesc.pixel_format = descriptor.pixelFormat;
		cDesc.width = (mtl_size_t)descriptor.width;
		cDesc.height = (mtl_size_t)descriptor.height;
		cDesc.depth = (mtl_size_t)descriptor.depth > 0 ? (mtl_size_t)descriptor.depth : 1;
		cDesc.usage = (unsigned int)descriptor.usage;
		cDesc.storage_mode = (unsigned int)descriptor.storageMode;

		_cTexture = mtl_device_new_texture(NULL, &cDesc);
	}
	return self;
}

- (void)dealloc
{
	if (_cTexture) {
		mtl_texture_release(_cTexture);
	}
}

- (void *)contents
{
	return mtl_texture_contents(_cTexture);
}

- (MTLTextureType)textureType
{
	return _descriptor.textureType;
}

- (MTLPixelFormat)pixelFormat
{
	return _descriptor.pixelFormat;
}

- (NSUInteger)width
{
	return _descriptor.width;
}

- (NSUInteger)height
{
	return _descriptor.height;
}

- (NSUInteger)depth
{
	return _descriptor.depth > 0 ? _descriptor.depth : 1;
}

- (NSUInteger)mipmapLevelCount
{
	return _descriptor.mipmapLevelCount > 0 ? _descriptor.mipmapLevelCount : 1;
}

- (NSUInteger)sampleCount
{
	return _descriptor.sampleCount > 0 ? _descriptor.sampleCount : 1;
}

- (NSUInteger)arrayLength
{
	return _descriptor.arrayLength > 0 ? _descriptor.arrayLength : 1;
}

- (MTLTextureUsage)usage
{
	return _descriptor.usage;
}

- (id<MTLResource>)rootResource
{
	return self;
}

- (id<MTLTexture>)parentTexture
{
	return nil;
}

- (NSUInteger)parentRelativeLevel
{
	return 0;
}

- (NSUInteger)parentRelativeSlice
{
	return 0;
}

- (id<MTLBuffer>)buffer
{
	return nil;
}

- (NSUInteger)bufferOffset
{
	return 0;
}

- (NSUInteger)bufferBytesPerRow
{
	return 0;
}

- (MTLCPUCacheMode)cpuCacheMode
{
	return _descriptor.cpuCacheMode;
}

- (MTLStorageMode)storageMode
{
	return _descriptor.storageMode;
}

- (MTLHazardTrackingMode)hazardTrackingMode
{
	return MTLHazardTrackingModeDefault;
}

- (MTLResourceOptions)resourceOptions
{
	return _descriptor.resourceOptions;
}

- (MTLPurgeableState)setPurgeableState:(MTLPurgeableState)state
{
	return state;
}

- (BOOL)isAliasable
{
	return NO;
}

- (void)makeAliasable
{
}

- (void)getBytes:(void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow fromRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level
{
	(void)level;
	mtl_texture_get_bytes(_cTexture, region, pixelBytes, (mtl_size_t)bytesPerRow);
}

- (void)getBytes:(void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage fromRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level slice:(NSUInteger)slice
{
	(void)bytesPerImage; (void)slice;
	[self getBytes:pixelBytes bytesPerRow:bytesPerRow fromRegion:region mipmapLevel:level];
}

- (void)replaceRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level withBytes:(const void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow
{
	(void)level;
	mtl_texture_replace_region(_cTexture, region, pixelBytes, (mtl_size_t)bytesPerRow);
}

- (void)replaceRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level slice:(NSUInteger)slice withBytes:(const void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage
{
	(void)slice; (void)bytesPerImage;
	[self replaceRegion:region mipmapLevel:level withBytes:pixelBytes bytesPerRow:bytesPerRow];
}

@end

/*
 * MTLSoftwareBuffer.m - ravynOS Metal: buffer implementation
 */

#import "MTLSoftwareBuffer.h"

@implementation MTLSoftwareBuffer

@synthesize label = _label;
@synthesize device = _device;
@synthesize length = _length;
@synthesize resourceOptions = _options;
@synthesize cBuffer = _cBuffer;

- (instancetype)initWithDevice:(id<MTLDevice>)device length:(NSUInteger)length options:(MTLResourceOptions)options
{
	self = [super init];
	if (self) {
		_device = device;
		_length = length;
		_options = options;
		_contents = calloc(1, length > 0 ? length : 1);
		_ownsMemory = YES;

		_cBuffer = (mtl_buffer_t *)calloc(1, sizeof(mtl_buffer_t));
		_cBuffer->length = (mtl_size_t)length;
		_cBuffer->bytes = (unsigned char *)_contents;
	}
	return self;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device bytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options
{
	self = [self initWithDevice:device length:length options:options];
	if (self && pointer && length > 0) {
		memcpy(_contents, pointer, length);
	}
	return self;
}

- (instancetype)initWithDevice:(id<MTLDevice>)device bytesNoCopy:(void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options deallocator:(void (^)(void *, NSUInteger))deallocator
{
	self = [super init];
	if (self) {
		_device = device;
		_length = length;
		_options = options;
		_contents = pointer;
		_ownsMemory = NO;
		_deallocator = [deallocator copy];

		_cBuffer = (mtl_buffer_t *)calloc(1, sizeof(mtl_buffer_t));
		_cBuffer->length = (mtl_size_t)length;
		_cBuffer->bytes = (unsigned char *)_contents;
	}
	return self;
}

- (void)dealloc
{
	if (_deallocator) {
		_deallocator(_contents, _length);
	} else if (_ownsMemory && _contents) {
		free(_contents);
	}
	if (_cBuffer) {
		free(_cBuffer);
	}
}

- (void *)contents
{
	return _contents;
}

- (void)didModifyRange:(NSRange)range
{
	(void)range;
}

- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor offset:(NSUInteger)offset bytesPerRow:(NSUInteger)bytesPerRow
{
	(void)descriptor; (void)offset; (void)bytesPerRow;
	return nil;
}

- (void)addDebugMarker:(NSString *)marker range:(NSRange)range
{
	(void)marker; (void)range;
}

- (void)removeAllDebugMarkers
{
}

- (MTLCPUCacheMode)cpuCacheMode
{
	return MTLCPUCacheModeDefaultCache;
}

- (MTLStorageMode)storageMode
{
	return MTLStorageModeShared;
}

- (MTLHazardTrackingMode)hazardTrackingMode
{
	return MTLHazardTrackingModeDefault;
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

@end

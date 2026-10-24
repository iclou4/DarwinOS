/*
 * MTLSoftwareCommandQueue.m - ravynOS Metal: command queue implementation
 */

#import "MTLSoftwareCommandQueue.h"
#import "MTLSoftwareCommandBuffer.h"

@implementation MTLSoftwareCommandQueue

@synthesize label = _label;
@synthesize device = _device;
@synthesize cQueue = _cQueue;

- (instancetype)initWithDevice:(id<MTLDevice>)device
{
	self = [super init];
	if (self) {
		_device = device;
		_cQueue = mtl_device_new_command_queue(NULL);
	}
	return self;
}

- (void)dealloc
{
	if (_cQueue) {
		mtl_command_queue_release(_cQueue);
	}
}

- (id<MTLCommandBuffer>)commandBuffer
{
	mtl_command_buffer_t *cBuf = mtl_command_queue_command_buffer(_cQueue);
	return [[MTLSoftwareCommandBuffer alloc] initWithQueue:self cCommandBuffer:cBuf];
}

- (id<MTLCommandBuffer>)commandBufferWithUnretainedReferences
{
	return [self commandBuffer];
}

- (void)insertDebugCaptureBoundary
{
}

@end

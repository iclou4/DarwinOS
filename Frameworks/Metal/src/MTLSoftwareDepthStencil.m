/*
 * MTLSoftwareDepthStencil.m - ravynOS Metal: depth stencil state implementation
 */

#import "MTLSoftwareDepthStencil.h"
#import "mtl_internal.h"

@implementation MTLSoftwareDepthStencilState

@synthesize label = _label;
@synthesize device = _device;
@synthesize compareFunction = _compareFunction;
@synthesize depthWriteEnabled = _depthWriteEnabled;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLDepthStencilDescriptor *)descriptor
{
	self = [super init];
	if (self) {
		_device = device;
		_label = [descriptor.label copy];
		_compareFunction = descriptor.depthCompareFunction;
		_depthWriteEnabled = descriptor.isDepthWriteEnabled;
	}
	return self;
}

@end

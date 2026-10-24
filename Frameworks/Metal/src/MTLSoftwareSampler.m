/*
 * MTLSoftwareSampler.m - ravynOS Metal: sampler state implementation
 */

#import "MTLSoftwareSampler.h"

@implementation MTLSoftwareSamplerState

@synthesize label = _label;
@synthesize device = _device;
@synthesize cSampler = _cSampler;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLSamplerDescriptor *)descriptor
{
	self = [super init];
	if (self) {
		_device = device;
		_label = [descriptor.label copy];

		mtl_sampler_descriptor cDesc;
		memset(&cDesc, 0, sizeof(cDesc));
		cDesc.min_filter = descriptor.minFilter;
		cDesc.mag_filter = descriptor.magFilter;
		cDesc.address_s = descriptor.sAddressMode;
		cDesc.address_t = descriptor.tAddressMode;

		_cSampler = mtl_device_new_sampler(NULL, &cDesc);
	}
	return self;
}

- (void)dealloc
{
	if (_cSampler) {
		mtl_sampler_release(_cSampler);
	}
}

@end

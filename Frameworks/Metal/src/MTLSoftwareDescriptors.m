/*
 * MTLSoftwareDescriptors.m - ravynOS Metal: descriptor class implementations
 */

#import <Metal/Metal.h>
#include <math.h>

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

@implementation MTLTextureDescriptor

@synthesize textureType = _textureType;
@synthesize pixelFormat = _pixelFormat;
@synthesize width = _width;
@synthesize height = _height;
@synthesize depth = _depth;
@synthesize mipmapLevelCount = _mipmapLevelCount;
@synthesize sampleCount = _sampleCount;
@synthesize arrayLength = _arrayLength;
@synthesize resourceOptions = _resourceOptions;
@synthesize cpuCacheMode = _cpuCacheMode;
@synthesize storageMode = _storageMode;
@synthesize usage = _usage;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_textureType = MTLTextureType2D;
		_pixelFormat = MTLPixelFormatRGBA8Unorm;
		_width = 1;
		_height = 1;
		_depth = 1;
		_mipmapLevelCount = 1;
		_sampleCount = 1;
		_arrayLength = 1;
		_storageMode = MTLStorageModeShared;
		_usage = MTLTextureUsageShaderRead;
	}
	return self;
}

+ (MTLTextureDescriptor *)texture2DDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat width:(NSUInteger)width height:(NSUInteger)height mipmapped:(BOOL)mipmapped
{
	MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
	desc.textureType = MTLTextureType2D;
	desc.pixelFormat = pixelFormat;
	desc.width = width;
	desc.height = height;
	desc.mipmapLevelCount = mipmapped ? (NSUInteger)(floor(log2(MAX(width, height))) + 1) : 1;
	return desc;
}

+ (MTLTextureDescriptor *)textureCubeDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat size:(NSUInteger)size mipmapped:(BOOL)mipmapped
{
	MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
	desc.textureType = MTLTextureTypeCube;
	desc.pixelFormat = pixelFormat;
	desc.width = size;
	desc.height = size;
	desc.mipmapLevelCount = mipmapped ? (NSUInteger)(floor(log2(size)) + 1) : 1;
	return desc;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLTextureDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.textureType = self.textureType;
	copy.pixelFormat = self.pixelFormat;
	copy.width = self.width;
	copy.height = self.height;
	copy.depth = self.depth;
	copy.mipmapLevelCount = self.mipmapLevelCount;
	copy.sampleCount = self.sampleCount;
	copy.arrayLength = self.arrayLength;
	copy.resourceOptions = self.resourceOptions;
	copy.cpuCacheMode = self.cpuCacheMode;
	copy.storageMode = self.storageMode;
	copy.usage = self.usage;
	return copy;
}

@end

@implementation MTLSamplerDescriptor

@synthesize minFilter = _minFilter;
@synthesize magFilter = _magFilter;
@synthesize mipFilter = _mipFilter;
@synthesize maxAnisotropy = _maxAnisotropy;
@synthesize sAddressMode = _sAddressMode;
@synthesize tAddressMode = _tAddressMode;
@synthesize rAddressMode = _rAddressMode;
@synthesize borderColor = _borderColor;
@synthesize normalizedCoordinates = _normalizedCoordinates;
@synthesize lodMinClamp = _lodMinClamp;
@synthesize lodMaxClamp = _lodMaxClamp;
@synthesize compareFunction = _compareFunction;
@synthesize label = _label;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_minFilter = MTLSamplerMinMagFilterNearest;
		_magFilter = MTLSamplerMinMagFilterNearest;
		_mipFilter = MTLSamplerMipFilterNotMipmapped;
		_maxAnisotropy = 1;
		_sAddressMode = MTLSamplerAddressModeClampToEdge;
		_tAddressMode = MTLSamplerAddressModeClampToEdge;
		_rAddressMode = MTLSamplerAddressModeClampToEdge;
		_normalizedCoordinates = YES;
		_lodMaxClamp = FLT_MAX;
		_compareFunction = MTLCompareFunctionNever;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLSamplerDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.minFilter = self.minFilter;
	copy.magFilter = self.magFilter;
	copy.mipFilter = self.mipFilter;
	copy.maxAnisotropy = self.maxAnisotropy;
	copy.sAddressMode = self.sAddressMode;
	copy.tAddressMode = self.tAddressMode;
	copy.rAddressMode = self.rAddressMode;
	copy.borderColor = self.borderColor;
	copy.normalizedCoordinates = self.normalizedCoordinates;
	copy.lodMinClamp = self.lodMinClamp;
	copy.lodMaxClamp = self.lodMaxClamp;
	copy.compareFunction = self.compareFunction;
	copy.label = self.label;
	return copy;
}

@end

@implementation MTLRenderPipelineColorAttachmentDescriptor

@synthesize pixelFormat = _pixelFormat;
@synthesize blendingEnabled = _blendingEnabled;
@synthesize sourceRGBBlendFactor = _sourceRGBBlendFactor;
@synthesize destinationRGBBlendFactor = _destinationRGBBlendFactor;
@synthesize rgbBlendOperation = _rgbBlendOperation;
@synthesize sourceAlphaBlendFactor = _sourceAlphaBlendFactor;
@synthesize destinationAlphaBlendFactor = _destinationAlphaBlendFactor;
@synthesize alphaBlendOperation = _alphaBlendOperation;
@synthesize writeMask = _writeMask;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_pixelFormat = MTLPixelFormatInvalid;
		_blendingEnabled = NO;
		_sourceRGBBlendFactor = MTLBlendFactorOne;
		_destinationRGBBlendFactor = MTLBlendFactorZero;
		_rgbBlendOperation = MTLBlendOperationAdd;
		_sourceAlphaBlendFactor = MTLBlendFactorOne;
		_destinationAlphaBlendFactor = MTLBlendFactorZero;
		_alphaBlendOperation = MTLBlendOperationAdd;
		_writeMask = MTLColorWriteMaskAll;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLRenderPipelineColorAttachmentDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.pixelFormat = self.pixelFormat;
	copy.blendingEnabled = self.blendingEnabled;
	copy.sourceRGBBlendFactor = self.sourceRGBBlendFactor;
	copy.destinationRGBBlendFactor = self.destinationRGBBlendFactor;
	copy.rgbBlendOperation = self.rgbBlendOperation;
	copy.sourceAlphaBlendFactor = self.sourceAlphaBlendFactor;
	copy.destinationAlphaBlendFactor = self.destinationAlphaBlendFactor;
	copy.alphaBlendOperation = self.alphaBlendOperation;
	copy.writeMask = self.writeMask;
	return copy;
}

@end

@implementation MTLRenderPipelineColorAttachmentDescriptorArray {
	MTLRenderPipelineColorAttachmentDescriptor *_descriptors[8];
}

- (MTLRenderPipelineColorAttachmentDescriptor *)objectAtIndexedSubscript:(NSUInteger)attachmentIndex
{
	if (attachmentIndex >= 8) return nil;
	if (!_descriptors[attachmentIndex]) {
		_descriptors[attachmentIndex] = [[MTLRenderPipelineColorAttachmentDescriptor alloc] init];
	}
	return _descriptors[attachmentIndex];
}

- (void)setObject:(MTLRenderPipelineColorAttachmentDescriptor *)attachment atIndexedSubscript:(NSUInteger)attachmentIndex
{
	if (attachmentIndex < 8) {
		_descriptors[attachmentIndex] = [attachment copy];
	}
}

@end

@implementation MTLRenderPipelineDescriptor

@synthesize label = _label;
@synthesize vertexFunction = _vertexFunction;
@synthesize fragmentFunction = _fragmentFunction;
@synthesize vertexDescriptor = _vertexDescriptor;
@synthesize sampleCount = _sampleCount;
@synthesize colorAttachments = _colorAttachments;
@synthesize depthAttachmentPixelFormat = _depthAttachmentPixelFormat;
@synthesize stencilAttachmentPixelFormat = _stencilAttachmentPixelFormat;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_colorAttachments = [[MTLRenderPipelineColorAttachmentDescriptorArray alloc] init];
		_sampleCount = 1;
		_depthAttachmentPixelFormat = MTLPixelFormatInvalid;
		_stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
	}
	return self;
}

- (void)reset
{
	_colorAttachments = [[MTLRenderPipelineColorAttachmentDescriptorArray alloc] init];
	_vertexFunction = nil;
	_fragmentFunction = nil;
	_vertexDescriptor = nil;
	_sampleCount = 1;
	_depthAttachmentPixelFormat = MTLPixelFormatInvalid;
	_stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLRenderPipelineDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.label = self.label;
	copy.vertexFunction = self.vertexFunction;
	copy.fragmentFunction = self.fragmentFunction;
	copy.vertexDescriptor = self.vertexDescriptor;
	copy.sampleCount = self.sampleCount;
	for (NSUInteger i = 0; i < 8; i++) {
		copy.colorAttachments[i] = [self.colorAttachments[i] copy];
	}
	copy.depthAttachmentPixelFormat = self.depthAttachmentPixelFormat;
	copy.stencilAttachmentPixelFormat = self.stencilAttachmentPixelFormat;
	return copy;
}

@end

@implementation MTLRenderPassAttachmentDescriptor

@synthesize texture = _texture;
@synthesize level = _level;
@synthesize slice = _slice;
@synthesize depthPlane = _depthPlane;
@synthesize resolveTexture = _resolveTexture;
@synthesize resolveLevel = _resolveLevel;
@synthesize resolveSlice = _resolveSlice;
@synthesize resolveDepthPlane = _resolveDepthPlane;
@synthesize loadAction = _loadAction;
@synthesize storeAction = _storeAction;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_loadAction = MTLLoadActionDontCare;
		_storeAction = MTLStoreActionDontCare;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLRenderPassAttachmentDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.texture = self.texture;
	copy.level = self.level;
	copy.slice = self.slice;
	copy.depthPlane = self.depthPlane;
	copy.resolveTexture = self.resolveTexture;
	copy.resolveLevel = self.resolveLevel;
	copy.resolveSlice = self.resolveSlice;
	copy.resolveDepthPlane = self.resolveDepthPlane;
	copy.loadAction = self.loadAction;
	copy.storeAction = self.storeAction;
	return copy;
}

@end

@implementation MTLRenderPassColorAttachmentDescriptor
@synthesize clearColor = _clearColor;
@end

@implementation MTLRenderPassDepthAttachmentDescriptor
@synthesize clearDepth = _clearDepth;
@synthesize depthResolveFilter = _depthResolveFilter;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_clearDepth = 1.0;
	}
	return self;
}
@end

@implementation MTLRenderPassStencilAttachmentDescriptor
@synthesize clearStencil = _clearStencil;
@end

@implementation MTLRenderPassColorAttachmentDescriptorArray {
	MTLRenderPassColorAttachmentDescriptor *_descriptors[8];
}

- (MTLRenderPassColorAttachmentDescriptor *)objectAtIndexedSubscript:(NSUInteger)attachmentIndex
{
	if (attachmentIndex >= 8) return nil;
	if (!_descriptors[attachmentIndex]) {
		_descriptors[attachmentIndex] = [[MTLRenderPassColorAttachmentDescriptor alloc] init];
	}
	return _descriptors[attachmentIndex];
}

- (void)setObject:(MTLRenderPassColorAttachmentDescriptor *)attachment atIndexedSubscript:(NSUInteger)attachmentIndex
{
	if (attachmentIndex < 8) {
		_descriptors[attachmentIndex] = [attachment copy];
	}
}

@end

@implementation MTLRenderPassDescriptor

@synthesize colorAttachments = _colorAttachments;
@synthesize depthAttachment = _depthAttachment;
@synthesize stencilAttachment = _stencilAttachment;
@synthesize visibilityResultBuffer = _visibilityResultBuffer;
@synthesize renderTargetArrayLength = _renderTargetArrayLength;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_colorAttachments = [[MTLRenderPassColorAttachmentDescriptorArray alloc] init];
		_depthAttachment = [[MTLRenderPassDepthAttachmentDescriptor alloc] init];
		_stencilAttachment = [[MTLRenderPassStencilAttachmentDescriptor alloc] init];
	}
	return self;
}

+ (MTLRenderPassDescriptor *)renderPassDescriptor
{
	return [[MTLRenderPassDescriptor alloc] init];
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLRenderPassDescriptor *copy = [[[self class] allocWithZone:zone] init];
	for (NSUInteger i = 0; i < 8; i++) {
		copy.colorAttachments[i] = [self.colorAttachments[i] copy];
	}
	copy.depthAttachment = [self.depthAttachment copy];
	copy.stencilAttachment = [self.stencilAttachment copy];
	copy.visibilityResultBuffer = self.visibilityResultBuffer;
	copy.renderTargetArrayLength = self.renderTargetArrayLength;
	return copy;
}

@end

@implementation MTLDepthStencilDescriptor
@synthesize depthCompareFunction = _depthCompareFunction;
@synthesize depthWriteEnabled = _depthWriteEnabled;
@synthesize frontFaceStencil = _frontFaceStencil;
@synthesize backFaceStencil = _backFaceStencil;
@synthesize label = _label;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_depthCompareFunction = MTLCompareFunctionAlways;
		_depthWriteEnabled = NO;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLDepthStencilDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.depthCompareFunction = self.depthCompareFunction;
	copy.depthWriteEnabled = self.depthWriteEnabled;
	copy.frontFaceStencil = self.frontFaceStencil;
	copy.backFaceStencil = self.backFaceStencil;
	copy.label = self.label;
	return copy;
}

@end

@implementation MTLStencilDescriptor
@synthesize stencilFailureOperation = _stencilFailureOperation;
@synthesize depthFailureOperation = _depthFailureOperation;
@synthesize depthNormalHandling = _depthNormalHandling;
@synthesize stencilCompareFunction = _stencilCompareFunction;
@synthesize readMask = _readMask;
@synthesize writeMask = _writeMask;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_stencilCompareFunction = MTLCompareFunctionAlways;
		_readMask = 0xFFFFFFFF;
		_writeMask = 0xFFFFFFFF;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLStencilDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.stencilFailureOperation = self.stencilFailureOperation;
	copy.depthFailureOperation = self.depthFailureOperation;
	copy.depthNormalHandling = self.depthNormalHandling;
	copy.stencilCompareFunction = self.stencilCompareFunction;
	copy.readMask = self.readMask;
	copy.writeMask = self.writeMask;
	return copy;
}

@end

@implementation MTLCompileOptions
@synthesize preprocessorMacros = _preprocessorMacros;
@synthesize fastMathEnabled = _fastMathEnabled;
@synthesize languageVersion = _languageVersion;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_fastMathEnabled = YES;
	}
	return self;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLCompileOptions *copy = [[[self class] allocWithZone:zone] init];
	copy.preprocessorMacros = self.preprocessorMacros;
	copy.fastMathEnabled = self.fastMathEnabled;
	copy.languageVersion = self.languageVersion;
	return copy;
}

@end

@implementation MTLComputePipelineDescriptor
@synthesize label = _label;
@synthesize computeFunction = _computeFunction;
@synthesize threadGroupSizeIsMultipleOfThreadExecutionWidth = _threadGroupSizeIsMultipleOfThreadExecutionWidth;

- (void)reset
{
	_label = nil;
	_computeFunction = nil;
}

- (id)copyWithZone:(NSZone *)zone
{
	MTLComputePipelineDescriptor *copy = [[[self class] allocWithZone:zone] init];
	copy.label = self.label;
	copy.computeFunction = self.computeFunction;
	copy.threadGroupSizeIsMultipleOfThreadExecutionWidth = self.threadGroupSizeIsMultipleOfThreadExecutionWidth;
	return copy;
}

@end

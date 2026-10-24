/*
 * CAMetalLayer.m - ravynOS QuartzCore: Metal layer and drawable
 */

#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/CATransaction.h>
#import <Metal/Metal.h>

@interface _CAMetalDrawable : NSObject <CAMetalDrawable> {
@public
	CAMetalLayer *_layer;
	id<MTLTexture> _texture;
	NSUInteger _drawableID;
}
@property (readonly) CAMetalLayer *layer;
@property (readonly) id<MTLTexture> texture;
@property (readonly) NSUInteger drawableID;
@end

@implementation _CAMetalDrawable
@synthesize layer = _layer;
@synthesize texture = _texture;
@synthesize drawableID = _drawableID;

- (instancetype)initWithLayer:(CAMetalLayer *)layer texture:(id<MTLTexture>)texture drawableID:(NSUInteger)drawableID
{
	self = [super init];
	if (self) {
		_layer = layer;
		_texture = texture;
		_drawableID = drawableID;
	}
	return self;
}

- (void)present
{
	// 1. Update QuartzCore layer contents with the rendered Metal texture
	[_layer setContents:_texture];
	[_layer setNeedsDisplay];

	// 2. Direct WindowServer / IOKit scanout presentation
	void *pixels = NULL;
	if ([_texture respondsToSelector:@selector(contents)]) {
		pixels = [((id)_texture) contents];
	}
	if (pixels != NULL) {
		NSUInteger width = [_texture width];
		NSUInteger height = [_texture height];
		NSUInteger stride = width * 4;
#if defined(__APPLE__) || defined(__ravynOS__) || defined(__Darwin__)
		register long rax __asm__("rax") = (0x2000000 | 535); // SYS_FB_PRESENT
		register long rdi __asm__("rdi") = (long)pixels;
		register long rsi __asm__("rsi") = (long)width;
		register long rdx __asm__("rdx") = (long)height;
		register long r10 __asm__("r10") = (long)stride;
		__asm__ volatile("syscall"
		    : "+r"(rax)
		    : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
		    : "rcx", "r11", "memory");
#endif
	}
}

- (void)presentAtTime:(CFTimeInterval)presentationTime
{
	(void)presentationTime;
	[self present];
}

- (void)presentAfterMinimumDuration:(CFTimeInterval)duration
{
	(void)duration;
	[self present];
}

@end

@implementation CAMetalLayer

@synthesize device = _device;
@synthesize pixelFormat = _pixelFormat;
@synthesize framebufferOnly = _framebufferOnly;
@synthesize drawableSize = _drawableSize;
@synthesize presentsWithTransaction = _presentsWithTransaction;

- (instancetype)init
{
	self = [super init];
	if (self) {
		_pixelFormat = MTLPixelFormatBGRA8Unorm;
		_framebufferOnly = YES;
		_drawableSize = CGSizeMake(1280, 800);
		_drawableID = 0;
	}
	return self;
}

+ (id)layer
{
	return [[self alloc] init];
}

- (id<CAMetalDrawable>)nextDrawable
{
	if (_device == nil) {
		_device = MTLCreateSystemDefaultDevice();
	}
	if (_device == nil) {
		return nil;
	}

	NSUInteger w = (NSUInteger)(_drawableSize.width > 0 ? _drawableSize.width : 1280);
	NSUInteger h = (NSUInteger)(_drawableSize.height > 0 ? _drawableSize.height : 800);

	MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_pixelFormat width:w height:h mipmapped:NO];
	desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
	desc.storageMode = MTLStorageModeShared;

	id<MTLTexture> tex = [_device newTextureWithDescriptor:desc];
	_drawableID++;
	return [[_CAMetalDrawable alloc] initWithLayer:self texture:tex drawableID:_drawableID];
}

@end

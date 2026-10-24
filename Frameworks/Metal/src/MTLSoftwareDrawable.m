/*
 * MTLSoftwareDrawable.m - ravynOS Metal: presentation drawable
 */

#import "MTLSoftwareDrawable.h"
#import "MTLSoftwareTexture.h"
#import "mtl_internal.h"
#import <unistd.h>

#if defined(__APPLE__) || defined(__ravynOS__) || defined(__Darwin__)
#define SYS_FB_PRESENT (0x2000000 | 535)

static long
raw_fb_present(void *buf, unsigned w, unsigned h, unsigned stride)
{
	register long rax __asm__("rax") = SYS_FB_PRESENT;
	register long rdi __asm__("rdi") = (long)buf;
	register long rsi __asm__("rsi") = (long)w;
	register long rdx __asm__("rdx") = (long)h;
	register long r10 __asm__("r10") = (long)stride;

	__asm__ volatile("syscall"
	    : "+r"(rax)
	    : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
	    : "rcx", "r11", "memory");
	return rax;
}
#endif

@implementation MTLSoftwareDrawable

@synthesize texture = _texture;
@synthesize drawableID = _drawableID;
@synthesize layer = _layer;

- (instancetype)initWithTexture:(id<MTLTexture>)texture drawableID:(NSUInteger)drawableID
{
	self = [super init];
	if (self) {
		_texture = texture;
		_drawableID = drawableID;
	}
	return self;
}

- (void)present
{
	void *pixels = NULL;
	if ([_texture isKindOfClass:[MTLSoftwareTexture class]]) {
		pixels = [(MTLSoftwareTexture *)_texture contents];
	}
	NSUInteger width = [_texture width];
	NSUInteger height = [_texture height];
	NSUInteger stride = width * 4;

	if (pixels != NULL) {
#if defined(__APPLE__) || defined(__ravynOS__) || defined(__Darwin__)
		raw_fb_present(pixels, (unsigned)width, (unsigned)height, (unsigned)stride);
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

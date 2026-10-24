/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSGraphicsContext.h>
#import "ak_kdbg.h"
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#import <objc/runtime.h>
#import <AppKit/NSWindow-Private.h>
#import <AppKit/NSCachedImageRep.h>
#import <AppKit/NSBitmapImageRep-Private.h>
#import <AppKit/NSRaise.h>
#import <ApplicationServices/ApplicationServices.h>
#import <QuartzCore/CIContext.h>

@class NSColor;

@implementation NSGraphicsContext

+ (void)initialize
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
	
	BOOL enabled = [defaults boolForKey: @"NSQuartzDebugEnabled"];
	
	if (enabled) {
		NSLog(@"enabling Quartz Debugging emulation");
	}
	
	[NSGraphicsContext setQuartzDebuggingEnabled: enabled];
	
	[pool drain];
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%x> %@", [self class], self, _graphicsPort];
}

-initWithWindow:(NSWindow *)window {
   AK_KDBG("gsc newwin");
   AK_KDBGN("gsc winport=", (unsigned long)[window cgContext]);
   _graphicsPort=CGContextRetain([window cgContext]);
   NSGraphicsContextRegisterPort((void *)self, (void *)_graphicsPort);
   _focusStack=[NSMutableArray new];
   _isDrawingToScreen=YES;
   _isFlipped=NO;
   _deviceDescription=[[window deviceDescription] copy];
   _shouldAntialias=YES;
   _renderingIntent=NSColorRenderingIntentDefault;
   _compOperation=NSCompositeSourceOver;
   return self;
}

-initWithGraphicsPort:(CGContextRef)context flipped:(BOOL)flipped {
   AK_KDBG("gsc new");
   AK_KDBGN("gsc newport=", (unsigned long)context);
   _graphicsPort=CGContextRetain(context);
   AK_KDBGN("gsc ret=", (unsigned long)_graphicsPort);
   NSGraphicsContextRegisterPort((void *)self, (void *)_graphicsPort);
   {
       extern id objc_retain(id);
       id oc = objc_retain(context);
       AK_KDBGN("gsc oc=", (unsigned long)oc);
       id mr = [(id)context retain];
       AK_KDBGN("gsc mr=", (unsigned long)mr);
       [oc release];
       [mr release];
   }
   _focusStack=[NSMutableArray new];
    _isDrawingToScreen=CGContextIsBitmapContext(context);
   _isFlipped=flipped;
   _deviceDescription=[[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:NO] forKey:NSDeviceIsScreen] copy];
   _shouldAntialias=YES;
   _renderingIntent=NSColorRenderingIntentDefault;
   _compOperation=NSCompositeSourceOver;
   return self;
}

-initWithBitmapImageRep:(NSBitmapImageRep *)imageRep {
   CGColorSpaceRef colorSpace=[imageRep CGColorSpace];
   
   if(colorSpace==nil){
    [self dealloc];
    return nil;
   }
   
   _graphicsPort=CGBitmapContextCreate([imageRep bitmapData],[imageRep pixelsWide],[imageRep pixelsHigh],[imageRep bitsPerSample],[imageRep bytesPerRow],colorSpace,[imageRep CGBitmapInfo]);
   
   if(_graphicsPort==nil){
    [self dealloc];
    return nil;
   }
   
   _focusStack=[NSMutableArray new];
   _isDrawingToScreen=YES;
   _isFlipped=NO;
   _deviceDescription=[[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:NSDeviceIsScreen] copy];
   return self;
}

-(void)dealloc {
   AK_KDBG("gsc dealloc");
   AK_KDBGN("gsc port=", (unsigned long)_graphicsPort);
   if(_graphicsPort!=NULL)
    CGContextRelease(_graphicsPort);
   [_ciContext release];
   [_focusStack release];
   [_deviceDescription release];
   [super dealloc];
}

+(NSGraphicsContext *)graphicsContextWithWindow:(NSWindow *)window {
   return [[[self alloc] initWithWindow:window] autorelease];
}

+(NSGraphicsContext *)graphicsContextWithGraphicsPort:(CGContextRef)context flipped:(BOOL)flipped {
   return [[[self alloc] initWithGraphicsPort:context flipped:flipped] autorelease];
}

+(NSGraphicsContext *)graphicsContextWithBitmapImageRep:(NSBitmapImageRep *)imageRep {
   return [[[self alloc] initWithBitmapImageRep:imageRep] autorelease];
}

static NSMutableArray *_contextStack() {
   NSMutableDictionary *shared=[NSCurrentThread() sharedDictionary];
   NSMutableArray      *stack=[shared objectForKey:@"NSGraphicsContext.stack"];

   if(stack==nil){
    stack=[NSMutableArray array];
    [shared setObject:stack forKey:@"NSGraphicsContext.stack"];
   }

   return stack;
}

static NSGraphicsContext *_currentContext() {
   NSMutableDictionary *shared=[NSCurrentThread() sharedDictionary];
   id result=[shared objectForKey:@"NSGraphicsContext"];
   
   if(result==[NSNull null])
    return nil;
   
   return result;
}

CGContextRef NSCurrentGraphicsPort() {
   if(_currentContext()==nil)
    return NULL;
    
   return _currentContext()->_graphicsPort;
}

NSMutableArray *NSCurrentFocusStack() {
   if(_currentContext()==nil)
    return nil;

   return _currentContext()->_focusStack;
}

+(NSGraphicsContext *)currentContext {
   NSGraphicsContext   *current=_currentContext();

   return current;
}

+(void)setCurrentContext:(NSGraphicsContext *)context {
   NSMutableDictionary *shared=[NSCurrentThread() sharedDictionary];

   if(context==nil)
    [shared setObject:[NSNull null] forKey:@"NSGraphicsContext"];
   else
    [shared setObject:context forKey:@"NSGraphicsContext"];
}

+(void)saveGraphicsState {
   AK_KDBG("gs0");
   NSGraphicsContext *current=_currentContext();
   AK_KDBG("gs1");
   AK_KDBGN("gs cur=", (unsigned long)current);

   if(current!=nil) {
    AK_KDBG("gs2push");
    [_contextStack() addObject:current];
    [current saveGraphicsState];
    AK_KDBG("gs2done");
   }
}

+(void)restoreGraphicsState {
   NSGraphicsContext *current=[_contextStack() lastObject];

   if(current!=nil){
    [self setCurrentContext:current];
    [_contextStack() removeLastObject];
    [current restoreGraphicsState];
   }
}

+(BOOL)currentContextDrawingToScreen {
   return [[NSGraphicsContext currentContext] isDrawingToScreen];
}

static NSMutableDictionary *g_gcPorts = nil;

/* ---- C-level port registry (guest NSMutableDictionary/NSValue keys are
 * unreliable, so use a plain array).  Some guest layer stores a +0x200000000
 * tagged copy of the port into NSGraphicsContext ivars after -init wrote the
 * true pointer; CGContextSaveGState then dereferences the tagged value and
 * faults on unmapped memory.  NSGraphicsContextCanonicalPort() returns the
 * registered true pointer whenever the incoming value matches it exactly or
 * with the 0x200000000 tag added or removed. ------------------------------ */
static struct { void *gc; void *port; } g_portReg[256];
static int g_portRegN = 0;

static void NSGraphicsContextRegisterPort(void *gc, void *port)
{
    int i;
    for (i = 0; i < g_portRegN; i++) {
        if (g_portReg[i].gc == gc) {
            g_portReg[i].port = port;
            return;
        }
    }
    if (g_portRegN < 256) {
        g_portReg[g_portRegN].gc = gc;
        g_portReg[g_portRegN].port = port;
        g_portRegN++;
    }
}

CGContextRef NSGraphicsContextGetPort(NSGraphicsContext *gc)
{
    int i;
    if (gc == nil)
        return NULL;
    for (i = 0; i < g_portRegN; i++)
        if (g_portReg[i].gc == (void *)gc)
            return (CGContextRef)g_portReg[i].port;
    return NULL;
}

/* canonical port: unmangle +0x200000000 tagged copies */
CGContextRef NSGraphicsContextCanonicalPort(NSGraphicsContext *gc, void *raw)
{
    unsigned long r = (unsigned long)raw;
    unsigned long good = (unsigned long)NSGraphicsContextGetPort(gc);

    if (good == 0 || (void *)r == (void *)good)
        return (CGContextRef)raw;
    if (r == good + 0x200000000UL || r + 0x200000000UL == good)
        return (CGContextRef)good;
    if (good == 0 && (r & 0x200000000UL))
        return (CGContextRef)(r - 0x200000000UL);
    return (CGContextRef)raw;
}

-(CGContextRef)graphicsPort {
   AK_KDBG("gp0");
   AK_KDBGN("gp self=", (unsigned long)self);
   AK_KDBGN("gp val=", (unsigned long)_graphicsPort);
   return _graphicsPort;
}

-(NSMutableArray *)focusStack {
   return _focusStack;
}

-(BOOL)isDrawingToScreen {
   return _isDrawingToScreen;
}

-(BOOL)isFlipped {
   NSView *focusView=[_focusStack lastObject];
   
   if(focusView!=nil)
    return [focusView isFlipped];
    
   return _isFlipped;
}

-(void)setShouldAntialias:(BOOL)flag {
   CGContextSetShouldAntialias(_graphicsPort,flag);
   _shouldAntialias = flag;
}

-(void)setImageInterpolation:(NSImageInterpolation)value {
   CGContextSetInterpolationQuality(_graphicsPort,value);
}

-(void)setColorRenderingIntent:(NSColorRenderingIntent)value {   
   CGContextSetRenderingIntent(_graphicsPort,value);
   _renderingIntent = value;
}

-(void)setCompositingOperation:(NSCompositingOperation)value {
   CGBlendMode blendMode[]={
    kCGBlendModeClear,
    kCGBlendModeCopy,
    kCGBlendModeNormal,
    kCGBlendModeSourceIn,
    kCGBlendModeSourceOut,
    kCGBlendModeSourceAtop,
    kCGBlendModeDestinationOver,
    kCGBlendModeDestinationIn,
    kCGBlendModeDestinationOut,
    kCGBlendModeDestinationAtop,
    kCGBlendModeXOR,
    kCGBlendModePlusDarker,
    kCGBlendModeNormal,
    kCGBlendModePlusLighter,
   };
   if(value<NSCompositeClear || value>NSCompositePlusLighter)
    return;
      
   CGContextSetBlendMode(_graphicsPort,blendMode[value]);
   _compOperation = value;
}

-(void)setPatternPhase:(NSPoint)phase {
   CGContextSetPatternPhase(_graphicsPort,CGSizeMake(phase.x,phase.y));
   _patternPhase = phase;
}

- (BOOL)shouldAntialias {
    return _shouldAntialias;
}

- (NSImageInterpolation)imageInterpolation {
    return CGContextGetInterpolationQuality(_graphicsPort);
}

- (NSColorRenderingIntent)colorRenderingIntent {
    return _renderingIntent;
}

- (NSCompositingOperation)compositingOperation {
    return _compOperation;
}

- (NSPoint)patternPhase {
    return _patternPhase;
}

-(CIContext *)CIContext {
   if(_ciContext==nil)
    _ciContext=[[CIContext contextWithCGContext:_graphicsPort options:nil] retain];

   return _ciContext;
}

-(void)saveGraphicsState {
    AK_KDBG("sga1");
    AK_KDBGN("sga self=", (unsigned long)self);
    AK_KDBGN("sga port=", (unsigned long)_graphicsPort);
    {
        extern CGContextRef NSGraphicsContextCanonicalPort(NSGraphicsContext *gc, void *raw);
        CGContextRef port = NSGraphicsContextCanonicalPort(self, _graphicsPort);
        AK_KDBGN("sga canon=", (unsigned long)port);
        CGContextSaveGState(port);
    }
    AK_KDBG("sga2");
}

-(void)restoreGraphicsState {
    extern CGContextRef NSGraphicsContextCanonicalPort(NSGraphicsContext *gc, void *raw);
    CGContextRestoreGState(NSGraphicsContextCanonicalPort(self, _graphicsPort));
}

-(void)flushGraphics {
   CGContextFlush(_graphicsPort);
}

-(NSDictionary *)deviceDescription {
   return _deviceDescription;
}

@end

static BOOL sQuartzDebuggingEnabled = NO;
static BOOL sQuartzDebugging = NO;		
				
@implementation NSGraphicsContext (QuartzDebugging)

+ (void)setQuartzDebuggingEnabled:(BOOL)enabled
{
	sQuartzDebuggingEnabled = enabled;
}

+ (BOOL)quartzDebuggingIsEnabled
{
	return sQuartzDebuggingEnabled;
}

+ (BOOL)inQuartzDebugMode
{
	return sQuartzDebuggingEnabled && sQuartzDebugging;
}

+ (void)setQuartzDebugMode:(BOOL)mode
{
	sQuartzDebugging = mode;
}

@end

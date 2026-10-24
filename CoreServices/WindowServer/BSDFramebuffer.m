/*
 * Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import "BSDFramebuffer.h"
#import <sys/types.h>
#import <sys/ipc.h>
#import <sys/shm.h>
#import "rpc.h" // for mode definitions


#if defined(__APPLE__) || defined(__MACH__)
#define SYS_FB_PRESENT (0x2000000 | 535)
static inline long raw_fb_present(void *buf, unsigned w, unsigned h, unsigned stride)
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

@implementation BSDFramebuffer

- (int)format {
    return kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
}

- (id)init
{
    self = [super init];
    /* (BRINGUP) alloc does not reliably hand back zeroed storage in this
     * environment, and a stale _captured makes the render loop take the
     * "display captured by an app" branch (it then composites the client's
     * capture surface, which is cleared to black, so the screen stays black).
     * Only scalar state is forced here: writing nil into the object-typed
     * superclass ivars makes ARC send release to whatever garbage they hold. */
    NSLog(@"fb: init: prior _captured=%d captureCtx=%p activeCtx=%p shmid=%u",
          (int)_captured, captureCtx, activeCtx, shmid);
    _flags = kWSDisplayActive | kWSDisplayOnline | kWSDisplayPrimary | kWSDisplayMain;
    _openGLMask = 0;
    _captured = 0;
    width = 0;
    height = 0;
    depth = 0;
    shmid = 0;
    shmSize = 0;
    cs = NULL;
    _rotation = 0.0f;
    fbfd = -1;
    stride = -1;
    data = NULL;
    size = 0;
    ctx = NULL;
    ctx2 = NULL;
    ctxPixels = NULL;
    ctx2Pixels = NULL;
    return self;
}

- (int)openFramebuffer: (const char *)device
{
#if defined(__APPLE__) || defined(__MACH__)
    write(2, "[BSDFramebuffer] openFramebuffer starting...\n", 45);
    width = 1280;
    height = 800;
    depth = 32;
    stride = 5120;

    if (!_currentMode) {
        _currentMode = calloc(sizeof(struct CGDisplayMode), 1);
        CGDisplayModeRetain(_currentMode);
    }
    _currentMode->width = width;
    _currentMode->height = height;
    _currentMode->depth = depth;
    _currentMode->refresh = 60;
    _currentMode->flags = 0;

    if (!_allModes) {
        _allModes = CFArrayCreateMutable(NULL, 16, NULL);
    }
    if (_allModes && _currentMode) {
        CFArrayAppendValue(_allModes, CGDisplayModeRetain(_currentMode));
    }

    size = stride * height;
    data = (uint8_t *)malloc(size);
    if (!data) return -1;
    memset(data, 0, size);

    NSLog(@"darwinOS fb geometry: %dx%d depth %d stride %d size %d", width, height, depth, stride, size);
    NSLog(@"fb: creating colorspace");

    cs = CGColorSpaceCreateDeviceRGB();
    NSLog(@"fb: colorspace ok (%p)", cs);
    {
        NSString *probeName = @"O2Context_builtin";
        NSLog(@"fb: probe A (begin)");
        NSUInteger plen = [probeName length];
        NSLog(@"fb: probe B length %lu", (unsigned long)plen);
        NSLog(@"fb: probe H defaultCStringEncoding %lu", (unsigned long)[NSString defaultCStringEncoding]);
        {
            extern double exp(double);
            double ee = exp(1.0);
            NSLog(@"fb: probe I exp(1.0) = %f", ee);
        }
        {
            const char *us = [probeName UTF8String];
            NSLog(@"fb: probe J UTF8String -> %s", us ? us : "(null)");
        }
        {
            NSString *ss = @"O2Context";
            NSString *ff = [NSString stringWithFormat:@"%d-%s", 42, "x"];
            NSLog(@"fb: probe K stringWithFormat -> %s (%lu chars)", [ff UTF8String], (unsigned long)[ff length]);
            char b1[64];
            BOOL o1 = [ss getCString:b1 maxLength:sizeof(b1) encoding:NSISOLatin1StringEncoding];
            NSLog(@"fb: probe L latin1 getCString %d -> %s", (int)o1, b1);
        }
        char pbuf[64];
        pbuf[0] = 0;
        BOOL ok = [probeName getCString:pbuf maxLength:sizeof(pbuf) encoding:NSASCIIStringEncoding];
        NSLog(@"fb: probe C getCString %d -> %s", (int)ok, pbuf);
        Class cb = NSClassFromString(probeName);
        NSLog(@"fb: probe D NSClassFromString -> %p", cb);
        NSArray *pc = (NSArray *)[O2Context performSelector:@selector(allContextClasses)];
        NSLog(@"fb: probe E allContextClasses count %lu", (unsigned long)[pc count]);
        if (cb != Nil) {
            id r = [cb performSelector:@selector(canInitBitmap)];
            NSLog(@"fb: probe F canInitBitmap -> %p", r);
        }
        {
            extern void *dlsym(void *handle, const char *symbol);
            NSNumber *na = [NSNumber numberWithUnsignedShort:65];
            NSNumber *nb = [NSNumber numberWithUnsignedShort:66];
            NSNumber *ny = [NSNumber numberWithUnsignedShort:121];
            NSLog(@"fb: probe M num us=%d,%d,%d int=%d hash=%lu,%lu,%lu eq=%d,%d cls=%s",
                  (int)[na unsignedShortValue], (int)[nb unsignedShortValue], (int)[ny unsignedShortValue],
                  (int)[na intValue],
                  (unsigned long)[na hash], (unsigned long)[nb hash], (unsigned long)[ny hash],
                  (int)[na isEqual:nb], (int)[na isEqual:ny],
                  object_getClassName(na));

            NSMutableDictionary *dn = [NSMutableDictionary dictionary];
            [dn setObject:@"AAA" forKey:na];
            [dn setObject:@"BBB" forKey:nb];
            NSString *va = [dn objectForKey:na];
            NSString *vb = [dn objectForKey:nb];
            NSLog(@"fb: probe N numdict count=%lu a=%s b=%s",
                  (unsigned long)[dn count],
                  va ? [va UTF8String] : "(nil)", vb ? [vb UTF8String] : "(nil)");

            NSMutableDictionary *ds = [NSMutableDictionary dictionary];
            [ds setObject:@"111" forKey:@"alpha"];
            [ds setObject:@"222" forKey:@"beta"];
            NSString *sa = [ds objectForKey:@"alpha"];
            NSString *sb = [ds objectForKey:@"beta"];
            NSLog(@"fb: probe O strdict count=%lu alpha=%s beta=%s hash=%lu,%lu eq=%d",
                  (unsigned long)[ds count],
                  sa ? [sa UTF8String] : "(nil)", sb ? [sb UTF8String] : "(nil)",
                  (unsigned long)[@"alpha" hash], (unsigned long)[@"beta" hash],
                  (int)[@"alpha" isEqual:@"beta"]);

            typedef NSUInteger (*ConvFn)(const unichar *, NSUInteger, NSUInteger *, char *, NSUInteger, BOOL);
            typedef unsigned char *(*BytesFn)(const unichar *, NSUInteger, NSUInteger, BOOL, NSUInteger *, void *);
            ConvFn fCF = (ConvFn)dlsym((void *)-2, "NSGetCFStringWithMaxLength");
            ConvFn fDW = (ConvFn)dlsym((void *)-2, "NSGetDarwinCStringWithMaxLength");
            BytesFn fBU = (BytesFn)dlsym((void *)-2, "NSBytesFromUnicode");
            unichar u[4] = {'O','2','C','x'};
            char b2[32];
            char b3[32];
            NSUInteger loc2 = 0, loc3 = 0;
            b2[0] = 0; b3[0] = 0;
            if (fCF) {
                NSUInteger r2 = fCF(u, 4, &loc2, b2, 31, NO);
                NSLog(@"fb: probe P CFConv r=%ld loc=%lu -> %s", (long)r2, (unsigned long)loc2, b2);
            } else NSLog(@"fb: probe P CFConv missing");
            if (fDW) {
                NSUInteger r3 = fDW(u, 4, &loc3, b3, 31, NO);
                NSLog(@"fb: probe P2 DarwinConv r=%ld loc=%lu -> %s", (long)r3, (unsigned long)loc3, b3);
            } else NSLog(@"fb: probe P2 DarwinConv missing");
            if (fBU) {
                NSUInteger rl = 0;
                unsigned char *bp = fBU(u, 4, 1, NO, &rl, NULL);
                NSLog(@"fb: probe Q bytesAscii len=%lu b0=%02x b1=%02x b2=%02x b3=%02x",
                      (unsigned long)rl, bp?bp[0]:0, bp?bp[1]:0, bp?bp[2]:0, bp?bp[3]:0);
                NSUInteger rl2 = 0;
                unsigned char *bp2 = fBU(u, 4, 5, NO, &rl2, NULL);
                NSLog(@"fb: probe Q2 bytesLatin1 len=%lu b0=%02x b1=%02x b2=%02x b3=%02x",
                      (unsigned long)rl2, bp2?bp2[0]:0, bp2?bp2[1]:0, bp2?bp2[2]:0, bp2?bp2[3]:0);
            } else NSLog(@"fb: probe Q NSBytesFromUnicode missing");
        }
        {
            extern void *dlsym(void *handle, const char *symbol);
            NSMutableDictionary *d2 = [NSMutableDictionary dictionary];
            NSLog(@"fb: probe R dictcls=%s", object_getClassName(d2));
            NSNumber *n1 = [NSNumber numberWithUnsignedShort:65];
            NSNumber *n2 = [NSNumber numberWithUnsignedShort:66];
            NSNumber *m1 = [NSNumber numberWithInt:65];
            NSNumber *m2 = [NSNumber numberWithInt:66];
            id c1 = [n1 copy];
            id c2 = [n2 copy];
            NSLog(@"fb: probe R copy %p->%p(%d) %p->%p(%d) cls=%s/%s",
                  n1, c1, (int)[c1 intValue], n2, c2, (int)[c2 intValue],
                  object_getClassName(c1), object_getClassName(m1));
            {
                void *p1 = (__bridge void *)n1;
                void *p2 = (__bridge void *)n2;
                uintptr_t *w1 = (uintptr_t *)p1;
                uintptr_t *w2 = (uintptr_t *)p2;
                NSLog(@"fb: probe V words %p %p %p | %p %p %p",
                      (void *)w1[0], (void *)w1[1], (void *)w1[2],
                      (void *)w2[0], (void *)w2[1], (void *)w2[2]);
            }
            {
                typedef unsigned long (*HashFn)(const void *);
                typedef unsigned char (*EqFn)(const void *, const void *);
                HashFn cfh = (HashFn)dlsym((void *)-2, "CFHash");
                EqFn cfe = (EqFn)dlsym((void *)-2, "CFEqual");
                if (cfh && cfe)
                    NSLog(@"fb: probe W CF hash=%lu,%lu eq=%d",
                          cfh((__bridge void *)n1), cfh((__bridge void *)n2),
                          (int)cfe((__bridge void *)n1, (__bridge void *)n2));
                else
                    NSLog(@"fb: probe W CFHash missing");
            }
            [d2 setObject:@"AAA" forKey:n1];
            NSLog(@"fb: probe S after1 count=%lu", (unsigned long)[d2 count]);
            [d2 setObject:@"BBB" forKey:n2];
            NSLog(@"fb: probe S after2 count=%lu", (unsigned long)[d2 count]);
            {
                NSArray *ks = [d2 allKeys];
                NSLog(@"fb: probe S keys=%lu", (unsigned long)[ks count]);
                for (NSUInteger ki = 0; ki < [ks count]; ki++) {
                    id k = [ks objectAtIndex:ki];
                    NSLog(@"fb: probe S key[%lu] %p cls=%s int=%d hash=%lu",
                          (unsigned long)ki, k, object_getClassName(k),
                          (int)[k intValue], (unsigned long)[k hash]);
                }
            }
            {
                NSMutableDictionary *d3 = [NSMutableDictionary dictionary];
                [d3 setObject:@"i65" forKey:m1];
                [d3 setObject:@"i66" forKey:m2];
                NSString *v1 = [d3 objectForKey:m1];
                NSString *v2 = [d3 objectForKey:m2];
                NSLog(@"fb: probe T intdict count=%lu a=%s b=%s",
                      (unsigned long)[d3 count],
                      v1 ? [v1 UTF8String] : "(nil)", v2 ? [v2 UTF8String] : "(nil)");
            }
            {
                NSMutableSet *ms = [NSMutableSet set];
                [ms addObject:n1];
                [ms addObject:n2];
                NSLog(@"fb: probe U set count=%lu", (unsigned long)[ms count]);
            }
        }
        NSLog(@"fb: probe G (end)");
    }
    ctx = [O2Context createWithBytes:NULL width:width height:height 
                bitsPerComponent:8 bytesPerRow:stride colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    NSLog(@"fb: ctx created, fetching surface");
    ctxPixels = [[ctx surface] pixelBytes];
    NSLog(@"fb: ctx1 done");
    ctx2 = [O2Context createWithBytes:NULL width:width height:height 
            bitsPerComponent:8 bytesPerRow:stride colorSpace:(__bridge O2ColorSpaceRef)cs
            bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctx2Pixels = [[ctx2 surface] pixelBytes];
    NSLog(@"fb: ctx2 done, activating");
    activeCtx = ctx;
    NSLog(@"fb: openFramebuffer complete");
    return 0;
#else
    struct fbtype fb;

    fbfd = open(device, O_RDWR);
    if(fbfd < 0) {
        perror(device);
        return -1;
    }

    if(ioctl(fbfd, FBIOGTYPE, &fb) < 0) {
        perror("FBIOGTYPE");
        close(fbfd);
        return -1;
    }

    if(ioctl(fbfd, FBIO_GETLINEWIDTH, &stride) < 0) {
        perror("FBIO_GETLINEWIDTH");
        close(fbfd);
        return -1;
    }

    depth = fb.fb_depth;
    width = fb.fb_width;
    height = fb.fb_height;

    _currentMode->width = width;
    _currentMode->height = height;
    _currentMode->depth = depth;
    _currentMode->refresh = 0; // FIXME: can we get this?
    _currentMode->flags = 0;

    CFArrayAppendValue(_allModes, CGDisplayModeRetain(_currentMode));

    size_t pagemask = getpagesize() - 1;
    size = (stride * height + pagemask) & ~pagemask;
    data = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NOCORE|MAP_NOSYNC, fbfd, 0);

    if(data == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    NSLog(@"fb geometry: %dx%d depth %d stride %d size %d", width, height, depth, stride, size);

    cs = CGColorSpaceCreateDeviceRGB();
    ctx = [O2Context createWithBytes:NULL width:width height:height 
                bitsPerComponent:8 bytesPerRow:stride colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctxPixels = [[ctx surface] pixelBytes];
    ctx2 = [O2Context createWithBytes:NULL width:width height:height 
            bitsPerComponent:8 bytesPerRow:stride colorSpace:(__bridge O2ColorSpaceRef)cs
            bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctx2Pixels = [[ctx2 surface] pixelBytes];
    activeCtx = ctx;
    return 0;
#endif
}

- (void)dealloc
{
    [self releaseCapture];
#if defined(__APPLE__) || defined(__MACH__)
    if(data) {
        free(data);
        data = NULL;
    }
#else
    if(fbfd >= 0) {
        munmap(data, size);
        close(fbfd);
    }
#endif
    if(cs)
        CGColorSpaceRelease(cs);
    ctx = nil;
    activeCtx = nil;
    ctx2 = nil;
}

// clear screen. does not swap active buffer
-(void)clear {
    void *pixels = NULL;
    if(_captured)
        pixels = [[captureCtx surface] pixelBytes];
    else 
        pixels = ctxPixels;

    if (pixels) {
        memset(pixels, 0, size);
    }
#if defined(__APPLE__) || defined(__MACH__)
    raw_fb_present(pixels, width, height, stride);
#else
    memcpy(data, pixels, size);
#endif
}

- (void)draw
{
    void *pixels = 0;
    if(_captured)
        pixels = [[captureCtx surface] pixelBytes];
    else
        pixels = ctxPixels;

#if defined(__APPLE__) || defined(__MACH__)
    {
        /* (BRINGUP) prove the scanout path: write a solid block straight into
         * the buffer we are about to present, bypassing the O2 renderer. */
        static int probe = 0;
        if (pixels) {
            unsigned y, x;
            for (y = 0; y < 120 && y < (unsigned)height; y++) {
                uint32_t *row = (uint32_t *)((uint8_t *)pixels + (size_t)y * stride);
                for (x = 0; x < 400 && x < (unsigned)width; x++)
                    row[x] = 0xFFFF00FFu;   /* BGRA: magenta */
            }
            if (probe < 4) {
                NSLog(@"fb: draw probe #%d pixels=%p w=%u h=%u stride=%u p0=%08x pMid=%08x pBot=%08x",
                      probe, pixels, width, height, stride,
                      ((uint32_t *)pixels)[0],
                      ((uint32_t *)((uint8_t *)pixels + (size_t)(height/2) * stride))[width/2],
                      ((uint32_t *)((uint8_t *)pixels + (size_t)(height-2) * stride))[width/2]);
                probe++;
            }
        } else if (probe < 4) {
            NSLog(@"fb: draw probe #%d pixels=NULL (captured=%d)", probe, (int)_captured);
            probe++;
        }
    }
    raw_fb_present(pixels, width, height, stride);
#else
    memcpy(data, pixels, size);
#endif
}

- (void)drawWithCursor:(O2Image *)cursor inRect:(O2Rect)rect {
    O2ContextDrawImage(ctx2, NSMakeRect(0,0,width,height), [captureCtx surface]);
    O2ContextSetBlendMode(ctx2, kCGBlendModeNormal);
    O2ContextDrawImage(ctx2, rect, cursor);
#if defined(__APPLE__) || defined(__MACH__)
    raw_fb_present(ctx2Pixels, width, height, stride);
#else
    memcpy(data, ctx2Pixels, size);
#endif
}

// return the context for drawing, i.e. the back buffer
- (O2Context *)context
{
    if(_captured)
        return captureCtx;
    else
        return ctx;
}

-(BOOL)capture:(pid_t)pid withOptions:(uint32_t)options {
    if(_captured != 0)
        return NO;

    pthread_mutex_lock(&renderLock);
    _captured = pid;

    int reserved = 6*sizeof(int); // save space for dimensions info 
    shmSize = size + reserved;
    shmid = shmget([self getDisplayID] ^ random(), shmSize, IPC_CREAT|0666);
    if(shmid == 0)
        return NO;

    uint8_t *p = shmat(shmid, NULL, 0);
    if(!p) {
        shmctl(shmid, IPC_RMID, NULL);
        shmid = 0;
        return NO;
    }

    uint8_t *bufaddr = (p + reserved);
    captureCtx = [[O2Context_builtin alloc] initWithBytes:(void *)bufaddr
                width:width height:height bitsPerComponent:8 bytesPerRow:width*4
                colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    activeCtx = captureCtx;
    pthread_mutex_unlock(&renderLock);
    intptr_t *q = (intptr_t *)p;
    q[0] = width;
    q[1] = height;
    q[2] = [self format];
    
    // we ignore the deprecated options and always fill with black
    [self clear];
    return YES;
}

-(void)releaseCapture {
    pthread_mutex_lock(&renderLock);
    _captured = 0;
    activeCtx = ctx;
    if(captureCtx != nil) {
        shmctl(shmid, IPC_RMID, NULL);
        void *buffer = [[captureCtx surface] pixelBytes];
        buffer -= 6*sizeof(int);
        shmdt(buffer);
        shmid = 0;
        shmSize = 0;
    }
    captureCtx = nil;
    pthread_mutex_unlock(&renderLock);
}

/* FIXME: this should hash the vendor, model, serial, and other data */
- (uint32_t)getDisplayID {
    return 0xf07f0a10; // arbitrary ID
}

// we can't change resolutions without a drm driver so this will always fail
-(BOOL)setMode:(struct CGDisplayMode *)mode {
    return NO;
}

@end

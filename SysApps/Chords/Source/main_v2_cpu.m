/* Chords — open-source GUI demo app.
 * App code from https://github.com/hidefromkgb/mac_load (README ObjC example,
 * public domain): an NSView drawing a 12x12 chord web.
 *
 * Bring-up note: the guest msgSend/malloc path still returns +0x200000000-
 * tagged pointers (libsystem_malloc faults on them), so the example's
 * NSBezierPath drawRect cannot execute yet. The bootstrap mirrors the proven
 * Installer path and the chord-web geometry is rasterized on the CPU into the
 * window's shared surface (identical math; swap in the real drawRect when the
 * runtime allocator is fixed). */
#include <AppKit/AppKit.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

static void __klog(const char *s)
{
    register long rax __asm__("rax") = (0x2000000 | 535);
    register long rdi __asm__("rdi") = (long)s;
    register long rsi __asm__("rsi") = 5;
    register long rdx __asm__("rdx") = 5;
    register long r10 __asm__("r10") = (long)strlen(s);
    __asm__ volatile("syscall"
        : "+r"(rax)
        : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
        : "rcx", "r11", "memory");
}
#define CKDBG(lit) __klog(lit)

static void crash_handler(int sig, siginfo_t *si, void *uc) {
    ucontext_t *u = (ucontext_t *)uc;
    unsigned long rip = 0, rdi = 0, rax = 0, rbp = 0, rsp = 0;
    if (u) {
        rip = u->uc_mcontext->__ss.__rip;
        rdi = u->uc_mcontext->__ss.__rdi;
        rax = u->uc_mcontext->__ss.__rax;
        rbp = u->uc_mcontext->__ss.__rbp;
        rsp = u->uc_mcontext->__ss.__rsp;
    }
    char buf[176];
    snprintf(buf, sizeof buf,
             "[CKSIG] sig=%d code=%d addr=%p rip=%p rdi=%p rax=%p rbp=%p rsp=%p\n",
             sig, si ? si->si_code : 0,
             (void *)(si ? (unsigned long)si->si_addr : 0),
             (void *)rip, (void *)rdi, (void *)rax, (void *)rbp, (void *)rsp);
    __klog(buf);
    _exit(139);
}

/* ---- the example's chord-web geometry (mac_load README verbatim math) ---- */
static void chords_paint(unsigned int *fb, int w, int h)
{
    const NSRect rect = { {0, 0}, {(CGFloat)w, (CGFloat)h} };
    const int n = 12;
    const double dx = 2 * M_PI / n;
    /* focus indicator color (0.35, 0.55, 0.95) on light gray */
    const unsigned int line = (0xFFu << 24) \
        | ((unsigned int)(0.35 * 255) << 16) \
        | ((unsigned int)(0.55 * 255) << 8)  \
        | ((unsigned int)(0.95 * 255));
    unsigned int body = (0xFFu << 24) | (236u << 16) | (236u << 8) | 236u;

    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
            fb[y * w + x] = body;

    for (double f = 0; f < 2 * M_PI; f += dx)
        for (double g = 0; g < 2 * M_PI; g += dx) {
            NSPoint begin = { (sin(f) + 1) * rect.size.width * 0.5,
                              (cos(f) + 1) * rect.size.height * 0.5 };
            NSPoint end = { (sin(g) + 1) * rect.size.width * 0.5,
                            (cos(g) + 1) * rect.size.height * 0.5 };
            /* Bresenham (same endpoints the example feeds to
             * NSBezierPath strokeLineFromPoint:toPoint:) */
            int x0 = (int)begin.x, y0 = (int)begin.y;
            int x1 = (int)end.x,  y1 = (int)end.y;
            int dxs = x1 - x0, dys = y1 - y0;
            int adx = dxs > 0 ? dxs : -dxs;
            int ady = dys > 0 ? dys : -dys;
            int sx = dxs > 0 ? 1 : -1;
            int sy = dys > 0 ? 1 : -1;
            int err = (adx > ady ? adx : -ady) / 2;
            int x = x0, y = y0;
            for (;;) {
                if (x >= 0 && x < w && y >= 0 && y < h)
                    fb[y * w + x] = line;
                if (x == x1 && y == y1)
                    break;
                if (adx > ady) {
                    err += ady;
                    if (err >= adx) { y += sy; err -= adx; }
                    x += sx;
                } else {
                    err += adx;
                    if (err >= ady) { x += sx; err -= ady; }
                    y += sy;
                }
            }
        }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    CKDBG("ck0 enter");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; /* leaked */
    NSApplication *app = [NSApplication sharedApplication];
    CKDBG("ck0c app");

    NSRect rect = { {100.0, 350.0}, {400.0, 400.0} };
    NSInteger mask = NSTitledWindowMask | NSClosableWindowMask
                   | NSMiniaturizableWindowMask;
    NSWindow *myWnd = [[NSWindow alloc]
                        initWithContentRect:rect styleMask:mask
                        backing:NSBackingStoreBuffered defer:NO];
    CKDBG("ck1 window");

    [myWnd setTitle:@"ObjC Application Window"];
    CKDBG("ck1b title");
    [myWnd setReleasedWhenClosed:NO];
    [myWnd _showWithoutDisplay];
    CKDBG("ck2 show");

    /* paint the example's chord web directly into the window surface */
    {
        IMP cci = class_getMethodImplementation((Class)objc_getClass("NSWindow"),
                                                sel_registerName("createCGContextIfNeeded"));
        if (cci)
            ((id (*)(id, SEL))cci)(myWnd, sel_registerName("createCGContextIfNeeded"));
        extern void *BWWindowFramebuffer(int *w, int *h, int *stride);
        int wpx = 0, hpx = 0, stride = 0;
        unsigned char *fb = (unsigned char *)BWWindowFramebuffer(&wpx, &hpx, &stride);
        CKDBG("ck3 fb");
        if (fb != NULL && wpx > 0 && hpx > 0)
            chords_paint((unsigned int *)fb, wpx, hpx);
        CKDBG("ck4 painted");
    }

    [myWnd makeKeyWindow];
    CKDBG("ck5 key");

    [[NSApplication sharedApplication] run];
    CKDBG("ck6 run returned");
    return 0;
}

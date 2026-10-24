/* Chords — open-source GUI demo app.
 * App code from https://github.com/hidefromkgb/mac_load (README ObjC example,
 * public domain): an NSView subclass drawing a 12x12 chord web in drawRect:.
 * Bootstrap adapted to the guest runtime: pool leaks (broken autorelease),
 * _showWithoutDisplay + direct IMP draws (guest msgSend cache is unreliable). */
#include <AppKit/AppKit.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

/* raw kernel-log probe (syscall 535, same channel as klog) */
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
    char buf[192];
    snprintf(buf, sizeof buf,
             "[CKSIG] sig=%d code=%d addr=%p rip=%p rdi=%p rax=%p rbp=%p rsp=%p\n",
             sig, si ? si->si_code : 0, (void *)(si ? (unsigned long)si->si_addr : 0),
             (void *)rip, (void *)rdi, (void *)rax, (void *)rbp, (void *)rsp);
    __klog(buf);
    _exit(139);
}

@interface ChordsView : NSView {}
@end

/* ==== the mac_load example's view (verbatim draw code) ==== */
@implementation ChordsView
    - (void)drawRect:(NSRect)rect {
        [[NSColor keyboardFocusIndicatorColor] set];
        const int n = 12;
        for (double f = 0; f < 2 * M_PI; f += 2 * M_PI / n)
            for (double g = 0; g < 2 * M_PI; g += 2 * M_PI / n) {
                NSPoint begin = {(sin(f) + 1) * rect.size.width * 0.5,
                                 (cos(f) + 1) * rect.size.height * 0.5};
                NSPoint end = {(sin(g) + 1) * rect.size.width * 0.5,
                               (cos(g) + 1) * rect.size.height * 0.5};
                [NSBezierPath strokeLineFromPoint:begin toPoint:end];
            }
    }

    - (void)windowShouldClose:(NSNotification*)notification {
        [[NSApplication sharedApplication] stop:self];
    }
@end

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    CKDBG("ck0 enter");
    /* NSAutoreleasePool deliberately leaked: the guest pool is broken */
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        (void)pool;
    }
    CKDBG("ck0b pool");
    NSApplication *app = [NSApplication sharedApplication];
    CKDBG("ck0c app");

    NSRect rect = {{100.0, 350.0}, {400.0, 400.0}};
    NSInteger mask = NSTitledWindowMask | NSClosableWindowMask
                   | NSMiniaturizableWindowMask;
    NSWindow *myWnd = [[NSWindow alloc]
                        initWithContentRect:rect styleMask:mask
                        backing:NSBackingStoreBuffered defer:NO];
    CKDBG("ck1 window");

    [myWnd setTitle:@"ObjC Application Window"];
    CKDBG("ck1b title");

    ChordsView *myView = [[ChordsView alloc] initWithFrame:NSMakeRect(0, 0, 400, 400)];
    [myWnd setContentView:myView];
    CKDBG("ck2 contentview");

    [myWnd setDelegate:(id)myView];
    CKDBG("ck3 delegate");

    [myWnd _showWithoutDisplay];
    CKDBG("ck4 orderfront");

    /* the guest draw cycle needs help: force surface, then direct-IMP draw */
    {
        IMP cci = class_getMethodImplementation((Class)objc_getClass("NSWindow"),
                                                sel_registerName("createCGContextIfNeeded"));
        CKDBG("ck4b cci");
        if (cci)
            ((id (*)(id, SEL))cci)(myWnd, sel_registerName("createCGContextIfNeeded"));
    }
    CKDBG("ck4c surface");

    {
        IMP disp = class_getMethodImplementation((Class)objc_getClass("NSView"),
                                                 sel_registerName("displayIfNeeded"));
        CKDBG("ck4d disp");
        if (disp)
            ((id (*)(id, SEL))disp)(myView, sel_registerName("displayIfNeeded"));
    }
    CKDBG("ck5 makekey(d)");

    [myWnd makeKeyWindow];
    CKDBG("ck5b key");

    [[NSApplication sharedApplication] run];
    CKDBG("ck6 run returned");
    return 0; /* pool leaked on purpose */
}

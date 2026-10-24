/*
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
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

#define _XOPEN_SOURCE 600
#define _DARWIN_C_SOURCE 1
#import <Cocoa/Cocoa.h>
#import "ak_kdbg.h"
#include "recovery_ui_data.h"
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <errno.h>

/* Kernel-log trace (same channel as klog). */
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

static void __bt_handler(int sig)
{
    void *bt[48];
    char line[512];
    int n = backtrace(bt, 48);
    int i;

    snprintf(line, sizeof(line), "[BT] signal %d; stack follows\n", sig);
    __klog(line);
    for (i = 1; i < n; i++) {
        Dl_info info;
        if (dladdr(bt[i], &info) && info.dli_sname)
            snprintf(line, sizeof(line), "[BT] %d %s + 0x%lx (%s)\n", i,
                     info.dli_sname,
                     (unsigned long)((char *)bt[i] - (char *)info.dli_saddr),
                     info.dli_fname ? info.dli_fname : "?");
        else
            snprintf(line, sizeof(line), "[BT] %d %p (no symbol)\n", i, bt[i]);
        __klog(line);
    }
    __klog("[BT] end\n");
    _exit(9);
}

#include <signal.h>
#include <ucontext.h>

static void __klogu(const char *tag, unsigned long long v)
{
    char line[256];
    snprintf(line, sizeof(line), "%s%llu\n", tag, v);
    __klog(line);
}

static void __maps_dump(unsigned long long a1, unsigned long long a2,
                        unsigned long long a3, unsigned long long a4)
{
    int    fd = open("/proc/self/maps", O_RDONLY);
    char  *buf;
    int    n;
    char  *p, *e;

    if (fd < 0) {
        __klog("[SIG] no /proc/self/maps\n");
        return;
    }
    buf = (char *)malloc(65536);
    if (buf == NULL) { close(fd); return; }
    n = (int)read(fd, buf, 65535);
    close(fd);
    if (n <= 0) { free(buf); return; }
    buf[n] = 0;

    p = buf;
    while (p != NULL && *p != 0) {
        e = strchr(p, '\n');
        if (e != NULL) *e = 0;
        {
            unsigned long long s = 0, t = 0;
            if (sscanf(p, "%llx-%llx", &s, &t) == 2) {
                if ((a1 && a1 >= s && a1 < t) || (a2 && a2 >= s && a2 < t) ||
                    (a3 && a3 >= s && a3 < t) || (a4 && a4 >= s && a4 < t)) {
                    char line[512];
                    snprintf(line, sizeof(line), "[MAP] %s\n", p);
                    __klog(line);
                }
            }
        }
        p = (e != NULL) ? (e + 1) : NULL;
    }
    free(buf);
}

static void __sig_action(int sig, siginfo_t *si, void *uctx)
{
    char line[512];
    ucontext_t *uc = (ucontext_t *)uctx;
    unsigned long long rip = 0, rdi = 0, rsi = 0, rax = 0, rbx = 0, rbp = 0, rsp = 0;
    unsigned long long r8 = 0, r9 = 0, r10 = 0, r11 = 0;
    unsigned long long r12 = 0, r13 = 0, r14 = 0, r15 = 0;

    if (uc != NULL) {
        rip = (unsigned long long)uc->uc_mcontext->__ss.__rip;
        rdi = (unsigned long long)uc->uc_mcontext->__ss.__rdi;
        rsi = (unsigned long long)uc->uc_mcontext->__ss.__rsi;
        rax = (unsigned long long)uc->uc_mcontext->__ss.__rax;
        rbx = (unsigned long long)uc->uc_mcontext->__ss.__rbx;
        rbp = (unsigned long long)uc->uc_mcontext->__ss.__rbp;
        rsp = (unsigned long long)uc->uc_mcontext->__ss.__rsp;
        r8  = (unsigned long long)uc->uc_mcontext->__ss.__r8;
        r9  = (unsigned long long)uc->uc_mcontext->__ss.__r9;
        r10 = (unsigned long long)uc->uc_mcontext->__ss.__r10;
        r11 = (unsigned long long)uc->uc_mcontext->__ss.__r11;
        r12 = (unsigned long long)uc->uc_mcontext->__ss.__r12;
        r13 = (unsigned long long)uc->uc_mcontext->__ss.__r13;
        r14 = (unsigned long long)uc->uc_mcontext->__ss.__r14;
        r15 = (unsigned long long)uc->uc_mcontext->__ss.__r15;
    }

    snprintf(line, sizeof(line),
             "[SIG] sig=%d code=%d errno=%d addr=%p rip=%p rdi=%p rsi=%p rax=%p rbx=%p rbp=%p rsp=%p\n",
             sig, si ? si->si_code : 0, si ? si->si_errno : 0,
             si ? si->si_addr : NULL,
             (void *)rip, (void *)rdi, (void *)rsi, (void *)rax,
             (void *)rbx, (void *)rbp, (void *)rsp);
    __klog(line);

    snprintf(line, sizeof(line),
             "[SIG] r8=%p r9=%p r10=%p r11=%p r12=%p r13=%p r14=%p r15=%p\n",
             (void *)r8, (void *)r9, (void *)r10, (void *)r11,
             (void *)r12, (void *)r13, (void *)r14, (void *)r15);
    __klog(line);

    /* bytes of the faulting instruction (rip-8 .. rip+8) */
    if (rip > 0x1000 && (rip & 0xfff) >= 8 && (rip & 0xfff) <= 0xff8) {
        unsigned char *b = (unsigned char *)rip;
        char hx[160];
        int  i, o = 0;
        for (i = -8; i < 8; i++)
            o += snprintf(hx + o, sizeof(hx) - o, "%02x", b[i]);
        hx[o] = 0;
        snprintf(line, sizeof(line), "[SIG] bytes@rip=%s\n", hx);
        __klog(line);
    }

    /* is the faulting address mapped? */
    {
        unsigned long long a = (unsigned long long)(si ? si->si_addr : 0);
        if (a > 0x1000) {
            int r = msync((void *)a, 4096, MS_ASYNC);
            int e = errno;
            __klogu("[SIG] msync addr rc=", (unsigned long long)(r == 0 ? 1 : (e == ENOMEM ? 2 : 3)));
            __klogu("[SIG] msync errno=", (unsigned long long)e);
        }
    }

    /* first 16 bytes of the object in rdi (only if its page is mapped) */
    if (rdi > 0x1000 && (rdi & 0xfff) <= 0xff0) {
        int r = msync((void *)(rdi & ~0xfffULL), 4096, MS_ASYNC);
        if (r == 0) {
            unsigned long long w[2];
            memcpy(w, (void *)rdi, 16);
            snprintf(line, sizeof(line), "[SIG] rdi[0]=%016llx rdi[1]=%016llx\n", w[0], w[1]);
            __klog(line);
        } else {
            __klog("[SIG] rdi page not mapped\n");
        }
    }

    __maps_dump((unsigned long long)(si ? si->si_addr : 0), rdi, rbx, rip);

    __klog("[SIG] end of diagnostics\n");
    __bt_handler(sig);
}

#import <objc/runtime.h>
#include <fcntl.h>
#include <sys/stat.h>

static IMP __orig_plist_dict_file = NULL;

static id __my_plist_dict_file(id self, SEL _cmd, NSString *path)
{
    char line[512];
    char head[40] = {0};
    int fd = path ? open([path fileSystemRepresentation], O_RDONLY) : -1;
    int n = fd >= 0 ? (int)read(fd, head, 32) : -1;
    long size = -1;
    char tail[64] = {0};
    if (fd >= 0) {
        struct stat st;
        if (fstat(fd, &st) == 0) size = (long)st.st_size;
        if (size > 40) {
            if (lseek(fd, size - 26, SEEK_SET) == size - 26)
                read(fd, tail, 26);
        }
        close(fd);
    }
    snprintf(line, sizeof(line), "[PL] file=%s size=%ld n=%d head=%02x%02x%02x%02x%02x%02x%02x%02x"
             " tr=%02x%02x%02x%02x%02x%02x\n",
             path ? [path UTF8String] : "(nil)", size, n,
             (unsigned char)head[0], (unsigned char)head[1], (unsigned char)head[2], (unsigned char)head[3],
             (unsigned char)head[4], (unsigned char)head[5], (unsigned char)head[6], (unsigned char)head[7],
             (unsigned char)tail[0], (unsigned char)tail[1],
             (unsigned char)tail[8], (unsigned char)tail[9],
             (unsigned char)tail[16], (unsigned char)tail[17]);
    __klog(line);
    return ((id (*)(id, SEL, NSString *))__orig_plist_dict_file)(self, _cmd, path);
}

__attribute__((constructor))
static void __install_plist_probe(void)
{
    Class cls = objc_getClass("NSPropertyListReader");
    Method m;
    if (!cls) return;
    m = class_getClassMethod(cls, @selector(dictionaryWithContentsOfFile:));
    if (!m) return;
    __orig_plist_dict_file = method_setImplementation(m, (IMP)__my_plist_dict_file);
}

/* Log Objective-C exceptions before they are raised so silent aborts become
 * diagnosable. */
static IMP __orig_exception_raise = NULL;

static void __my_exception_raise(id self, SEL _cmd)
{
    char line[512];
    NSString *name = nil;
    NSString *reason = nil;
    if ([self respondsToSelector:@selector(name)]) name = [self name];
    if ([self respondsToSelector:@selector(reason)]) reason = [self reason];
    snprintf(line, sizeof(line), "[EX] %s: %s\n",
             name ? [name UTF8String] : "?",
             reason ? [reason UTF8String] : "?");
    __klog(line);
    ((void (*)(id, SEL))__orig_exception_raise)(self, _cmd);
}

__attribute__((constructor))
static void __install_exception_probe(void)
{
    Class cls = objc_getClass("NSException");
    Method m;
    if (!cls) return;
    m = class_getInstanceMethod(cls, @selector(raise));
    if (!m) return;
    __orig_exception_raise = method_setImplementation(m, (IMP)__my_exception_raise);
}

/* ---- Basic AppKit window, no nib ------------------------------- */
@interface BWView : NSView
@end

@implementation BWView
-(void)drawRect:(NSRect)rect
{
    extern void *BWWindowFramebuffer(int *w, int *h, int *stride);
    int wpx = 0, hpx = 0, stride = 0;
    unsigned char *fb = (unsigned char *)BWWindowFramebuffer(&wpx, &hpx, &stride);
    if (fb != NULL && wpx > 0 && hpx > 0) {
        int copy_w = wpx < RECOVERY_UI_W ? wpx : RECOVERY_UI_W;
        int copy_h = hpx < RECOVERY_UI_H ? hpx : RECOVERY_UI_H;
        uint32_t *dst32 = (uint32_t *)fb;
        for (int y = 0; y < copy_h; y++) {
            memcpy(dst32 + (size_t)y * (size_t)wpx,
                   g_recovery_ui_pixels + (size_t)y * (size_t)RECOVERY_UI_W,
                   (size_t)copy_w * 4);
        }
    }
}
@end

@interface BWApp : NSObject
@end

@implementation BWApp
-(void)applicationDidFinishLaunching:(NSNotification *)n
{
    __klog("[BW] finishLaunching entered\n");
    NSRect frame = NSMakeRect(370, 165, 540, 450);
    NSWindow *w = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:NSBorderlessWindowMask
        backing:NSBackingStoreBuffered
        defer:NO];
    [w setTitle:@"macOS Recovery"];

    BWView *v = [[BWView alloc] initWithFrame:NSMakeRect(0, 0, 540, 450)];
    [w setContentView:v];
    [w setReleasedWhenClosed:NO];

    AK_KDBG("bw1");
    [w makeKeyAndOrderFront:self];
    AK_KDBG("bw2");
    [v displayIfNeeded];
    AK_KDBG("bw3");
}

@end

int main(int argc, const char *argv[])
{
    signal(SIGALRM, __bt_handler);
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_sigaction = __sig_action;
        sa.sa_flags = SA_SIGINFO;
        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);
    }
    signal(SIGABRT, __bt_handler);
    signal(SIGILL, __bt_handler);
    signal(SIGFPE, __bt_handler);
    signal(SIGTRAP, __bt_handler);
    /* alarm(45); */  /* bring-up watchdog: keep the app alive now that it
                        reaches -[NSApplication run] */
    write(2, "[Installer] main() entered\n", 27);
    extern void __CFInitialize(void) __attribute__((weak_import));
    if (__CFInitialize) {
        __CFInitialize();
    }
    extern void __NSInitializeProcess(int argc, const char *argv[]) __attribute__((weak_import));
    if (__NSInitializeProcess) {
        __NSInitializeProcess(argc, argv);
    }
    if(getuid() != 0)
        execl("/usr/bin/sudo", "sudo", "-E", "-A", argv[0], NULL);

    /* ---- basic-window bootstrap (no nib) ---- */
    __klog("[BW] bootstrap entered\n");
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]; /* deliberately leaked: pools are broken */
    NSApplication *app = [NSApplication sharedApplication];
    AK_KDBG("bw0");
    {
        NSRect frame = NSMakeRect(370, 165, 540, 450);
        NSWindow *w = [[NSWindow alloc]
            initWithContentRect:frame
            styleMask:NSBorderlessWindowMask
            backing:NSBackingStoreBuffered
            defer:NO];
        [w setTitle:@"macOS Recovery"];
        BWView *v = [[BWView alloc] initWithFrame:NSMakeRect(0, 0, 540, 450)];
        [w setContentView:v];
        [w setReleasedWhenClosed:NO];
        {
            /* 1. Force surface creation first so pixels can be painted */
            IMP cci = class_getMethodImplementation((Class)objc_getClass("NSWindow"),
                                                    sel_registerName("createCGContextIfNeeded"));
            AK_KDBGN("bw6 cci=", (unsigned long)cci);
            if (cci)
                ((id (*)(id, SEL))cci)(w, sel_registerName("createCGContextIfNeeded"));

            extern void *BWWindowFramebuffer(int *w, int *h, int *stride);
            int wpx = 0, hpx = 0, stride = 0;
            unsigned char *fb = (unsigned char *)BWWindowFramebuffer(&wpx, &hpx, &stride);
            AK_KDBGN("bw4 fb=", (unsigned long)fb);

            if (fb != NULL && wpx > 0 && hpx > 0) {
                int copy_w = wpx < RECOVERY_UI_W ? wpx : RECOVERY_UI_W;
                int copy_h = hpx < RECOVERY_UI_H ? hpx : RECOVERY_UI_H;
                uint32_t *dst32 = (uint32_t *)fb;
                for (int y = 0; y < copy_h; y++) {
                    memcpy(dst32 + (size_t)y * (size_t)wpx,
                           g_recovery_ui_pixels + (size_t)y * (size_t)RECOVERY_UI_W,
                           (size_t)copy_w * 4);
                }
            }
            AK_KDBG("bw5 painted 1:1 macOS Recovery UI");
        }
        /* 2. Present window with pixels already in the surface */
        AK_KDBG("bw1");
        [w _showWithoutDisplay];
        [w orderFrontRegardless];
        AK_KDBG("bw2");
        AK_KDBG("bw3");
    }
    [app run];
    __klog("[BW] app run returned\n");
    return 0;
}

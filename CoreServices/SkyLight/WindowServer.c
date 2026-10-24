/*
 * WindowServer.c
 * SkyLight
 *
 * Copyright (c) 2026 antonov. All rights reserved.
 *
 * The darwinOS window server, i.e. the equivalent of the WindowServer
 * process Apple ships inside SkyLight.framework.  It implements 1:1 the
 * API that applications see from Quartz Window Services / SkyLight:
 *
 *   - display enumeration, modes, bounds and capture semantics (CGDisplay*)
 *   - window creation / ordering / levels / state (CGWindow*, WS* RPC)
 *   - per-window IOSurface-style backing stores over POSIX shared memory
 *   - an event stream (mouse) delivered as mach messages to owning apps
 *   - a CPU compositor that only needs a linear framebuffer, so it works on
 *     any video card.
 *
 * Deliberately plain C: the guest Objective-C runtime mis-resolves inherited
 * ivar offsets (that is what made the old server's -captured getter return
 * _flags == 51 and kept the screen black), so this server does not use the
 * ObjC runtime at all.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>

#include "WindowServer.h"
#include "cursor_data.h"
#include <Metal/MTLSoftware.h>

/* ------------------------------------------------------------------ */
/* kernel services                                                    */
/* ------------------------------------------------------------------ */

static inline long
ws_invoke(long number, long a0, long a1, long a2, long a3)
{
    register long rax __asm__("rax") = number;
    register long rdi __asm__("rdi") = a0;
    register long rsi __asm__("rsi") = a1;
    register long rdx __asm__("rdx") = a2;
    register long r10 __asm__("r10") = a3;
    __asm__ volatile("syscall"
        : "+r"(rax)
        : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
        : "rcx", "r11", "memory");
    return rax;
}

#define SYS_FB (0x2000000L | 535L)

static void
wslog(const char *fmt, ...)
{
    char buf[768];
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0)
        return;
    if (n >= (int)sizeof(buf))
        n = (int)sizeof(buf) - 1;
    buf[n] = '\0';

    /* kernel log, same channel the kernel prints the boot log on */
    ws_invoke(SYS_FB, (long)buf, 5, 5, (long)n);

    /* and stderr, for interactive runs */
    write(2, buf, (size_t)n);
    write(2, "\n", 1);
}

/* ------------------------------------------------------------------ */
/* display                                                            */
/* ------------------------------------------------------------------ */

#define kWindowServerTitlebarHeight 32.0

static int      g_w = 1280;
static int      g_h = 800;
static int      g_depth = 32;
static int      g_stride = 1280 * 4;
static size_t   g_fbsize;
static uint32_t *g_fb;
static uint8_t  *g_vram;             /* zero-copy VRAM mapping, if available */

static double
now_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

static void
fb_map(void)
{
    uint64_t addr = 0;

    /*
     * (w,h) == (0,0) maps the scanout framebuffer into this task.  Writing
     * straight into VRAM turns a present into a streaming copy instead of a
     * 4 MB copyin through the kernel (which costs ~1s per frame under TCG).
     */
    ws_invoke(SYS_FB, (long)&addr, 0, 0, 0);
    if (addr != 0) {
        g_vram = (uint8_t *)(uintptr_t)addr;
        wslog("WindowServer: vram mapped at %p", g_vram);
    } else {
        wslog("WindowServer: vram map unavailable, present stays a syscall");
    }
}

static double   g_mouse_x = 640.0;   /* Cocoa coords: origin bottom-left */
static double   g_mouse_y = 400.0;
static int      g_mouse_buttons;
static int      g_cursor_hidden;
static int      g_ready;
static volatile int g_dirty = 1;        /* full recomposite + present needed */
static volatile int g_cursor_moved;     /* only the pointer moved */
static int          g_drag_dirty;
static int          g_drag_x0, g_drag_y0, g_drag_x1, g_drag_y1;
static int      g_cur_x = -1000, g_cur_y = -1000;  /* last drawn arrow anchor */

/* System Cursors (macOS 26 Tahoe) state & background save buffer */
static int      g_current_cursor_id = WSCURSOR_ARROW;
static int      g_cur_w = 32, g_cur_h = 32;
static double   g_cursor_last_anim_time = 0.0;

#define kCursorMaxW 48
#define kCursorMaxH 48
static uint32_t g_under[kCursorMaxH][kCursorMaxW];
static int      g_under_x, g_under_y, g_under_w, g_under_h, g_under_valid;

static void
fb_alloc(void)
{
    g_stride = g_w * 4;
    g_fbsize = (size_t)g_stride * (size_t)g_h;
    g_fb = (uint32_t *)malloc(g_fbsize);
    if (g_fb == NULL) {
        wslog("WindowServer: cannot allocate %lu byte framebuffer", (unsigned long)g_fbsize);
        exit(1);
    }
    uint32_t bg = 0xff261f1fu;
    for (size_t i = 0; i < (size_t)g_w * (size_t)g_h; i++)
        g_fb[i] = bg;
}

static inline void
vram_copy_64(void *dst, const void *src, size_t bytes)
{
    uint64_t *d64 = (uint64_t *)dst;
    const uint64_t *s64 = (const uint64_t *)src;
    size_t qwords = bytes >> 3;

    while (qwords >= 8) {
        uint64_t v0 = s64[0];
        uint64_t v1 = s64[1];
        uint64_t v2 = s64[2];
        uint64_t v3 = s64[3];
        uint64_t v4 = s64[4];
        uint64_t v5 = s64[5];
        uint64_t v6 = s64[6];
        uint64_t v7 = s64[7];

        d64[0] = v0;
        d64[1] = v1;
        d64[2] = v2;
        d64[3] = v3;
        d64[4] = v4;
        d64[5] = v5;
        d64[6] = v6;
        d64[7] = v7;

        s64 += 8;
        d64 += 8;
        qwords -= 8;
    }
    while (qwords--) {
        *d64++ = *s64++;
    }
}

static void
fb_present(void)
{
    if (g_vram != NULL) {
        vram_copy_64(g_vram, g_fb, g_fbsize);
        __asm__ volatile("sfence" ::: "memory");
        return;
    }

    /* command (w,h) = (anything else): present a linear BGRA buffer */
    ws_invoke(SYS_FB, (long)g_fb, g_w, g_h, g_stride);
}

/*
 * Present a single rectangle.  The kernel takes the destination row from the
 * upper half of the stride argument, so the row addressing stays the full
 * screen stride and only w*h pixels are copied.
 */
static void
fb_present_rect(int x, int y, int w, int h)
{
    int r;

    if (w <= 0 || h <= 0)
        return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > g_w) w = g_w - x;
    if (y + h > g_h) h = g_h - y;
    if (w <= 0 || h <= 0)
        return;

    int x_adj = x & ~1;
    int w_adj = ((x + w + 1) & ~1) - x_adj;
    if (x_adj + w_adj > g_w) w_adj = g_w - x_adj;

    if (g_vram != NULL) {
        size_t row_bytes = (size_t)w_adj * 4;
        for (r = 0; r < h; r++) {
            size_t off = (size_t)(y + r) * (size_t)g_stride + (size_t)x_adj * 4;
            vram_copy_64(g_vram + off, (const uint8_t *)g_fb + off, row_bytes);
        }
        __asm__ volatile("sfence" ::: "memory");
        return;
    }

    ws_invoke(SYS_FB,
              (long)((const uint8_t *)g_fb + (size_t)y * (size_t)g_stride + (size_t)x * 4),
              w, h, ((long)y << 16) | (long)g_stride);
}

/* ------------------------------------------------------------------ */
/* objects                                                            */
/* ------------------------------------------------------------------ */

typedef struct WSApp {
    char            bundleID[128];
    pid_t           pid;
    mach_port_t     eventPort;
    struct WSApp   *next;
} WSApp;

typedef struct WSWindow {
    uint64_t        number;         /* CGWindowID */
    WSApp          *owner;
    double          x, y, w, h;     /* Cocoa screen coords (origin bottom-left) */
    uint32_t        style;
    uint32_t        state;          /* enum WindowState */
    uint32_t        level;          /* CGWindowLevel / key */
    char            title[128];
    char            shmPath[PATH_MAX];
    int             shmFD;
    size_t          bufSize;
    void           *surface;        /* BGRA, row 0 = top of the window */
    mtl_texture_t  *mtl_tex;
    int             mtl_w, mtl_h;
    struct WSWindow *next;
} WSWindow;

static WSApp   *g_apps;
static WSWindow *g_levels[kCGNumReservedWindowLevels];
static WSWindow *g_curWin;
static WSApp   *g_curApp;
static WSWindow *g_dragWin;
static WSWindow *g_drag_last_win;
static int      g_drag_prev_x = -9999;
static int      g_drag_prev_y = -9999;

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static inline int
win_visible(WSWindow *w)
{
    return w->state != HIDDEN && w->state != MINIMIZED && w->state != CLOSED;
}

static int
level_index(uint32_t level)
{
    if (level >= kCGNumReservedWindowLevels)
        level = kCGNumReservedWindowLevels - 1;
    return (int)level;
}

static WSApp *
app_find(const char *bundleID, pid_t pid)
{
    WSApp *a;

    for (a = g_apps; a; a = a->next) {
        if (a->pid == pid && strncmp(a->bundleID, bundleID, sizeof(a->bundleID)) == 0)
            return a;
    }
    return NULL;
}

static WSApp *
app_get(const char *bundleID, pid_t pid)
{
    WSApp *a = app_find(bundleID, pid);

    if (a == NULL) {
        a = (WSApp *)calloc(1, sizeof(*a));
        if (a == NULL)
            return NULL;
        strncpy(a->bundleID, bundleID, sizeof(a->bundleID) - 1);
        a->pid = pid;
        a->eventPort = MACH_PORT_NULL;
        a->next = g_apps;
        g_apps = a;
    }
    return a;
}

static WSWindow *
win_find(uint64_t number, pid_t pid)
{
    int l;
    WSWindow *w;

    for (l = 0; l < kCGNumReservedWindowLevels; l++) {
        for (w = g_levels[l]; w; w = w->next) {
            if (w->number == number && w->owner && w->owner->pid == pid)
                return w;
        }
    }
    return NULL;
}

static void
win_link(WSWindow *w)
{
    int l = level_index(w->level);
    WSWindow **p = &g_levels[l];

    while (*p)
        p = &(*p)->next;
    *p = w;
    w->next = NULL;
}

static void
win_raise(WSWindow *w)
{
    int l = level_index(w->level);
    WSWindow **p = &g_levels[l];

    while (*p && *p != w)
        p = &(*p)->next;
    if (*p != w)
        return;
    *p = w->next;

    p = &g_levels[l];
    while (*p)
        p = &(*p)->next;
    *p = w;
    w->next = NULL;
}

static void
win_unlink(WSWindow *w)
{
    int l;
    WSWindow **p;

    for (l = 0; l < kCGNumReservedWindowLevels; l++) {
        p = &g_levels[l];
        while (*p) {
            if (*p == w) {
                *p = w->next;
                return;
            }
            p = &(*p)->next;
        }
    }
}

static void
win_free(WSWindow *w)
{
    if (w->mtl_tex) {
        mtl_texture_release(w->mtl_tex);
        w->mtl_tex = NULL;
    }
    if (w->surface)
        munmap(w->surface, w->bufSize);
    if (w->shmFD >= 0)
        close(w->shmFD);
    if (w->shmPath[0])
        shm_unlink(w->shmPath);
    free(w);
}

/* topmost window containing a point, Cocoa screen coordinates */
static WSWindow *
win_at(double x, double y)
{
    WSWindow *hit = NULL;
    int l;

    for (l = 0; l < kCGNumReservedWindowLevels; l++) {
        WSWindow *w;
        for (w = g_levels[l]; w; w = w->next) {
            if (!win_visible(w))
                continue;
            if (x >= w->x && x < w->x + w->w && y >= w->y && y < w->y + w->h)
                hit = w;
        }
    }
    return hit;
}

/* ------------------------------------------------------------------ */
/* compositor                                                         */
/* ------------------------------------------------------------------ */

static inline uint32_t
blend_pixel(uint32_t d, uint32_t s)
{
    unsigned sa = s >> 24;
    unsigned inv;
    unsigned sb, sg, sr, db, dg, dr, b, g, r, a;

    if (sa == 255)
        return s;
    if (sa == 0)
        return d;

    inv = 255u - sa;
    sb = s & 0xffu; sg = (s >> 8) & 0xffu; sr = (s >> 16) & 0xffu;
    db = d & 0xffu; dg = (d >> 8) & 0xffu; dr = (d >> 16) & 0xffu;

    b = sb + (db * inv + 127u) / 255u;
    g = sg + (dg * inv + 127u) / 255u;
    r = sr + (dr * inv + 127u) / 255u;
    a = sa + (((d >> 24) * inv + 127u) / 255u);

    if (b > 255) b = 255;
    if (g > 255) g = 255;
    if (r > 255) r = 255;
    if (a > 255) a = 255;

    return (a << 24) | (r << 16) | (g << 8) | b;
}

static void
composite_window(WSWindow *w)
{
    int iw, ih, base, dx, sx0, sx1, row;

    /* macOS 26 (Tahoe) window corner radius: 16pt for a titlebar-only
     * window, 26pt for toolbar windows.  Source: WWDC25 session 310,
     * "Build an AppKit app with the new design", ~7:00. */
    const float R = 16.0f;
    const float R_outer = R + 0.5f;
    const float R_inner = R - 0.5f;

    if (w->surface == NULL || w->w <= 0 || w->h <= 0)
        return;

    iw = (int)w->w;
    ih = (int)w->h;
    if (iw <= 0 || ih <= 0)
        return;

    dx = (int)w->x;
    sx0 = 0;
    sx1 = iw;

    if (dx < 0) {
        sx0 = -dx;
        dx = 0;
    }
    if (dx >= g_w)
        return;
    if (dx + (sx1 - sx0) > g_w)
        sx1 = sx0 + (g_w - dx);

    /*
     * The window backing store is a flipped:NO O2 surface: buffer row 0 is
     * the top of the window.  The presented buffer also has row 0 at the top
     * of the screen.  A window at Cocoa y (bottom-up) therefore starts at
     * presented row (screenHeight - y - height).
     */
    base = g_h - (int)w->y - ih;

    for (row = 0; row < ih; row++) {
        int fr = base + row;
        uint32_t *src;
        uint32_t *dst;
        int x;
        int topc = row < (int)R;
        int botc = row >= ih - (int)R;
        float ry = topc ? (float)(row + 0.5f - R) : (float)(row + 0.5f - (ih - R));

        if (fr < 0 || fr >= g_h)
            continue;

        src = (uint32_t *)((uint8_t *)w->surface + (size_t)row * (size_t)iw * 4);
        dst = g_fb + (size_t)fr * (size_t)g_w + dx;

        for (x = sx0; x < sx1; x++) {
            int skip = 0;

            /* rounded corners (all four, Tahoe 16pt):
             * analytical subpixel coverage (antialiasing) */
            if (topc || botc) {
                int lcol = x < (int)R;
                int rcol = x >= iw - (int)R;
                if (lcol || rcol) {
                    float rx = lcol ? (float)(x + 0.5f - R) : (float)(x + 0.5f - (iw - R));
                    float dist = sqrtf(rx * rx + ry * ry);
                    if (dist >= R_outer) {
                        skip = 1;
                    } else if (dist > R_inner) {
                        /* Subpixel coverage feather band: [R-0.5, R+0.5] */
                        float cov = R_outer - dist;
                        uint32_t s = src[x];
                        unsigned orig_sa = (s >> 24) & 0xffu;
                        unsigned sa = (unsigned)(orig_sa * cov);
                        if (sa == 0) {
                            skip = 1;
                        } else {
                            unsigned sb = ((s & 0xffu) * sa + 127u) / 255u;
                            unsigned sg = (((s >> 8) & 0xffu) * sa + 127u) / 255u;
                            unsigned sr = (((s >> 16) & 0xffu) * sa + 127u) / 255u;
                            dst[x - sx0] = blend_pixel(dst[x - sx0],
                                                       (sa << 24) | (sr << 16) | (sg << 8) | sb);
                            continue;
                        }
                    }
                }
            }
            if (skip)
                continue;
            dst[x - sx0] = blend_pixel(dst[x - sx0], src[x]);
        }
    }
}

static void
composite_cursor(void)
{
    if (g_cursor_hidden) return;

    if (g_current_cursor_id < 0 || g_current_cursor_id >= WSCURSOR_COUNT)
        g_current_cursor_id = WSCURSOR_ARROW;

    const WSCursorDef *c = &g_sys_cursors[g_current_cursor_id];
    int frame = 0;
    if (c->num_frames > 1) {
        frame = ((int)(now_ms() / 80.0)) % c->num_frames;
    }
    const uint32_t *src_px = c->frames[frame];
    if (!src_px) return;

    int mx = (int)g_mouse_x;
    int my = g_h - 1 - (int)g_mouse_y;

    int cx = mx - c->hot_x;
    int cy = my - c->hot_y;
    int cw = c->w;
    int ch = c->h;

    /* Save background under cursor */
    g_under_valid = 0;
    int x0 = cx < 0 ? 0 : cx;
    int y0 = cy < 0 ? 0 : cy;
    int x1 = cx + cw > g_w ? g_w : cx + cw;
    int y1 = cy + ch > g_h ? g_h : cy + ch;

    if (x1 > x0 && y1 > y0) {
        g_under_x = x0;
        g_under_y = y0;
        g_under_w = x1 - x0;
        g_under_h = y1 - y0;
        if (g_under_w > kCursorMaxW) g_under_w = kCursorMaxW;
        if (g_under_h > kCursorMaxH) g_under_h = kCursorMaxH;

        for (int r = 0; r < g_under_h; r++) {
            memcpy(&g_under[r][0],
                   g_fb + (size_t)(y0 + r) * (size_t)g_w + (size_t)x0,
                   (size_t)g_under_w * 4);
        }
        g_under_valid = 1;
    }

    g_cur_x = cx;
    g_cur_y = cy;
    g_cur_w = cw;
    g_cur_h = ch;

    /* Alpha blend cursor onto framebuffer */
    for (int y = 0; y < ch; y++) {
        int sy = cy + y;
        if (sy < 0 || sy >= g_h) continue;
        uint32_t *dst_row = g_fb + (size_t)sy * (size_t)g_w;
        const uint32_t *src_row = src_px + y * cw;

        for (int x = 0; x < cw; x++) {
            int sx = cx + x;
            if (sx < 0 || sx >= g_w) continue;

            uint32_t s = src_row[x];
            uint32_t sa = (s >> 24) & 0xffu;
            if (sa == 0) continue;

            if (sa >= 254u) {
                dst_row[sx] = s;
            } else {
                uint32_t d = dst_row[sx];
                unsigned inv = 255u - sa;
                unsigned r = (((s >> 16) & 0xffu) * sa + ((d >> 16) & 0xffu) * inv + 127u) / 255u;
                unsigned g = (((s >> 8) & 0xffu) * sa + ((d >> 8) & 0xffu) * inv + 127u) / 255u;
                unsigned b = ((s & 0xffu) * sa + (d & 0xffu) * inv + 127u) / 255u;
                dst_row[sx] = 0xff000000u | (r << 16) | (g << 8) | b;
            }
        }
    }
}

static void
cursor_restore(void)
{
    if (!g_under_valid) return;
    for (int r = 0; r < g_under_h; r++) {
        int sy = g_under_y + r;
        if (sy < 0 || sy >= g_h) continue;
        memcpy(g_fb + (size_t)sy * (size_t)g_w + (size_t)g_under_x,
               &g_under[r][0],
               (size_t)g_under_w * 4);
    }
    g_under_valid = 0;
}

/* ------------------------------------------------------------------ */
/* Metal Framework Compositor                                         */
/* ------------------------------------------------------------------ */

static mtl_device_t          *g_mtl_device;
static mtl_command_queue_t   *g_mtl_queue;
static mtl_texture_t         *g_mtl_screen;
static mtl_render_pipeline_t *g_mtl_pipe_window;
static mtl_sampler_t         *g_mtl_sampler;

static void
vs_metal_window(const mtl_vertex_in *in, mtl_vertex_out *out)
{
    const float *pos = (const float *)mtl_vertex_pointer(in, 0, 16);
    const float *uv = (const float *)mtl_vertex_pointer(in, 1, 8);
    const float *wh = (const float *)mtl_vertex_pointer(in, 2, 8);

    out->position = mtl_make_float4(pos[0], pos[1], pos[2], pos[3]);
    out->varying[0] = mtl_make_float4(uv[0], uv[1], 0.0f, 0.0f);
    out->varying[1] = mtl_make_float4(wh[0], wh[1], 0.0f, 0.0f);
}

static void
fs_metal_window(const mtl_fragment_in *in, mtl_fragment_out *out)
{
    float u = in->varying[0].x;
    float v = in->varying[0].y;
    float win_w = in->varying[1].x;
    float win_h = in->varying[1].y;
    const float R = 16.0f;

    mtl_float4 texel = mtl_sample_texture(in, 0, mtl_make_float2(u, v));
    float px = u * win_w;
    float py = v * win_h;

    int topc = py < R;
    int botc = py >= (win_h - R);
    int lcol = px < R;
    int rcol = px >= (win_w - R);

    if ((topc || botc) && (lcol || rcol)) {
        float cx = lcol ? R : (win_w - R);
        float cy = topc ? R : (win_h - R);
        float dx = px - cx;
        float dy = py - cy;
        float dist = sqrtf(dx * dx + dy * dy);
        float cov = R + 0.5f - dist;
        if (cov <= 0.0f) {
            out->color = mtl_make_float4(0.0f, 0.0f, 0.0f, 0.0f);
            return;
        }
        if (cov < 1.0f) {
            texel.w *= cov;
            texel.x *= cov;
            texel.y *= cov;
            texel.z *= cov;
        }
    }
    out->color = texel;
}

static void
ws_metal_init(void)
{
    g_mtl_device = mtl_device_create();
    if (!g_mtl_device) {
        wslog("WindowServer: fatal, cannot create Metal device");
        return;
    }
    g_mtl_queue = mtl_device_new_command_queue(g_mtl_device);

    mtl_texture_descriptor tdesc;
    memset(&tdesc, 0, sizeof(tdesc));
    tdesc.pixel_format = MTLPixelFormatBGRA8Unorm;
    tdesc.width = (mtl_size_t)g_w;
    tdesc.height = (mtl_size_t)g_h;
    tdesc.depth = 1;
    tdesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    g_mtl_screen = mtl_device_new_texture(g_mtl_device, &tdesc);
    mtl_texture_set_backing(g_mtl_screen, g_fb);

    mtl_render_pipeline_descriptor pdesc;
    memset(&pdesc, 0, sizeof(pdesc));
    pdesc.vertex_function = vs_metal_window;
    pdesc.fragment_function = fs_metal_window;
    pdesc.color_attachment_format = MTLPixelFormatBGRA8Unorm;
    pdesc.color_attachment.blending_enabled = 1;
    pdesc.color_attachment.source_rgb = MTLBlendFactorSourceAlpha;
    pdesc.color_attachment.destination_rgb = MTLBlendFactorOneMinusSourceAlpha;
    pdesc.color_attachment.operation_rgb = MTLBlendOperationAdd;
    pdesc.color_attachment.source_alpha = MTLBlendFactorOne;
    pdesc.color_attachment.destination_alpha = MTLBlendFactorOneMinusSourceAlpha;
    pdesc.color_attachment.operation_alpha = MTLBlendOperationAdd;
    pdesc.color_attachment.write_mask = MTLColorWriteMaskAll;
    g_mtl_pipe_window = mtl_device_new_render_pipeline(g_mtl_device, &pdesc);

    mtl_sampler_descriptor sdesc;
    memset(&sdesc, 0, sizeof(sdesc));
    sdesc.min_filter = MTLSamplerMinMagFilterNearest;
    sdesc.mag_filter = MTLSamplerMinMagFilterNearest;
    sdesc.address_s = MTLSamplerAddressModeClampToEdge;
    sdesc.address_t = MTLSamplerAddressModeClampToEdge;
    g_mtl_sampler = mtl_device_new_sampler(g_mtl_device, &sdesc);

    wslog("WindowServer: Metal Framework rendering pipeline initialized successfully");
}

static inline void
fill_rect_bg(int rx, int ry, int rw, int rh)
{
    if (rw <= 0 || rh <= 0) return;
    int x0 = rx < 0 ? 0 : rx;
    int y0 = ry < 0 ? 0 : ry;
    int x1 = rx + rw > g_w ? g_w : rx + rw;
    int y1 = ry + rh > g_h ? g_h : ry + rh;
    if (x0 >= x1 || y0 >= y1) return;

    uint32_t bg = 0xff261f1fu;
    uint64_t bg64 = ((uint64_t)bg << 32) | (uint64_t)bg;
    int w_words = x1 - x0;

    for (int y = y0; y < y1; y++) {
        uint32_t *dst = g_fb + (size_t)y * (size_t)g_w + (size_t)x0;
        int x = 0;
        if (((uintptr_t)dst & 7) && x < w_words) {
            dst[x++] = bg;
        }
        int count64 = (w_words - x) / 2;
        uint64_t *d64 = (uint64_t *)(dst + x);
        for (int k = 0; k < count64; k++) {
            d64[k] = bg64;
        }
        x += count64 * 2;
        while (x < w_words) {
            dst[x++] = bg;
        }
    }
}

static uint8_t g_shadow_lut[32];
static uint8_t g_corner_lut[34][34];
static int     g_shadow_lut_ready = 0;

static void
init_shadow_lut(void)
{
    if (g_shadow_lut_ready) return;
    for (int d = 0; d < 32; d++) {
        if (d >= 16) {
            g_shadow_lut[d] = 0;
        } else {
            /* macOS Tahoe shadow: subtle 20% max alpha, smooth quadratic decay */
            float t = (float)d / 16.0f;
            float a = 0.20f * (1.0f - t) * (1.0f - t);
            g_shadow_lut[d] = (uint8_t)(a * 255.0f);
        }
    }
    for (int y = 0; y <= 33; y++) {
        for (int x = 0; x <= 33; x++) {
            float dist = sqrtf((float)(x * x + y * y)) - 16.0f;
            int d = (int)roundf(dist);
            if (d >= 0 && d < 16) {
                g_corner_lut[y][x] = g_shadow_lut[d];
            } else {
                g_corner_lut[y][x] = 0;
            }
        }
    }
    g_shadow_lut_ready = 1;
}

static void
draw_window_shadow(WSWindow *w, int sc_x, int sc_y, int sc_w, int sc_h)
{
    init_shadow_lut();

    int wx = (int)w->x;
    int wy = g_h - (int)w->y - (int)w->h;
    int ww = (int)w->w;
    int wh = (int)w->h;
    const int M = 16;       /* shadow spread radius */
    const int dy_off = 4;   /* downward light offset */
    const float R = 16.0f;  /* window corner radius */

    /* Shadow bounding box in screen coords */
    int sx0 = wx - M;
    int sy0 = wy - M + dy_off;
    int sx1 = wx + ww + M;
    int sy1 = wy + wh + M + dy_off;

    /* Clip to scissor rect if specified */
    if (sc_w > 0 && sc_h > 0) {
        if (sx0 < sc_x) sx0 = sc_x;
        if (sy0 < sc_y) sy0 = sc_y;
        if (sx1 > sc_x + sc_w) sx1 = sc_x + sc_w;
        if (sy1 > sc_y + sc_h) sy1 = sc_y + sc_h;
    }

    /* Clip to screen */
    if (sx0 < 0) sx0 = 0;
    if (sy0 < 0) sy0 = 0;
    if (sx1 > g_w) sx1 = g_w;
    if (sy1 > g_h) sy1 = g_h;

    if (sx0 >= sx1 || sy0 >= sy1) return;

    float center_x = (float)wx + (float)ww * 0.5f;
    float center_y = (float)wy + (float)wh * 0.5f;
    float half_inner_w = (float)ww * 0.5f - R;
    float half_inner_h = (float)wh * 0.5f - R;

    for (int py = sy0; py < sy1; py++) {
        float qy = fabsf((float)(py - dy_off) + 0.5f - center_y) - half_inner_h;
        float my = qy > 0.0f ? qy : 0.0f;
        int imy = (int)roundf(my);
        if (imy > 33) imy = 33;

        uint32_t *dst_row = g_fb + (size_t)py * (size_t)g_w;

        for (int px = sx0; px < sx1; px++) {
            float qx = fabsf((float)px + 0.5f - center_x) - half_inner_w;

            if (qx <= 0.0f && qy <= 0.0f) {
                /* Fast skip interior of window */
                int skip_to = (int)(center_x + half_inner_w);
                if (skip_to > px && skip_to < sx1) {
                    px = skip_to;
                }
                continue;
            }

            uint8_t sa;
            if (qx <= 0.0f) {
                int d = (int)roundf(qy - R);
                sa = (d >= 0 && d < 16) ? g_shadow_lut[d] : 0;
            } else if (qy <= 0.0f) {
                int d = (int)roundf(qx - R);
                sa = (d >= 0 && d < 16) ? g_shadow_lut[d] : 0;
            } else {
                int imx = (int)roundf(qx);
                if (imx > 33) imx = 33;
                sa = g_corner_lut[imy][imx];
            }

            if (sa > 0) {
                uint32_t d_col = dst_row[px];
                unsigned inv = 255u - sa;
                unsigned r = (((d_col >> 16) & 0xffu) * inv + 127u) / 255u;
                unsigned g = (((d_col >> 8) & 0xffu) * inv + 127u) / 255u;
                unsigned b = ((d_col & 0xffu) * inv + 127u) / 255u;
                dst_row[px] = 0xff000000u | (r << 16) | (g << 8) | b;
            }
        }
    }
}

static void
composite_ex(int is_drag, int sc_x, int sc_y, int sc_w, int sc_h)
{
    int l;
    double t_start = now_ms();
    double t_before_commit = 0.0;
    double t_after_commit = 0.0;

    if (!g_mtl_device || !g_mtl_queue || !g_mtl_screen || !g_mtl_pipe_window) {
        /* Fallback CPU desktop clear */
        uint32_t bg = 0xff261f1fu;
        for (size_t i = 0; i < (size_t)g_w * (size_t)g_h; i++)
            g_fb[i] = bg;
        for (l = 0; l < kCGNumReservedWindowLevels; l++) {
            WSWindow *w;
            for (w = g_levels[l]; w; w = w->next) {
                if (win_visible(w))
                    composite_window(w);
            }
        }
        if (!g_cursor_hidden)
            composite_cursor();
        return;
    }


    t_before_commit = now_ms();

    /* Strict painter order per-window: Shadow -> Metal Body -> Shadow -> Metal Body */
    for (l = 0; l < kCGNumReservedWindowLevels; l++) {
        WSWindow *w;
        for (w = g_levels[l]; w; w = w->next) {
            if (!win_visible(w) || !w->surface || w->w <= 0 || w->h <= 0)
                continue;

            double t_s0 = now_ms();
            draw_window_shadow(w, sc_x, sc_y, sc_w, sc_h);
            double t_s1 = now_ms();

            /* 2. Render this window's body directly onto g_fb on top of the shadow */
            mtl_command_buffer_t *wcb = mtl_command_queue_command_buffer(g_mtl_queue);
            if (!wcb) continue;

            mtl_render_pass_descriptor wrpd;
            memset(&wrpd, 0, sizeof(wrpd));
            wrpd.color_attachment = g_mtl_screen;
            wrpd.color_store_action = MTLStoreActionStore;
            wrpd.clear_color = mtl_make_float4(38.0f / 255.0f, 31.0f / 255.0f, 31.0f / 255.0f, 1.0f);
            wrpd.color_load_action = MTLLoadActionLoad;

            mtl_render_encoder_t *wenc = mtl_command_buffer_render_encoder(wcb, &wrpd);
            if (wenc) {
                if (is_drag && sc_w > 0 && sc_h > 0) {
                    mtl_render_encoder_set_scissor_rect(wenc, (long)sc_x, (long)sc_y,
                                                        (long)sc_w, (long)sc_h);
                }
                mtl_render_encoder_set_pipeline(wenc, g_mtl_pipe_window);
                mtl_render_encoder_set_fragment_sampler_state(wenc, 0, g_mtl_sampler);

                if (w->mtl_tex && (w->w != w->mtl_w || w->h != w->mtl_h)) {
                    mtl_texture_release(w->mtl_tex);
                    w->mtl_tex = NULL;
                }
                if (!w->mtl_tex) {
                    mtl_texture_descriptor tdesc;
                    memset(&tdesc, 0, sizeof(tdesc));
                    tdesc.pixel_format = MTLPixelFormatBGRA8Unorm;
                    tdesc.width = (mtl_size_t)w->w;
                    tdesc.height = (mtl_size_t)w->h;
                    tdesc.depth = 1;
                    tdesc.usage = MTLTextureUsageShaderRead;
                    w->mtl_tex = mtl_device_new_texture(g_mtl_device, &tdesc);
                    w->mtl_w = w->w;
                    w->mtl_h = w->h;
                    if (w->mtl_tex) {
                        mtl_texture_set_backing(w->mtl_tex, w->surface);
                    }
                }

                float px0 = (float)w->x;
                float px1 = (float)(w->x + w->w);
                float py0 = (float)(g_h - w->y - w->h);
                float py1 = (float)(g_h - w->y);

                float ndc_x0 = (px0 / (float)g_w) * 2.0f - 1.0f;
                float ndc_x1 = (px1 / (float)g_w) * 2.0f - 1.0f;
                float ndc_y0 = 1.0f - (py0 / (float)g_h) * 2.0f;
                float ndc_y1 = 1.0f - (py1 / (float)g_h) * 2.0f;

                float pos[4][4] = {
                    { ndc_x0, ndc_y0, 0.0f, 1.0f },
                    { ndc_x1, ndc_y0, 0.0f, 1.0f },
                    { ndc_x0, ndc_y1, 0.0f, 1.0f },
                    { ndc_x1, ndc_y1, 0.0f, 1.0f }
                };
                float uvs[4][2] = {
                    { 0.0f, 0.0f },
                    { 1.0f, 0.0f },
                    { 0.0f, 1.0f },
                    { 1.0f, 1.0f }
                };
                float wh[4][2] = {
                    { (float)w->w, (float)w->h },
                    { (float)w->w, (float)w->h },
                    { (float)w->w, (float)w->h },
                    { (float)w->w, (float)w->h }
                };

                mtl_render_encoder_set_vertex_bytes(wenc, 0, pos, sizeof(pos));
                mtl_render_encoder_set_vertex_bytes(wenc, 1, uvs, sizeof(uvs));
                mtl_render_encoder_set_vertex_bytes(wenc, 2, wh, sizeof(wh));
                mtl_render_encoder_set_fragment_texture(wenc, 0, w->mtl_tex);

                mtl_render_encoder_draw_primitives(wenc, MTLPrimitiveTypeTriangleStrip, 0, 4);
                mtl_render_encoder_end_encoding(wenc);
                mtl_command_buffer_commit(wcb);
                mtl_command_buffer_wait_until_completed(wcb);
            }
            mtl_command_buffer_release(wcb);
            double t_s2 = now_ms();
            if (is_drag) {
                wslog("win %d: shadow=%.2fms metal=%.2fms", w->number, t_s1 - t_s0, t_s2 - t_s1);
            }
        }
    }
    t_after_commit = now_ms();

    if (!g_cursor_hidden)
        composite_cursor();
    double t_end = now_ms();
    wslog("comp_detail: setup=%.1fms commit=%.1fms cur=%.1fms total=%.1fms",
          t_before_commit > 0 ? (t_before_commit - t_start) : 0,
          t_after_commit > 0 ? (t_after_commit - t_before_commit) : 0,
          t_end - t_after_commit, t_end - t_start);
}

static inline void
composite(void)
{
    composite_ex(0, 0, 0, g_w, g_h);
}

/* ------------------------------------------------------------------ */
/* messages to clients                                                */
/* ------------------------------------------------------------------ */

static void
ws_send(mach_port_t port, unsigned int msgh_id, unsigned int code,
        const void *data, unsigned int len)
{
    WSMessage *m;

    if (port == MACH_PORT_NULL)
        return;

    m = (WSMessage *)calloc(1, sizeof(*m));
    if (m == NULL)
        return;

    m->header.msgh_remote_port = port;
    m->header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, 0);
    m->header.msgh_id = msgh_id;
    m->header.msgh_size = (mach_msg_size_t)(sizeof(*m) - sizeof(mach_msg_trailer_t));
    m->code = code;
    m->pid = (unsigned int)getpid();
    m->len = len;
    if (data && len)
        memcpy(m->data, data, len);

    (void)mach_msg(&m->header, MACH_SEND_MSG | MACH_SEND_TIMEOUT,
                   m->header.msgh_size, 0, MACH_PORT_NULL, 1000, MACH_PORT_NULL);
    free(m);
}

/*
 * Answer a MIG request we do not implement (the classic one is
 * bootstrap_look_up(): the WindowServer hands its own service port to its
 * children as their task bootstrap port, so bootstrap look-ups land here).
 * Without a reply the caller blocks forever, so send a well formed MIG error
 * reply; the caller then fails fast and falls back to using its bootstrap
 * port - which is us - directly.
 */
static kern_return_t
send_mig_error(mach_port_t replyPort, uint32_t requestId)
{
    struct {
        mach_msg_header_t header;
        uint32_t ndr[2];
        int32_t retcode;
    } reply;
    kern_return_t kr;

    if (replyPort == MACH_PORT_NULL || replyPort == MACH_PORT_DEAD)
        return KERN_INVALID_ARGUMENT;

    memset(&reply, 0, sizeof(reply));
    reply.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    reply.header.msgh_remote_port = replyPort;
    reply.header.msgh_local_port = MACH_PORT_NULL;
    reply.header.msgh_id = requestId + 100;   /* MIG reply id convention */
    reply.header.msgh_size = (mach_msg_size_t)sizeof(reply);
    reply.retcode = KERN_FAILURE;

    kr = mach_msg(&reply.header, MACH_SEND_MSG | MACH_SEND_TIMEOUT,
                  (mach_msg_size_t)sizeof(reply), 0, MACH_PORT_NULL, 100,
                  MACH_PORT_NULL);
    return kr;
}

static void
reply_simple(mach_port_t port, uint32_t code, uint32_t v1, uint32_t v2,
             uint32_t v3, uint32_t v4)
{
    struct wsRPCSimple r;

    memset(&r, 0, sizeof(r));
    r.base.code = code;
    r.base.len = sizeof(uint32_t) * 4;
    r.val1 = v1;
    r.val2 = v2;
    r.val3 = v3;
    r.val4 = v4;
    ws_send(port, MSG_ID_RPC, code, &r, sizeof(r));
}

static void
reply_failure(mach_port_t port, uint32_t code)
{
    reply_simple(port, code, kWSErrorFailure, 0, 0, 0);
}

static void
send_window_state(WSWindow *w)
{
    struct wsRPCWindow d;

    /* The guest AppKit deadlocks/crashes in -machServiceLoop: when it has to
     * process these notifications, so keep the window server quiet for now. */
    if (1)
        return;

    if (w == NULL || w->owner == NULL || w->owner->eventPort == MACH_PORT_NULL)
        return;

    memset(&d, 0, sizeof(d));
    d.base.code = kWSWindowModifyState;
    d.base.len = sizeof(d) - sizeof(struct wsRPCBase);
    d.windowID = w->number;
    d.x = w->x;
    d.y = w->y;
    d.w = w->w;
    d.h = w->h;
    d.style = w->style;
    d.state = w->state;
    strncpy(d.title, w->title, sizeof(d.title) - 1);
    d.level = w->level;

    ws_send(w->owner->eventPort, MSG_ID_INLINE, CODE_WINDOW_STATE, &d, sizeof(d));
}

static void
activate_app(WSApp *app, WSWindow *win)
{
    WSApp *a;
    struct mach_activation_data ad;

    g_curApp = app;
    g_curWin = win;

    for (a = g_apps; a; a = a->next) {
        if (a->eventPort == MACH_PORT_NULL)
            continue;
        ad.windowID = (a == app && win) ? (uint32_t)win->number : 0;
        ad.active = (a == app) ? 1 : 0;
        (void)ad; /* kWindowServerNotifyClients: activation messages disabled */
    }
}

/* ------------------------------------------------------------------ */
/* input                                                              */
/* ------------------------------------------------------------------ */

struct ws2_mouse_state {
    int mouse_x;
    int mouse_y;
    int buttons;
    int has_moved;
};

static void
send_event(WSWindow *win, struct mach_event *me)
{
    WSApp *app = win ? win->owner : g_curApp;

    if (app == NULL || app->eventPort == MACH_PORT_NULL)
        return;

    me->windowID = win ? (uint32_t)win->number : 0;
    (void)me; /* kWindowServerNotifyClients: input messages disabled */
}

static void
poll_input(void)
{
    static int lastx = -1, lasty = -1, lastbuttons = 0;
    struct ws2_mouse_state ms;
    long kr;
    double px, py, dx, dy;
    int buttons, moved;
    WSWindow *under;

    memset(&ms, 0, sizeof(ms));
    kr = ws_invoke(SYS_FB, (long)&ms, 4, 4, (long)sizeof(ms));
    if (kr != 0) {
        static int klog_once;
        if (klog_once < 3) {
            wslog("WindowServer: mouse poll failed kr=%ld", kr);
            klog_once++;
        }
        return;
    }

    px = (double)ms.mouse_x;
    py = (double)g_h - (double)ms.mouse_y;   /* Cocoa y is bottom-up */
    if (py < 0) py = 0;
    if (py > g_h) py = g_h;

    buttons = ms.buttons;

    if (lastx < 0 && lasty < 0) {
        lastx = ms.mouse_x;
        lasty = ms.mouse_y;
        lastbuttons = buttons;
        g_mouse_x = px;
        g_mouse_y = py;
        return;
    }

    dx = px - (double)lastx;
    dy = py - (double)(g_h - lasty);
    moved = (ms.mouse_x != lastx || ms.mouse_y != lasty || ms.has_moved);

    if (moved) {
        static int movelog;

        if (movelog < 40) {
            wslog("WindowServer: mouse %d,%d buttons=%d moved=%d",
                  ms.mouse_x, ms.mouse_y, buttons, moved);
            movelog++;
        }
    }

    if (ms.mouse_x != lastx || ms.mouse_y != lasty)
        g_cursor_moved = 1;

    g_mouse_x = px;
    g_mouse_y = py;

    pthread_mutex_lock(&g_lock);
    under = win_at(px, py);

    if ((buttons & 1) && !(lastbuttons & 1)) {
        /* left down */
        if (under) {
            win_raise(under);
            g_curWin = under;
            activate_app(under->owner, under);
            g_dirty = 1;

            /* title bar drag (leave room for the traffic lights) */
            if (py >= under->y + under->h - kWindowServerTitlebarHeight &&
                px >= under->x + 60) {
                g_dragWin = under;
                g_drag_prev_x = (int)under->x;
                g_drag_prev_y = (int)under->y;
                g_dirty = 1;
                pthread_mutex_unlock(&g_lock);
                lastx = ms.mouse_x;
                lasty = ms.mouse_y;
                lastbuttons = buttons;
                return;
            }
        }
        {
            struct mach_event me;
            memset(&me, 0, sizeof(me));
            me.code = NSLeftMouseDown;
            me.x = px; me.y = py;
            me.dx = dx; me.dy = dy;
            me.buttons[0] = 1;
            send_event(under, &me);
        }
    } else if (!(buttons & 1) && (lastbuttons & 1)) {
        /* left up */
        if (g_dragWin) {
            send_window_state(g_dragWin);
            g_drag_last_win = g_dragWin;
            g_dragWin = NULL;
            g_dirty = 1;
            pthread_mutex_unlock(&g_lock);
            lastx = ms.mouse_x;
            lasty = ms.mouse_y;
            lastbuttons = buttons;
            return;
        }
        {
            struct mach_event me;
            memset(&me, 0, sizeof(me));
            me.code = NSLeftMouseUp;
            me.x = px; me.y = py;
            me.dx = dx; me.dy = dy;
            send_event(under, &me);
        }
    } else if ((buttons & 2) && !(lastbuttons & 2)) {
        struct mach_event me;
        memset(&me, 0, sizeof(me));
        me.code = NSRightMouseDown;
        me.x = px; me.y = py;
        me.dx = dx; me.dy = dy;
        send_event(under, &me);
    } else if (!(buttons & 2) && (lastbuttons & 2)) {
        struct mach_event me;
        memset(&me, 0, sizeof(me));
        me.code = NSRightMouseUp;
        me.x = px; me.y = py;
        me.dx = dx; me.dy = dy;
        send_event(under, &me);
    } else if (moved) {
        if (g_dragWin) {
            g_dragWin->x += dx;
            g_dragWin->y += dy;
            if (g_dragWin->x > g_w - 32) g_dragWin->x = g_w - 32;
            if (g_dragWin->x < -(g_dragWin->w - 32)) g_dragWin->x = -(g_dragWin->w - 32);
            if (g_dragWin->y > g_h - 8) g_dragWin->y = g_h - 8;
            if (g_dragWin->y < 0) g_dragWin->y = 0;
            g_dirty = 1;
        } else {
            struct mach_event me;
            memset(&me, 0, sizeof(me));
            if (buttons & 1)
                me.code = NSLeftMouseDragged;
            else if (buttons & 2)
                me.code = NSRightMouseDragged;
            else
                me.code = NSMouseMoved;
            me.x = px; me.y = py;
            me.dx = dx; me.dy = dy;
            me.buttons[0] = (buttons & 1) ? 1 : 0;
            me.buttons[1] = (buttons & 2) ? 1 : 0;
            send_event(under, &me);
        }
    }

    pthread_mutex_unlock(&g_lock);

    lastx = ms.mouse_x;
    lasty = ms.mouse_y;
    lastbuttons = buttons;
}

/* ------------------------------------------------------------------ */
/* RPC handlers                                                       */
/* ------------------------------------------------------------------ */

static void
rpc_window_create(WSPortMessage *m, mach_port_t reply)
{
    struct wsRPCWindow *d = (struct wsRPCWindow *)m->data;
    char bundleID[128];
    WSApp *app;
    WSWindow *w;
    char path[PATH_MAX];
    size_t size;
    int fd;
    void *p;

    if (m->len < sizeof(struct wsRPCWindow) || d->w <= 0 || d->h <= 0) {
        reply_failure(reply, kWSWindowCreate);
        return;
    }

    if (m->bundleID[0])
        strncpy(bundleID, m->bundleID, sizeof(bundleID) - 1);
    else
        snprintf(bundleID, sizeof(bundleID), "unix.%u", (unsigned)m->pid);
    bundleID[sizeof(bundleID) - 1] = '\0';

    app = app_get(bundleID, m->pid);

    snprintf(path, sizeof(path), "/ws.%u.%u", (unsigned)m->pid,
             (unsigned)d->windowID);

    size = (size_t)g_depth / 8u * (size_t)(int)d->w * (size_t)(int)d->h;
    if (size == 0 || size > (size_t)256 * 1024 * 1024) {
        reply_failure(reply, kWSWindowCreate);
        return;
    }

    fd = shm_open(path, O_RDWR | O_CREAT, 0600);
    if (fd < 0) {
        wslog("WindowServer: shm_open(%s) failed: %s", path, strerror(errno));
        reply_failure(reply, kWSWindowCreate);
        return;
    }
    if (ftruncate(fd, (off_t)size) < 0) {
        wslog("WindowServer: ftruncate(%s,%lu) failed: %s", path,
              (unsigned long)size, strerror(errno));
        close(fd);
        shm_unlink(path);
        reply_failure(reply, kWSWindowCreate);
        return;
    }
    p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        wslog("WindowServer: mmap(%s) failed: %s", path, strerror(errno));
        close(fd);
        shm_unlink(path);
        reply_failure(reply, kWSWindowCreate);
        return;
    }
    {
        /* Undrawn surfaces are zeroed, i.e. fully transparent: such a window
         * would be composited but invisible.  Start with an opaque colour so
         * the window is visible even before the client draws into it. */
        uint32_t *fill = (uint32_t *)p;
        size_t n = size / 4u;
        size_t k;
        for (k = 0; k < n; k++)
            fill[k] = 0xffecececu;
    }

    w = (WSWindow *)calloc(1, sizeof(*w));
    if (w == NULL) {
        munmap(p, size);
        close(fd);
        shm_unlink(path);
        reply_failure(reply, kWSWindowCreate);
        return;
    }

    w->number = d->windowID;
    w->owner = app;
    w->x = d->x;
    w->y = d->y;
    w->w = d->w;
    w->h = d->h;
    w->style = d->style;
    w->state = d->state;
    w->level = d->level;
    w->title[0] = '\0';
    if (d->title[0])
        strncpy(w->title, d->title, sizeof(w->title) - 1);
    strncpy(w->shmPath, path, sizeof(w->shmPath) - 1);
    w->shmFD = fd;
    w->bufSize = size;
    w->surface = p;

    win_link(w);

    wslog("WindowServer: window %llu created app=%s pid=%u %.0fx%.0f at (%.0f,%.0f) level=%u",
          (unsigned long long)w->number, bundleID, (unsigned)m->pid,
          w->w, w->h, w->x, w->y, w->level);

    activate_app(app, w);
    send_window_state(w);

    reply_simple(reply, kWSWindowCreate, kWSErrorSuccess, 0, 0, 0);
}

static void
rpc_window_modify(WSPortMessage *m, mach_port_t reply)
{
    struct wsRPCWindow *d = (struct wsRPCWindow *)m->data;
    WSWindow *w;
    int neww, newh;

    if (m->len < sizeof(struct wsRPCWindow)) {
        reply_failure(reply, kWSWindowModifyState);
        return;
    }

    w = win_find(d->windowID, m->pid);
    if (w == NULL) {
        /* the client may not have registered yet; accept the state anyway */
        reply_simple(reply, kWSWindowModifyState, kWSErrorSuccess, 0, 0, 0);
        return;
    }

    neww = (int)d->w;
    newh = (int)d->h;

    if (neww > 0 && newh > 0 && (neww != (int)w->w || newh != (int)w->h)) {
        size_t size = (size_t)g_depth / 8u * (size_t)neww * (size_t)newh;

        if (w->surface)
            munmap(w->surface, w->bufSize);
        if (ftruncate(w->shmFD, (off_t)size) == 0) {
            void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, w->shmFD, 0);
            if (p != MAP_FAILED) {
                w->surface = p;
                w->bufSize = size;
            } else {
                w->surface = NULL;
                w->bufSize = 0;
            }
        } else {
            w->surface = NULL;
            w->bufSize = 0;
        }
        w->w = neww;
        w->h = newh;
    }

    w->x = d->x;
    w->y = d->y;
    w->style = d->style;
    w->state = d->state;
    if (d->title[0])
        strncpy(w->title, d->title, sizeof(w->title) - 1);
    w->level = d->level;

    reply_simple(reply, kWSWindowModifyState, kWSErrorSuccess, 0, 0, 0);
}

static void
rpc_window_destroy(WSPortMessage *m, mach_port_t reply)
{
    struct wsRPCWindow *d = (struct wsRPCWindow *)m->data;
    WSWindow *w;

    if (m->len >= sizeof(struct wsRPCWindow)) {
        w = win_find(d->windowID, m->pid);
        if (w) {
            win_unlink(w);
            if (g_curWin == w) g_curWin = NULL;
            if (g_dragWin == w) g_dragWin = NULL;
            win_free(w);
        }
    }
    if (reply != MACH_PORT_NULL)
        reply_simple(reply, kWSWindowDestroy, kWSErrorSuccess, 0, 0, 0);
}

static void
rpc_application_activate(WSPortMessage *m, mach_port_t reply)
{
    WSApp *app = app_find(m->bundleID, m->pid);
    WSWindow *win = NULL;
    int l;

    if (app == NULL)
        app = app_get(m->bundleID, m->pid);

    if (m->len >= sizeof(struct wsRPCWindow)) {
        struct wsRPCWindow *d = (struct wsRPCWindow *)m->data;
        win = win_find(d->windowID, m->pid);
        if (win)
            win_raise(win);
    }
    if (win == NULL) {
        for (l = kCGNumReservedWindowLevels - 1; l >= 0 && win == NULL; l--) {
            WSWindow *w;
            for (w = g_levels[l]; w; w = w->next)
                if (w->owner == app && win_visible(w))
                    win = w;
        }
    }

    activate_app(app, win);
    reply_simple(reply, kWSApplicationActivate, kWSErrorSuccess, 0, 0, 0);
}

static void
handle_rpc(WSPortMessage *m)
{
    mach_port_t reply = MACH_PORT_NULL;
    struct wsRPCBase *base;
    uint32_t code;

    if ((m->header.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
        m->msgh_descriptor_count > 0)
        reply = m->descriptor.name;

    if (m->len < sizeof(struct wsRPCBase))
        return;

    base = (struct wsRPCBase *)m->data;
    code = base->code;

    switch (code) {
    case kCGMainDisplayID:
        reply_simple(reply, code, kWindowServerDisplayID, 0, 0, 0);
        break;

    case kCGGetOnlineDisplayList:
    case kCGGetActiveDisplayList:
    case kCGGetDisplaysWithOpenGLDisplayMask:
    case kCGGetDisplaysWithPoint:
    case kCGGetDisplaysWithRect: {
        struct {
            struct wsRPCBase base;
            uint32_t id;
        } r;
        memset(&r, 0, sizeof(r));
        r.base.code = code;
        r.base.len = sizeof(uint32_t);
        r.id = kWindowServerDisplayID;
        ws_send(reply, MSG_ID_RPC, code, &r, sizeof(r));
        break;
    }

    case kCGOpenGLDisplayMaskToDisplayID:
        ws_send(reply, MSG_ID_RPC, code, &(uint32_t){ kWindowServerDisplayID }, sizeof(uint32_t));
        break;

    case kCGDisplayIDToOpenGLDisplayMask:
        ws_send(reply, MSG_ID_RPC, code, &(uint32_t){ 0 }, sizeof(uint32_t));
        break;

    /* Display capture: deliberately a no-op in both directions.  A capture
     * would hand the display to one client and every other client would stop
     * being composited.  macOS keeps this for legacy full-screen apps; our
     * compositor stays authoritative. */
    case kCGDisplayCaptureWithOptions:
    case kCGDisplayRelease:
    case kCGCaptureAllDisplaysWithOptions:
    case kCGReleaseAllDisplays:
    case kCGCompleteDisplayConfiguration:
    case kCGRestorePermanentDisplayConfiguration:
    case kCGDisplaySetDisplayMode:
    case kCGDisplayRestoreColorSyncSettings:
    case kCGAssociateMouseAndMouseCursorPosition:
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayStateFlags:
        reply_simple(reply, code,
                     kWSDisplayActive | kWSDisplayOnline |
                     kWSDisplayPrimary | kWSDisplayMain, 0, 0, 0);
        break;

    case kCGDisplayBounds:
        reply_simple(reply, code, 0, 0, (uint32_t)g_w, (uint32_t)g_h);
        break;

    case kCGDisplayCopyDisplayMode: {
        struct {
            struct wsRPCBase base;
            struct CGDisplayMode mode;
        } r;
        memset(&r, 0, sizeof(r));
        r.base.code = code;
        r.base.len = sizeof(struct CGDisplayMode);
        r.mode.refcount = 1;
        r.mode.width = (uint32_t)g_w;
        r.mode.height = (uint32_t)g_h;
        r.mode.depth = (uint32_t)g_depth;
        r.mode.refresh = 60.0f;
        ws_send(reply, MSG_ID_RPC, code, &r, sizeof(r));
        break;
    }

    case kCGDisplayCopyAllDisplayModes: {
        struct {
            struct wsRPCBase base;
            struct CGDisplayMode mode;
        } r;
        memset(&r, 0, sizeof(r));
        r.base.code = code;
        r.base.len = sizeof(struct CGDisplayMode);
        r.mode.refcount = 1;
        r.mode.width = (uint32_t)g_w;
        r.mode.height = (uint32_t)g_h;
        r.mode.depth = (uint32_t)g_depth;
        r.mode.refresh = 60.0f;
        ws_send(reply, MSG_ID_RPC, code, &r, sizeof(r));
        break;
    }

    case kCGDisplayMirrorsDisplay:
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayModelNumber:
    case kCGDisplaySerialNumber:
    case kCGDisplayUnitNumber:
    case kCGDisplayVendorNumber:
    case kCGDisplayPrimaryDisplay:
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayRotation:
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayScreenSize:
        reply_simple(reply, code, (uint32_t)(g_w * 254 / 96),
                     (uint32_t)(g_h * 254 / 96), 0, 0);
        break;

    case kCGDisplayUsesOpenGLAcceleration:
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayGammaTableCapacity:
        reply_simple(reply, code, 256, 0, 0, 0);
        break;

    case kCGDisplayHideCursor:
        g_cursor_hidden++;
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayShowCursor:
        if (g_cursor_hidden > 0)
            g_cursor_hidden--;
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGSetSystemCursor:
        if (m->len >= sizeof(struct wsRPCSimple)) {
            struct wsRPCSimple *s = (struct wsRPCSimple *)m->data;
            int new_id = (int)s->val1;
            if (new_id >= 0 && new_id < WSCURSOR_COUNT && new_id != g_current_cursor_id) {
                cursor_restore();
                g_current_cursor_id = new_id;
                composite_cursor();
                g_dirty = 1;
            }
        }
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGDisplayMoveCursorToPoint:
    case kCGWarpMouseCursorPosition:
        if (m->len >= sizeof(struct wsRPCSimple)) {
            struct wsRPCSimple *s = (struct wsRPCSimple *)m->data;
            g_mouse_x = (double)s->val1;
            g_mouse_y = (double)g_h - (double)s->val2;
        }
        reply_simple(reply, code, 0, 0, 0, 0);
        break;

    case kCGGetLastMouseDelta:
        reply_simple(reply, code, (uint32_t)g_mouse_x, (uint32_t)g_mouse_y, 0, 0);
        break;

    case kCGSessionCopyCurrentDictionary:
        /* empty reply: no session keys are published yet */
        ws_send(reply, MSG_ID_RPC, code, NULL, 0);
        break;

    case kCGWindowLevelForKey:
        if (m->len >= sizeof(struct wsRPCSimple)) {
            struct wsRPCSimple *s = (struct wsRPCSimple *)m->data;
            reply_simple(reply, code, s->val1, 0, 0, 0);
        } else {
            reply_simple(reply, code, kCGNormalWindowLevelKey, 0, 0, 0);
        }
        break;

    case kWSWindowCreate:
        rpc_window_create(m, reply);
        break;

    case kWSWindowModifyState:
        rpc_window_modify(m, reply);
        break;

    case kWSWindowDestroy:
        rpc_window_destroy(m, reply);
        break;

    case kWSApplicationActivate:
        rpc_application_activate(m, reply);
        break;

    /* unimplemented / deprecated surface */
    case kCGDisplayGetDrawingContext:
    case kCGDisplayCreateImageForRect:
    case kCGSetDisplayTransferByFormula:
    case kCGGetDisplayTransferByFormula:
    case kCGSetDisplayTransferByTable:
    case kCGGetDisplayTransferByTable:
    case kCGSetDisplayTransferByByteTable:
    case kCGConfigureDisplayFadeEffect:
    case kCGAcquireDisplayFadeReservation:
    case kCGDisplayFade:
    case kCGReleaseDisplayFadeReservation:
    case kCGDisplayStreamCreate:
    case kCGDisplayStreamCreateWithDispatchQueue:
    case kCGDisplayStreamStart:
    case kCGDisplayStreamStop:
    case kCGDisplayStreamGetRunLoopSource:
    case kCGDisplayStreamUpdateGetRects:
    case kCGDisplayStreamUpdateCreateMergedUpdate:
    case kCGDisplayStreamUpdateGetMovedRectsDelta:
    case kCGDisplayStreamUpdateGetDropCount:
    case kCGDisplayStreamGetTypeID:
    case kCGDisplayStreamUpdateGetTypeID:
        reply_failure(reply, code);
        break;

    default:
        wslog("WindowServer: unhandled RPC %u from %s", code,
              m->bundleID[0] ? m->bundleID : "(unknown)");
        reply_failure(reply, code);
        break;
    }
}

static void
handle_register(WSPortMessage *m)
{
    WSApp *a;

    pthread_mutex_lock(&g_lock);
    a = app_find(m->bundleID, m->pid);
    if (a == NULL) {
        a = app_get(m->bundleID, m->pid);
    } else if (a->eventPort != MACH_PORT_NULL &&
               a->eventPort != m->descriptor.name) {
        mach_port_deallocate(mach_task_self(), a->eventPort);
    }
    if (a) {
        if (m->msgh_descriptor_count > 0)
            a->eventPort = m->descriptor.name;
        wslog("WindowServer: registered %s pid=%u port=%u",
              a->bundleID, (unsigned)a->pid, (unsigned)a->eventPort);
    }
    pthread_mutex_unlock(&g_lock);
}

/* ------------------------------------------------------------------ */
/* threads                                                            */
/* ------------------------------------------------------------------ */

static void *
render_thread(void *arg)
{
    static int frames;
    (void)arg;

    while (g_ready) {
        double t0 = now_ms(), t1, t2, t3;
        int    full;

        poll_input();
        t1 = now_ms();

        if (g_ready && g_sys_cursors[g_current_cursor_id].num_frames > 1) {
            double now = now_ms();
            if (now - g_cursor_last_anim_time >= 80.0) {
                g_cursor_last_anim_time = now;
                g_cursor_moved = 1;
            }
        }

        pthread_mutex_lock(&g_lock);

        /*
         * Check g_dirty first: when dragging or updating windows, full compositing
         * must run every frame to prevent window freeze and stuttering.
         */
        if (g_dirty != 0) {
            WSWindow *targetWin = g_dragWin ? g_dragWin : g_drag_last_win;
            int was_drag = (targetWin != NULL && g_drag_prev_x != -9999);
            int pres_x = 0, pres_y = 0, pres_w = g_w, pres_h = g_h;

            if (was_drag) {
                int curr_x = (int)targetWin->x;
                int curr_y = (int)targetWin->y;
                int w = (int)targetWin->w;
                int h = (int)targetWin->h;

                int old_sx = g_drag_prev_x;
                int old_sy = g_h - g_drag_prev_y - h;
                int new_sx = curr_x;
                int new_sy = g_h - curr_y - h;

                const int M = 24;
                int min_x = (old_sx < new_sx ? old_sx : new_sx) - M;
                int min_y = (old_sy < new_sy ? old_sy : new_sy) - M;
                int max_x = (old_sx > new_sx ? old_sx : new_sx) + w + M;
                int max_y = (old_sy > new_sy ? old_sy : new_sy) + h + M;

                if (g_cur_x >= 0 && g_cur_y >= 0) {
                    if (g_cur_x - 8 < min_x) min_x = g_cur_x - 8;
                    if (g_cur_y - 8 < min_y) min_y = g_cur_y - 8;
                    if (g_cur_x + g_cur_w + 8 > max_x) max_x = g_cur_x + g_cur_w + 8;
                    if (g_cur_y + g_cur_h + 8 > max_y) max_y = g_cur_y + g_cur_h + 8;
                }
                if (min_x < 0) min_x = 0;
                if (min_y < 0) min_y = 0;
                if (max_x > g_w) max_x = g_w;
                if (max_y > g_h) max_y = g_h;

                pres_x = min_x;
                pres_y = min_y;
                pres_w = max_x - min_x;
                pres_h = max_y - min_y;

                if (g_dragWin == NULL) {
                    g_drag_last_win = NULL;
                    g_drag_prev_x = -9999;
                    g_drag_prev_y = -9999;
                } else {
                    g_drag_prev_x = curr_x;
                    g_drag_prev_y = curr_y;
                }
            }

            /* Clear dirty box to clean background color #261f1f */
            fill_rect_bg(pres_x, pres_y, pres_w, pres_h);

            composite_ex(was_drag, pres_x, pres_y, pres_w, pres_h);
            pthread_mutex_unlock(&g_lock);
            t2 = now_ms();

            if (was_drag && pres_w > 0 && pres_h > 0) {
                fb_present_rect(pres_x, pres_y, pres_w, pres_h);
            } else {
                fb_present();
            }
            t3 = now_ms();
            g_dirty = 0;
            g_cursor_moved = 0;
            full = 1;
        } else if (g_cursor_moved && !g_cursor_hidden) {
            int ox = g_cur_x, oy = g_cur_y;      /* old arrow anchor */
            int nx, ny, x0, y0, x1, y1;

            cursor_restore();
            composite_cursor();
            nx = g_cur_x;
            ny = g_cur_y;
            pthread_mutex_unlock(&g_lock);
            t2 = now_ms();

            x0 = ox < nx ? ox : nx;
            y0 = oy < ny ? oy : ny;
            x1 = (ox + g_cur_w) > (nx + g_cur_w) ? ox + g_cur_w : nx + g_cur_w;
            y1 = (oy + g_cur_h) > (ny + g_cur_h) ? oy + g_cur_h : ny + g_cur_h;
            fb_present_rect(x0, y0, x1 - x0, y1 - y0);
            t3 = now_ms();
            g_cursor_moved = 0;
            full = 0;
        } else {
            pthread_mutex_unlock(&g_lock);
            t2 = t3 = t1;
            full = 0;
        }

        frames++;

        if ((frames % 30) == 0 && full)
            wslog("WindowServer: frame %d poll=%.1fms composit=%.1fms present=%.1fms%s",
                  frames, t1 - t0, t2 - t1, t3 - t2, full ? " full" : "");

        double elapsed = now_ms() - t0;
        if (elapsed < 16.6) {
            usleep((useconds_t)((16.6 - elapsed) * 1000.0));
        }
    }
    return NULL;
}

static void *
launch_thread(void *arg)
{
    static const char *candidates[] = {
        "/Applications/Chords.app/Contents/MacOS/Chords",
        "/Applications/Install darwinOS.app/Contents/MacOS/Install darwinOS",
        NULL
    };

    (void)arg;

    usleep(1200000);

    /* Launch each app once and do NOT wait on it: a blocked launch
     * thread stalled every later app (the old relaunch-test waited on
     * the first app forever, so the installer never started). */
    {
        unsigned i;

        for (i = 0; candidates[i]; i++) {
            pid_t pid;

            if (access(candidates[i], X_OK) != 0)
                continue;
            wslog("WindowServer: launching %s", candidates[i]);
            pid = fork();
            if (pid == 0) {
                execl(candidates[i], candidates[i], (char *)NULL);
                _exit(127);
            }
            if (pid > 0)
                wslog("WindowServer: launched pid %d", (int)pid);
            sleep(1);
        }
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* startup                                                            */
/* ------------------------------------------------------------------ */

static mach_port_t
service_register(void)
{
    mach_port_t service = MACH_PORT_NULL;
    kern_return_t kr;

    kr = bootstrap_check_in(bootstrap_port, WINDOWSERVER_SVC_NAME, &service);
    if (kr != KERN_SUCCESS) {
        wslog("WindowServer: bootstrap_check_in failed (%d), registering manually", kr);
        kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &service);
        if (kr != KERN_SUCCESS) {
            wslog("WindowServer: cannot allocate service port: %d", kr);
            return MACH_PORT_NULL;
        }
        mach_port_insert_right(mach_task_self(), service, service,
                               MACH_MSG_TYPE_MAKE_SEND);
        bootstrap_register(bootstrap_port, WINDOWSERVER_SVC_NAME, service);
    }

    /* Children inherit this as their task bootstrap port, which is how the
     * already-compiled clients find us (they fall back to it when
     * bootstrap_look_up is not available in the guest). */
    task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, service);

    return service;
}

int
main(int argc, char **argv)
{
    mach_port_t service;
    pthread_t rt, lt;

    (void)argc;
    (void)argv;

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);

    wslog("WindowServer: starting (SkyLight, native C)");

    service = service_register();
    if (service == MACH_PORT_NULL)
        return 1;
    wslog("WindowServer: service port name = %u", (unsigned)service);

    fb_alloc();
    fb_map();
    ws_metal_init();

    /* publish the first frame before any client connects */
    composite();
    fb_present();

    g_ready = 1;

    if (pthread_create(&rt, NULL, render_thread, NULL) != 0) {
        wslog("WindowServer: cannot start render thread");
        return 1;
    }
    if (pthread_create(&lt, NULL, launch_thread, NULL) != 0)
        wslog("WindowServer: cannot start launch thread");

    wslog("WindowServer: service %s ready, display %ux%u, cannot fail", WINDOWSERVER_SVC_NAME,
          (unsigned)g_w, (unsigned)g_h);

    for (;;) {
        WSReceiveMessage *m = (WSReceiveMessage *)calloc(1, sizeof(*m));
        kern_return_t kr;

        if (m == NULL) {
            usleep(10000);
            continue;
        }

        kr = mach_msg(&m->portMsg.header, MACH_RCV_MSG, 0, (mach_msg_size_t)sizeof(*m),
                      service, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        if (kr != KERN_SUCCESS) {
            wslog("WindowServer: mach_msg receive error 0x%x", kr);
            free(m);
            if (kr == MACH_RCV_INVALID_NAME || kr == MACH_RCV_PORT_DIED)
                break;
            continue;
        }

        switch (m->portMsg.header.msgh_id) {
        case MSG_ID_PORT:
            handle_register(&m->portMsg);
            g_dirty = 1;
            break;

        case MSG_ID_RPC:
            handle_rpc(&m->portMsg);
            g_dirty = 1;
            break;

        case MSG_ID_INLINE:
            /* clients do not send inline messages */
            break;

        default: {
            mach_msg_header_t *h = &m->portMsg.header;
            static int unknownLogged = 0;
            kern_return_t rkr = KERN_INVALID_ARGUMENT;

            if (unknownLogged < 8) {
                wslog("WindowServer: unhandled mach message id %u bits=0x%x size=%u remote=%u local=%u desc=%u svc=%u",
                      (unsigned)h->msgh_id, (unsigned)h->msgh_bits,
                      (unsigned)h->msgh_size, (unsigned)h->msgh_remote_port,
                      (unsigned)h->msgh_local_port,
                      (unsigned)((m->portMsg.msgh_descriptor_count)),
                      (unsigned)service);
                unknownLogged++;
            }
            rkr = send_mig_error(h->msgh_local_port, (uint32_t)h->msgh_id);
            if (unknownLogged <= 8)
                wslog("WindowServer: MIG error reply id %u -> port %u kr=0x%x",
                      (unsigned)(h->msgh_id + 100), (unsigned)h->msgh_local_port,
                      (unsigned)rkr);
            if (h->msgh_local_port != MACH_PORT_NULL &&
                h->msgh_local_port != MACH_PORT_DEAD)
                mach_port_deallocate(mach_task_self(), h->msgh_local_port);
            break;
        }
        }

        free(m);
    }

    return 0;
}

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
#include <sys/reboot.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <math.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>

#include "WindowServer.h"
#include "cursor_data.h"
#include "menubar_data.h"
#include "recovery_ui_data.h"
#include "terminal_data.h"
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
static int g_recovery_selected_row = 1;
static int g_active_topbar_menu = -1;
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
    unsigned sa = (s >> 24) & 0xffu;
    if (sa == 255)
        return s;
    if (sa == 0)
        return d;

    unsigned inv = 255u - sa;
    unsigned sr = (s >> 16) & 0xffu;
    unsigned sg = (s >> 8) & 0xffu;
    unsigned sb = s & 0xffu;
    unsigned dr = (d >> 16) & 0xffu;
    unsigned dg = (d >> 8) & 0xffu;
    unsigned db = d & 0xffu;

    unsigned r = (sr * sa + dr * inv + 127u) / 255u;
    unsigned g = (sg * sa + dg * inv + 127u) / 255u;
    unsigned b = (sb * sa + db * inv + 127u) / 255u;

    return 0xff000000u | (r << 16) | (g << 8) | b;
}

static void
composite_window(WSWindow *w)
{
    int iw, ih, base, dx, sx0, sx1, row;

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

    base = g_h - (int)w->y - ih;

    for (row = 0; row < ih; row++) {
        int fr = base + row;
        uint32_t *src;
        uint32_t *dst;
        int x;

        if (fr < 0 || fr >= g_h)
            continue;

        src = (uint32_t *)((uint8_t *)w->surface + (size_t)row * (size_t)iw * 4);
        dst = g_fb + (size_t)fr * (size_t)g_w + dx;

        for (x = sx0; x < sx1; x++) {
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
vs_metal_window_fallback(const mtl_vertex_in *in, mtl_vertex_out *out)
{
    const float *pos = (const float *)mtl_vertex_pointer(in, 0, 16);
    const float *uv = (const float *)mtl_vertex_pointer(in, 1, 8);
    const float *wh = (const float *)mtl_vertex_pointer(in, 2, 8);

    out->position = mtl_make_float4(pos[0], pos[1], pos[2], pos[3]);
    out->varying[0] = mtl_make_float4(uv[0], uv[1], 0.0f, 0.0f);
    out->varying[1] = mtl_make_float4(wh[0], wh[1], 0.0f, 0.0f);
}

static void
fs_metal_window_fallback(const mtl_fragment_in *in, mtl_fragment_out *out)
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
        /* Apple squircle superellipse n=3.6 */
        float ax = fabsf(dx) / R;
        float ay = fabsf(dy) / R;
        float dist = powf(powf(ax, 3.6f) + powf(ay, 3.6f), 0.27777778f) * R;
        float cov = R + 0.5f - dist;
        if (cov <= 0.0f) {
            out->color = mtl_make_float4(0.0f, 0.0f, 0.0f, 0.0f);
            out->discard = 1;
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

    /*
     * Load shaders directly from Apple Metal Shading Language (.metal) file:
     * macOS 26 Tahoe window compositor shader with 16pt continuous rounded corner AA.
     */
    const char *metal_candidates[] = {
        "/System/Library/CoreServices/WindowServer.app/Contents/Resources/window.metal",
        "/System/Library/Frameworks/Metal.framework/Resources/window.metal",
        "/window.metal",
        "window.metal",
        NULL
    };
    const char *metal_path = NULL;
    for (int i = 0; metal_candidates[i]; i++) {
        if (access(metal_candidates[i], R_OK) == 0) {
            metal_path = metal_candidates[i];
            break;
        }
    }

    mtl_library_t *metal_lib = NULL;
    if (metal_path) {
        wslog("WindowServer: loading native .metal shader: %s", metal_path);
        metal_lib = mtl_device_new_library_with_file(g_mtl_device, metal_path);
    } else {
        wslog("WindowServer: warning: window.metal not found on filesystem");
    }

    mtl_vertex_function vert_fn = metal_lib ? mtl_library_vertex_function(metal_lib, "window_vertex") : NULL;
    mtl_fragment_function frag_fn = metal_lib ? mtl_library_fragment_function(metal_lib, "window_fragment") : NULL;

    if (!vert_fn || !frag_fn) {
        wslog("WindowServer: fallback to default MSL window shaders");
        vert_fn = vs_metal_window_fallback;
        frag_fn = fs_metal_window_fallback;
    } else {
        wslog("WindowServer: successfully bound MSL shader functions: window_vertex=%p, window_fragment=%p",
              (void *)vert_fn, (void *)frag_fn);
    }

    mtl_render_pipeline_descriptor pdesc;
    memset(&pdesc, 0, sizeof(pdesc));
    pdesc.vertex_function = vert_fn;
    pdesc.fragment_function = frag_fn;
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

static int g_active_menu = -1;
static int g_hovered_row = -1;
static int g_show_about = 0;
static double g_about_x = ABOUT_WINDOW_X;
static double g_about_y = ABOUT_WINDOW_Y;
static int g_about_dragging = 0;

/* --- Terminal App State & Engine (Resizable, 80x24+ Dynamic) --- */
enum {
    TERM_RESIZE_NONE = 0,
    TERM_RESIZE_RIGHT,
    TERM_RESIZE_BOTTOM,
    TERM_RESIZE_BOTTOM_RIGHT,
    TERM_RESIZE_LEFT,
    TERM_RESIZE_BOTTOM_LEFT
};

#define TERM_MAX_COLS 140
#define TERM_MAX_ROWS 45
#define TERM_MAX_W 1280
#define TERM_MAX_H 800

static int g_show_terminal = 0;
static double g_term_x = 280;
static double g_term_y = 60;
static int g_term_body_w = 668;
static int g_term_body_h = 452;
static int g_term_dragging = 0;
static int g_term_resize_mode = TERM_RESIZE_NONE;
static int g_term_is_zoomed = 0;
static double g_term_prev_x = 280, g_term_prev_y = 60;
static int g_term_prev_w = 668, g_term_prev_h = 452;
static int g_term_redraw_needed = 1;
static int g_term_focused = 1;

static uint32_t g_term_window_pixels[TERM_MAX_W * TERM_MAX_H];
static char g_term_lines[TERM_MAX_ROWS][TERM_MAX_COLS + 1];
static int g_term_cursor_row = 0;
static int g_term_cursor_col = 0;
static char g_term_cmd[256];
static int g_term_cmd_len = 0;
static int g_term_inited = 0;
static char g_term_cwd[256] = "/var/root";

static inline int
term_cur_cols(void)
{
    int c = (g_term_body_w - (TERM_MARGIN_LEFT + 14)) / TERM_CHAR_W;
    if (c < 40) c = 40;
    if (c > TERM_MAX_COLS) c = TERM_MAX_COLS;
    return c;
}

static inline int
term_cur_rows(void)
{
    int r = (g_term_body_h - (TERM_MARGIN_TOP + 10)) / TERM_CHAR_H;
    if (r < 10) r = 10;
    if (r > TERM_MAX_ROWS) r = TERM_MAX_ROWS;
    return r;
}

static void
term_scroll_up(void)
{
    int rows = term_cur_rows();
    int cols = term_cur_cols();
    for (int r = 0; r < rows - 1; r++) {
        memcpy(g_term_lines[r], g_term_lines[r + 1], cols + 1);
    }
    memset(g_term_lines[rows - 1], 0, cols + 1);
}

static void
term_print_string(const char *str)
{
    int cols = term_cur_cols();
    int rows = term_cur_rows();
    while (*str) {
        if (*str == '\n') {
            g_term_cursor_col = 0;
            if (g_term_cursor_row < rows - 1) {
                g_term_cursor_row++;
            } else {
                term_scroll_up();
            }
        } else if (*str == '\r') {
            g_term_cursor_col = 0;
        } else {
            if (g_term_cursor_col >= cols) {
                g_term_cursor_col = 0;
                if (g_term_cursor_row < rows - 1) {
                    g_term_cursor_row++;
                } else {
                    term_scroll_up();
                }
            }
            g_term_lines[g_term_cursor_row][g_term_cursor_col++] = *str;
            g_term_lines[g_term_cursor_row][g_term_cursor_col] = '\0';
        }
        str++;
    }
    g_term_redraw_needed = 1;
}

static void
term_init(void)
{
    if (g_term_inited) return;
    for (int r = 0; r < TERM_MAX_ROWS; r++) {
        memset(g_term_lines[r], 0, TERM_MAX_COLS + 1);
    }
    g_term_cursor_row = 0;
    g_term_cursor_col = 0;
    getcwd(g_term_cwd, sizeof(g_term_cwd));
    if (g_term_cwd[0] == '\0') strcpy(g_term_cwd, "/var/root");
    term_print_string("-sh-3.2# ");
    g_term_cmd_len = 0;
    g_term_cmd[0] = '\0';
    g_term_inited = 1;
}

static void
term_exec_command(const char *cmd_line)
{
    while (*cmd_line == ' ') cmd_line++;
    if (*cmd_line == '\0') {
        term_print_string("\n-sh-3.2# ");
        return;
    }

    char line_copy[256];
    strncpy(line_copy, cmd_line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    char *argv[32];
    int argc = 0;
    char *token = strtok(line_copy, " \t\r\n");
    while (token && argc < 31) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    argv[argc] = NULL;
    if (argc == 0) {
        term_print_string("\n-sh-3.2# ");
        return;
    }

    const char *cmd = argv[0];

    /* 1. Try fork + execve external Darwin binary if on disk */
    int pfd[2];
    if (pipe(pfd) == 0) {
        pid_t pid = fork();
        if (pid == 0) {
            close(pfd[0]);
            dup2(pfd[1], 1);
            dup2(pfd[1], 2);
            close(pfd[1]);

            char bin_path[128];
            if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/')) {
                execv(cmd, argv);
            } else {
                snprintf(bin_path, sizeof(bin_path), "/bin/%s", cmd);
                execv(bin_path, argv);
                snprintf(bin_path, sizeof(bin_path), "/usr/bin/%s", cmd);
                execv(bin_path, argv);
                snprintf(bin_path, sizeof(bin_path), "/sbin/%s", cmd);
                execv(bin_path, argv);
            }
            _exit(127);
        }
        close(pfd[1]);
        if (pid > 0) {
            char buf[512];
            ssize_t n;
            int ran = 0;
            int status = 0;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 127) {
                ran = 1;
                term_print_string("\n");
                while ((n = read(pfd[0], buf, sizeof(buf) - 1)) > 0) {
                    buf[n] = '\0';
                    term_print_string(buf);
                }
            }
            close(pfd[0]);
            if (ran) {
                term_print_string("-sh-3.2# ");
                return;
            }
        } else {
            close(pfd[0]);
        }
    }

    /* 2. Authentic Darwin built-in utilities */
    if (strcmp(cmd, "clear") == 0) {
        int rows = term_cur_rows();
        int cols = term_cur_cols();
        for (int r = 0; r < rows; r++) {
            memset(g_term_lines[r], 0, cols + 1);
        }
        g_term_cursor_row = 0;
        g_term_cursor_col = 0;
        term_print_string("-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "cd") == 0) {
        const char *target = argc > 1 ? argv[1] : "/";
        if (chdir(target) == 0) {
            getcwd(g_term_cwd, sizeof(g_term_cwd));
        } else {
            term_print_string("\n-sh: cd: ");
            term_print_string(target);
            term_print_string(": No such file or directory");
        }
        term_print_string("\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "pwd") == 0) {
        term_print_string("\n");
        term_print_string(g_term_cwd);
        term_print_string("\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "echo") == 0) {
        term_print_string("\n");
        for (int i = 1; i < argc; i++) {
            if (i > 1) term_print_string(" ");
            term_print_string(argv[i]);
        }
        term_print_string("\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "whoami") == 0) {
        term_print_string("\nroot\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "id") == 0) {
        term_print_string("\nuid=0(root) gid=0(wheel) groups=0(wheel),1(daemon),2(kmem),3(sys),4(tty)\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "date") == 0) {
        time_t now = time(NULL);
        char date_buf[64];
        ctime_r(&now, date_buf);
        term_print_string("\n");
        term_print_string(date_buf);
        term_print_string("-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "uname") == 0) {
        if (argc > 1 && strcmp(argv[1], "-a") == 0) {
            term_print_string("\nDarwin darwinOS 26.0.0 Darwin Kernel Version 26.0.0: root:xnu-8792.81.2/RELEASE_X86_64 x86_64\n-sh-3.2# ");
        } else if (argc > 1 && strcmp(argv[1], "-r") == 0) {
            term_print_string("\n26.0.0\n-sh-3.2# ");
        } else if (argc > 1 && strcmp(argv[1], "-m") == 0) {
            term_print_string("\nx86_64\n-sh-3.2# ");
        } else {
            term_print_string("\nDarwin\n-sh-3.2# ");
        }
        return;
    } else if (strcmp(cmd, "sw_vers") == 0) {
        term_print_string("\nProductName:\tmacOS\nProductVersion:\t26.0\nBuildVersion:\t26A100\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "csrutil") == 0) {
        term_print_string("\nSystem Integrity Protection status: disabled (Apple Internal).\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "diskutil") == 0) {
        if (argc > 1 && strcmp(argv[1], "list") == 0) {
            term_print_string("\n/dev/disk0 (internal, physical):\n"
                              "   #:                       TYPE NAME                    SIZE       IDENTIFIER\n"
                              "   0:      GUID_partition_scheme                        *8.0 GB     disk0\n"
                              "   1:                        EFI EFI                     512.0 MB   disk0s1\n"
                              "   2:                  Apple_HFS darwinOS Base System    7.5 GB     disk0s2\n-sh-3.2# ");
        } else {
            term_print_string("\nUsage: diskutil [list|info|mount|unmount|eraseDisk]\n-sh-3.2# ");
        }
        return;
    } else if (strcmp(cmd, "nvram") == 0) {
        term_print_string("\nboot-args\tdebug=0x0C serial=0 iotrace=0 io=0 slto_us=60000000 mtlinit=/mtl_app\n"
                          "efi-boot-device\t<array><dict><key>IOMatch</key><dict><key>IOProviderClass</key><string>IOPCIDevice</string></dict></dict></array>\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "ps") == 0) {
        term_print_string("\n  PID TTY           TIME CMD\n"
                          "    1 ??         0:02.14 /sbin/launchd\n"
                          "    2 ??         0:00.86 /Applications/Install darwinOS.app/Contents/MacOS/Install darwinOS\n"
                          "    3 ??         0:05.42 /System/Library/CoreServices/WindowServer.app/Contents/MacOS/WindowServer\n"
                          "   10 ttys000    0:00.04 -sh\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "df") == 0) {
        term_print_string("\nFilesystem     512-blocks      Used Available Capacity iused      ifree %iused  Mounted on\n"
                          "/dev/disk0s2     15724544   1258240  14466304     8%  157248    1808288    8%   /\n"
                          "devfs                 384       384         0   100%     666          0  100%   /dev\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "ls") == 0) {
        const char *target = argc > 1 && argv[1][0] != '-' ? argv[1] : g_term_cwd;
        DIR *dir = opendir(target);
        if (dir) {
            term_print_string("\n");
            struct dirent *de;
            int count = 0;
            while ((de = readdir(dir)) != NULL) {
                if (de->d_name[0] == '.' && (argc <= 1 || strstr(argv[1], "a") == NULL))
                    continue;
                if (count > 0 && count % 5 == 0) term_print_string("\n");
                else if (count > 0) term_print_string("  ");
                term_print_string(de->d_name);
                count++;
            }
            closedir(dir);
            term_print_string("\n-sh-3.2# ");
        } else {
            term_print_string("\nApplications  Library  System  Users  bin  etc  sbin  tmp  usr  var\n-sh-3.2# ");
        }
        return;
    } else if (strcmp(cmd, "cat") == 0) {
        if (argc < 2) {
            term_print_string("\nUsage: cat <file>\n-sh-3.2# ");
            return;
        }
        int fd = open(argv[1], O_RDONLY);
        if (fd >= 0) {
            term_print_string("\n");
            char buf[256];
            ssize_t n;
            while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0';
                term_print_string(buf);
            }
            close(fd);
            term_print_string("\n-sh-3.2# ");
        } else {
            term_print_string("\ncat: ");
            term_print_string(argv[1]);
            term_print_string(": No such file or directory\n-sh-3.2# ");
        }
        return;
    } else if (strcmp(cmd, "mkdir") == 0) {
        if (argc < 2) {
            term_print_string("\nUsage: mkdir <directory>\n-sh-3.2# ");
            return;
        }
        if (mkdir(argv[1], 0755) != 0) {
            term_print_string("\nmkdir: cannot create directory '");
            term_print_string(argv[1]);
            term_print_string("'\n-sh-3.2# ");
        } else {
            term_print_string("\n-sh-3.2# ");
        }
        return;
    } else if (strcmp(cmd, "rm") == 0) {
        if (argc < 2) {
            term_print_string("\nUsage: rm <file>\n-sh-3.2# ");
            return;
        }
        if (unlink(argv[1]) != 0) {
            term_print_string("\nrm: ");
            term_print_string(argv[1]);
            term_print_string(": No such file or directory\n-sh-3.2# ");
        } else {
            term_print_string("\n-sh-3.2# ");
        }
        return;
    } else if (strcmp(cmd, "touch") == 0) {
        if (argc < 2) {
            term_print_string("\nUsage: touch <file>\n-sh-3.2# ");
            return;
        }
        int fd = open(argv[1], O_CREAT | O_WRONLY, 0644);
        if (fd >= 0) close(fd);
        term_print_string("\n-sh-3.2# ");
        return;
    } else if (strcmp(cmd, "reboot") == 0 || strcmp(cmd, "restart") == 0) {
        term_print_string("\nRebooting system...\n");
        sync();
        reboot(0);
        return;
    } else if (strcmp(cmd, "halt") == 0 || strcmp(cmd, "shutdown") == 0) {
        term_print_string("\nHalting system...\n");
        sync();
        reboot(0x08);
        return;
    } else if (strcmp(cmd, "exit") == 0) {
        g_show_terminal = 0;
        g_dirty = 1;
        return;
    } else if (strcmp(cmd, "help") == 0) {
        term_print_string("\nDarwin macOS Recovery Terminal (sh-3.2)\n"
                          "Standard commands supported:\n"
                          "  ls, pwd, cd, cat, echo, mkdir, rm, touch\n"
                          "  uname, sw_vers, diskutil, csrutil, nvram\n"
                          "  whoami, id, date, ps, df, clear\n"
                          "  reboot, halt, exit\n-sh-3.2# ");
        return;
    } else {
        term_print_string("\n-sh: ");
        term_print_string(cmd);
        term_print_string(": command not found\n-sh-3.2# ");
        return;
    }
}

static void
term_handle_char(char c)
{
    int cols = term_cur_cols();
    if (c == '\n' || c == '\r') {
        g_term_cmd[g_term_cmd_len] = '\0';
        char buf[256];
        strncpy(buf, g_term_cmd, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        g_term_cmd_len = 0;
        g_term_cmd[0] = '\0';
        term_exec_command(buf);
    } else if (c == '\b' || c == 127) {
        if (g_term_cmd_len > 0) {
            g_term_cmd[--g_term_cmd_len] = '\0';
            if (g_term_cursor_col > 0) {
                g_term_lines[g_term_cursor_row][--g_term_cursor_col] = '\0';
                g_term_redraw_needed = 1;
            }
        }
    } else if (c >= 32 && c <= 126) {
        if (g_term_cmd_len < (int)sizeof(g_term_cmd) - 2 && g_term_cursor_col < cols - 1) {
            g_term_cmd[g_term_cmd_len++] = c;
            g_term_cmd[g_term_cmd_len] = '\0';
            g_term_lines[g_term_cursor_row][g_term_cursor_col++] = c;
            g_term_lines[g_term_cursor_row][g_term_cursor_col] = '\0';
            g_term_redraw_needed = 1;
        }
    }
}

static void
term_render_glyph(uint32_t *dst_surface, int dst_w, int dst_h, int x0, int y0, char c, uint32_t text_color)
{
    if (c < 32 || c > 126) return;
    int glyph_idx = c - 32;
    const uint8_t *glyph_col_start = g_sfmono_atlas + glyph_idx * TERM_CHAR_W;

    unsigned tr = (text_color >> 16) & 0xffu;
    unsigned tg = (text_color >> 8) & 0xffu;
    unsigned tb = text_color & 0xffu;

    for (int gy = 0; gy < TERM_CHAR_H; gy++) {
        int py = y0 + gy;
        if (py < 0 || py >= dst_h) continue;
        const uint8_t *src_row = glyph_col_start + gy * (95 * TERM_CHAR_W);
        uint32_t *dst_row = dst_surface + (size_t)py * (size_t)dst_w;

        for (int gx = 0; gx < TERM_CHAR_W; gx++) {
            int px = x0 + gx;
            if (px < 0 || px >= dst_w) continue;
            unsigned a = src_row[gx];
            if (a == 0) continue;

            if (a == 255) {
                dst_row[px] = (0xff000000u | (tr << 16) | (tg << 8) | tb);
            } else {
                uint32_t d = dst_row[px];
                unsigned dr = (d >> 16) & 0xffu;
                unsigned dg = (d >> 8) & 0xffu;
                unsigned db = d & 0xffu;
                unsigned inv = 255u - a;
                unsigned r = (tr * a + dr * inv + 127u) / 255u;
                unsigned g = (tg * a + dg * inv + 127u) / 255u;
                unsigned b = (tb * a + db * inv + 127u) / 255u;
                dst_row[px] = (0xff000000u | (r << 16) | (g << 8) | b);
            }
        }
    }
}

static void
term_render_title_char(uint32_t *dst_surface, int dst_w, int dst_h, int x0, int y0, char c, uint32_t text_color)
{
    const char *chars = "-sh 0123456789x";
    const char *p = strchr(chars, c);
    if (!p) return;
    int idx = (int)(p - chars);
    const uint8_t *col_start = g_title_atlas + idx * TERM_TITLE_W;

    unsigned tr = (text_color >> 16) & 0xffu;
    unsigned tg = (text_color >> 8) & 0xffu;
    unsigned tb = text_color & 0xffu;

    for (int gy = 0; gy < TERM_TITLE_H; gy++) {
        int py = y0 + gy;
        if (py < 0 || py >= dst_h) continue;
        const uint8_t *src_row = col_start + gy * (15 * TERM_TITLE_W);
        uint32_t *dst_row = dst_surface + (size_t)py * (size_t)dst_w;

        for (int gx = 0; gx < TERM_TITLE_W; gx++) {
            int px = x0 + gx;
            if (px < 0 || px >= dst_w) continue;
            unsigned a = src_row[gx];
            if (a == 0) continue;

            uint32_t d = dst_row[px];
            unsigned dr = (d >> 16) & 0xffu;
            unsigned dg = (d >> 8) & 0xffu;
            unsigned db = d & 0xffu;
            unsigned inv = 255u - a;
            unsigned r = (tr * a + dr * inv + 127u) / 255u;
            unsigned g = (tg * a + dg * inv + 127u) / 255u;
            unsigned b = (tb * a + db * inv + 127u) / 255u;
            dst_row[px] = (0xff000000u | (r << 16) | (g << 8) | b);
        }
    }
}

static inline void
draw_terminal_window(void)
{
    if (!g_show_terminal || !g_fb) return;
    term_init();

    int cur_w = g_term_body_w + TERM_PAD * 2;
    int cur_h = g_term_body_h + TERM_PAD * 2;
    if (cur_w > TERM_MAX_W) cur_w = TERM_MAX_W;
    if (cur_h > TERM_MAX_H) cur_h = TERM_MAX_H;

    int cols = term_cur_cols();
    int rows = term_cur_rows();

    if (g_term_redraw_needed) {
        /* 9-Slice Dynamic Rendering into g_term_window_pixels */
        int sc = TERM_SLICE_C; /* 76 */

        /* 1. Corners */
        for (int y = 0; y < sc; y++) {
            memcpy(g_term_window_pixels + (size_t)y * cur_w,
                   g_term_slice_tl + (size_t)y * sc, sc * 4);
            memcpy(g_term_window_pixels + (size_t)y * cur_w + (cur_w - sc),
                   g_term_slice_tr + (size_t)y * sc, sc * 4);
            memcpy(g_term_window_pixels + (size_t)(cur_h - sc + y) * cur_w,
                   g_term_slice_bl + (size_t)y * sc, sc * 4);
            memcpy(g_term_window_pixels + (size_t)(cur_h - sc + y) * cur_w + (cur_w - sc),
                   g_term_slice_br + (size_t)y * sc, sc * 4);
        }

        /* 2. Top & Bottom Edges */
        for (int x = sc; x < cur_w - sc; x++) {
            for (int y = 0; y < sc; y++) {
                g_term_window_pixels[(size_t)y * cur_w + x] = g_term_slice_top[y];
                g_term_window_pixels[(size_t)(cur_h - sc + y) * cur_w + x] = g_term_slice_bot[y];
            }
        }

        /* 3. Left & Right Edges */
        for (int y = sc; y < cur_h - sc; y++) {
            uint32_t *row_l = g_term_window_pixels + (size_t)y * cur_w;
            uint32_t *row_r = row_l + (cur_w - sc);
            for (int x = 0; x < sc; x++) {
                row_l[x] = g_term_slice_left[x];
                row_r[x] = g_term_slice_right[x];
            }
        }

        /* 4. Center Body Fill */
        int body_x0 = TERM_PAD;
        int body_x1 = TERM_PAD + g_term_body_w;
        int body_y0 = TERM_PAD + 28;
        int body_y1 = TERM_PAD + g_term_body_h;

        for (int y = body_y0; y < body_y1; y++) {
            uint32_t *row = g_term_window_pixels + (size_t)y * cur_w;
            for (int x = body_x0; x < body_x1; x++) {
                row[x] = 0xff1e1e1eu;
            }
        }

        /* Titlebar horizontal filler */
        for (int y = TERM_PAD; y < body_y0; y++) {
            uint32_t *row = g_term_window_pixels + (size_t)y * cur_w;
            uint32_t val = (y == body_y0 - 1) ? 0xff1a1a1au : 0xff2a2a2au;
            for (int x = sc; x < cur_w - sc; x++) {
                row[x] = val;
            }
        }

        /* 5. Titlebar Center: Proxy Folder Icon + Title */
        char titlestr[32];
        snprintf(titlestr, sizeof(titlestr), "-sh - %dx%d", cols, rows);
        int tlen = strlen(titlestr);
        int total_tw = TERM_FOLDER_W + 6 + tlen * TERM_TITLE_W;
        int start_tx = TERM_PAD + (g_term_body_w - total_tw) / 2;
        int start_ty = TERM_PAD + (28 - TERM_FOLDER_H) / 2;

        /* Blit Folder Icon */
        for (int fy = 0; fy < TERM_FOLDER_H; fy++) {
            uint32_t *row = g_term_window_pixels + (size_t)(start_ty + fy) * cur_w + start_tx;
            const uint32_t *src_row = g_term_folder_icon + (size_t)fy * TERM_FOLDER_W;
            for (int fx = 0; fx < TERM_FOLDER_W; fx++) {
                row[fx] = blend_pixel(row[fx], src_row[fx]);
            }
        }

        /* Blit Title String in SF Pro Semibold */
        int text_tx = start_tx + TERM_FOLDER_W + 6;
        int text_ty = TERM_PAD + (28 - TERM_TITLE_H) / 2;
        for (int ti = 0; ti < tlen; ti++) {
            term_render_title_char(g_term_window_pixels, cur_w, cur_h,
                                   text_tx + ti * TERM_TITLE_W, text_ty,
                                   titlestr[ti], 0x00d8d8d8u);
        }

        /* 6. Subtle Right-Side Scrollbar Thumb */
        int sb_x = TERM_PAD + g_term_body_w - 6;
        int sb_y0 = TERM_PAD + 38;
        int sb_h = 44;
        for (int dy = 0; dy < sb_h; dy++) {
            uint32_t *row = g_term_window_pixels + (size_t)(sb_y0 + dy) * cur_w;
            for (int dx = 0; dx < 4; dx++) {
                row[sb_x + dx] = 0xff383838u;
            }
        }

        /* 7. Text Lines & Cursor */
        for (int r = 0; r < rows; r++) {
            int y = TERM_PAD + TERM_MARGIN_TOP + r * TERM_CHAR_H;
            const char *line = g_term_lines[r];
            int len = strlen(line);
            for (int c = 0; c < len && c < cols; c++) {
                int x = TERM_PAD + TERM_MARGIN_LEFT + c * TERM_CHAR_W;
                term_render_glyph(g_term_window_pixels, cur_w, cur_h, x, y, line[c], 0x00ECECECu);
            }
        }

        /* Render block cursor */
        int cur_x = TERM_PAD + TERM_MARGIN_LEFT + g_term_cursor_col * TERM_CHAR_W;
        int cur_y = TERM_PAD + TERM_MARGIN_TOP + g_term_cursor_row * TERM_CHAR_H;
        for (int dy = 1; dy < TERM_CHAR_H - 1; dy++) {
            int py = cur_y + dy;
            if (py >= 0 && py < cur_h) {
                uint32_t *row = g_term_window_pixels + (size_t)py * cur_w;
                for (int dx = 0; dx < TERM_CHAR_W; dx++) {
                    int px = cur_x + dx;
                    if (px >= 0 && px < cur_w) {
                        row[px] = 0xffc0c0c0u;
                    }
                }
            }
        }
        g_term_redraw_needed = 0;
    }

    /* Blit into Framebuffer */
    int tx = (int)g_term_x;
    int ty = (int)g_term_y;
    for (int dy = 0; dy < cur_h && (ty + dy) < g_h; dy++) {
        if (ty + dy < 0) continue;
        uint32_t *dst = g_fb + (size_t)(ty + dy) * (size_t)g_w + tx;
        const uint32_t *src = g_term_window_pixels + (size_t)dy * (size_t)cur_w;
        for (int dx = 0; dx < cur_w; dx++) {
            if (tx + dx < 0 || tx + dx >= g_w) continue;
            dst[dx] = blend_pixel(dst[dx], src[dx]);
        }
    }
}
static inline void
draw_about_window(void)
{
    if (!g_show_about || !g_fb) return;
    int ax = (int)g_about_x;
    int ay = (int)g_about_y;
    for (int dy = 0; dy < ABOUT_WINDOW_H && (ay + dy) < g_h; dy++) {
        if (ay + dy < 0) continue;
        uint32_t *dst = g_fb + (size_t)(ay + dy) * (size_t)g_w + ax;
        const uint32_t *src = g_about_window_pixels + (size_t)dy * (size_t)ABOUT_WINDOW_W;
        for (int dx = 0; dx < ABOUT_WINDOW_W; dx++) {
            if (ax + dx < 0 || ax + dx >= g_w) continue;
            dst[dx] = blend_pixel(dst[dx], src[dx]);
        }
    }
}

static inline void
draw_menubar(void)
{
    if (!g_fb || g_w < MENUBAR_W) return;
    const uint32_t *mb_src;
    int is_term_menu = (g_show_terminal && g_term_focused);

    if (is_term_menu) {
        if (g_active_menu == 10) mb_src = g_menubar_term_apple;
        else if (g_active_menu == 0) mb_src = g_menubar_term_terminal;
        else if (g_active_menu == 1) mb_src = g_menubar_term_shell;
        else if (g_active_menu == 2) mb_src = g_menubar_term_edit;
        else if (g_active_menu == 3) mb_src = g_menubar_term_view;
        else if (g_active_menu == 4) mb_src = g_menubar_term_window;
        else if (g_active_menu == 5) mb_src = g_menubar_term_help;
        else mb_src = g_menubar_term_normal;
    } else {
        if (g_active_menu == 10) mb_src = g_menubar_apple;
        else if (g_active_menu == 0) mb_src = g_menubar_recovery;
        else if (g_active_menu == 1) mb_src = g_menubar_file;
        else if (g_active_menu == 2) mb_src = g_menubar_edit;
        else if (g_active_menu == 3) mb_src = g_menubar_utilities;
        else if (g_active_menu == 4) mb_src = g_menubar_window;
        else if (g_active_menu == 5) mb_src = g_menubar_help;
        else mb_src = g_menubar_pixels;
    }

    for (int y = 0; y < MENUBAR_H && y < g_h; y++) {
        memcpy(g_fb + (size_t)y * (size_t)g_w, mb_src + (size_t)y * (size_t)MENUBAR_W, (size_t)MENUBAR_W * 4);
    }

    const uint32_t *drop_pixels = NULL;
    int drop_x = 0, drop_y = 0, drop_w = 0, drop_h = 0;

    if (g_active_menu == 10) {
        drop_x = DROPDOWN_APPLE_X; drop_y = DROPDOWN_APPLE_Y;
        drop_w = DROPDOWN_APPLE_W; drop_h = DROPDOWN_APPLE_H;
        int st_idx = 0;
        if (g_hovered_row == 0) st_idx = 1;
        else if (g_hovered_row == 2) st_idx = 2;
        else if (g_hovered_row == 3) st_idx = 3;
        drop_pixels = g_dropdown_apple_states[st_idx];
    } else if (is_term_menu) {
        if (g_active_menu == 0) {
            drop_x = DROPDOWN_TERM_APP_X; drop_y = DROPDOWN_TERM_APP_Y;
            drop_w = DROPDOWN_TERM_APP_W; drop_h = DROPDOWN_TERM_APP_H;
            int st_idx = 0;
            if (g_hovered_row == 0) st_idx = 1;
            else if (g_hovered_row == 2) st_idx = 2;
            else if (g_hovered_row == 4) st_idx = 3;
            else if (g_hovered_row == 6) st_idx = 4;
            drop_pixels = g_dropdown_term_app_states[st_idx];
        } else if (g_active_menu == 1) {
            drop_x = DROPDOWN_TERM_SHELL_X; drop_y = DROPDOWN_TERM_SHELL_Y;
            drop_w = DROPDOWN_TERM_SHELL_W; drop_h = DROPDOWN_TERM_SHELL_H;
            int st_idx = 0;
            if (g_hovered_row == 0) st_idx = 1;
            else if (g_hovered_row == 1) st_idx = 2;
            drop_pixels = g_dropdown_term_shell_states[st_idx];
        } else if (g_active_menu == 2) {
            drop_x = DROPDOWN_TERM_EDIT_X; drop_y = DROPDOWN_TERM_EDIT_Y;
            drop_w = DROPDOWN_TERM_EDIT_W; drop_h = DROPDOWN_TERM_EDIT_H;
            int st_idx = (g_hovered_row >= 0 && g_hovered_row < 3) ? (g_hovered_row + 1) : 0;
            drop_pixels = g_dropdown_term_edit_states[st_idx];
        } else if (g_active_menu == 3) {
            drop_x = DROPDOWN_TERM_VIEW_X; drop_y = DROPDOWN_TERM_VIEW_Y;
            drop_w = DROPDOWN_TERM_VIEW_W; drop_h = DROPDOWN_TERM_VIEW_H;
            int st_idx = (g_hovered_row >= 0 && g_hovered_row < 2) ? (g_hovered_row + 1) : 0;
            drop_pixels = g_dropdown_term_view_states[st_idx];
        } else if (g_active_menu == 4) {
            drop_x = DROPDOWN_TERM_WINDOW_X; drop_y = DROPDOWN_TERM_WINDOW_Y;
            drop_w = DROPDOWN_TERM_WINDOW_W; drop_h = DROPDOWN_TERM_WINDOW_H;
            int st_idx = 0;
            if (g_hovered_row == 0) st_idx = 1;
            else if (g_hovered_row == 1) st_idx = 2;
            else if (g_hovered_row == 3) st_idx = 3;
            drop_pixels = g_dropdown_term_window_states[st_idx];
        } else if (g_active_menu == 5) {
            drop_x = DROPDOWN_TERM_HELP_X; drop_y = DROPDOWN_TERM_HELP_Y;
            drop_w = DROPDOWN_TERM_HELP_W; drop_h = DROPDOWN_TERM_HELP_H;
            int st_idx = (g_hovered_row == 0) ? 1 : 0;
            drop_pixels = g_dropdown_term_help_states[st_idx];
        }
    } else {
        if (g_active_menu == 0) {
            drop_x = DROPDOWN_RECOVERY_X; drop_y = DROPDOWN_RECOVERY_Y;
            drop_w = DROPDOWN_RECOVERY_W; drop_h = DROPDOWN_RECOVERY_H;
            int st_idx = (g_hovered_row == 0) ? 1 : 0;
            drop_pixels = g_dropdown_recovery_states[st_idx];
        } else if (g_active_menu == 1) {
            drop_x = DROPDOWN_FILE_X; drop_y = DROPDOWN_FILE_Y;
            drop_w = DROPDOWN_FILE_W; drop_h = DROPDOWN_FILE_H;
            int st_idx = (g_hovered_row == 0) ? 1 : 0;
            drop_pixels = g_dropdown_file_states[st_idx];
        } else if (g_active_menu == 2) {
            drop_x = DROPDOWN_EDIT_X; drop_y = DROPDOWN_EDIT_Y;
            drop_w = DROPDOWN_EDIT_W; drop_h = DROPDOWN_EDIT_H;
            drop_pixels = g_dropdown_edit_states[0];
        } else if (g_active_menu == 3) {
            drop_x = DROPDOWN_UTILITIES_X; drop_y = DROPDOWN_UTILITIES_Y;
            drop_w = DROPDOWN_UTILITIES_W; drop_h = DROPDOWN_UTILITIES_H;
            int st_idx = (g_hovered_row >= 0 && g_hovered_row < 4) ? (g_hovered_row + 1) : 0;
            drop_pixels = g_dropdown_utilities_states[st_idx];
        } else if (g_active_menu == 4) {
            drop_x = DROPDOWN_WINDOW_X; drop_y = DROPDOWN_WINDOW_Y;
            drop_w = DROPDOWN_WINDOW_W; drop_h = DROPDOWN_WINDOW_H;
            int st_idx = (g_hovered_row == 3) ? 1 : 0;
            drop_pixels = g_dropdown_window_states[st_idx];
        } else if (g_active_menu == 5) {
            drop_x = DROPDOWN_HELP_X; drop_y = DROPDOWN_HELP_Y;
            drop_w = DROPDOWN_HELP_W; drop_h = DROPDOWN_HELP_H;
            int st_idx = (g_hovered_row == 0) ? 1 : 0;
            drop_pixels = g_dropdown_help_states[st_idx];
        }
    }

    if (drop_pixels) {
        for (int dy = 0; dy < drop_h && (drop_y + dy) < g_h; dy++) {
            uint32_t *dst = g_fb + (size_t)(drop_y + dy) * (size_t)g_w + drop_x;
            const uint32_t *src = drop_pixels + (size_t)dy * (size_t)drop_w;
            for (int dx = 0; dx < drop_w; dx++) {
                dst[dx] = blend_pixel(dst[dx], src[dx]);
            }
        }
    }
}

static uint32_t g_bg_gradient[1280 * 800];
static int g_bg_gradient_ready = 0;

static void
init_bg_gradient(void)
{
    if (g_bg_gradient_ready) return;
    float xc = (float)g_w * 0.5f;
    float yc = (float)g_h * 0.5f;
    for (int y = 0; y < g_h && y < 800; y++) {
        float dy = ((float)y - yc) / yc;
        for (int x = 0; x < g_w && x < 1280; x++) {
            float dx = ((float)x - xc) / xc;
            float r2 = dx * dx + dy * dy;
            float t = sqrtf(r2) * 0.75f;
            if (t > 1.0f) t = 1.0f;
            /* Authentic macOS Recovery vignette: rich Apple dark grey wallpaper */
            int v = (int)roundf(38.0f - 16.0f * t);
            if (v < 20) v = 20;
            g_bg_gradient[y * g_w + x] = 0xff000000u | ((uint32_t)v << 16) | ((uint32_t)v << 8) | (uint32_t)v;
        }
    }
    g_bg_gradient_ready = 1;
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

    init_bg_gradient();
    int copy_bytes = (x1 - x0) * (int)sizeof(uint32_t);
    for (int y = y0; y < y1; y++) {
        memcpy(g_fb + (size_t)y * (size_t)g_w + (size_t)x0,
               g_bg_gradient + (size_t)y * (size_t)g_w + (size_t)x0,
               copy_bytes);
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

#define BLEND_SHADOW_PX(dst_ptr, sa) do { \
    uint32_t d_col = *(dst_ptr); \
    unsigned inv = 255u - (sa); \
    unsigned r = (((d_col >> 16) & 0xffu) * inv + 127u) / 255u; \
    unsigned g = (((d_col >> 8) & 0xffu) * inv + 127u) / 255u; \
    unsigned b = ((d_col & 0xffu) * inv + 127u) / 255u; \
    *(dst_ptr) = 0xff000000u | (r << 16) | (g << 8) | b; \
} while (0)

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
    const int R = 16;       /* window corner radius */

    int win_x0 = wx;
    int win_y0 = wy;
    int win_x1 = wx + ww;
    int win_y1 = wy + wh;

    int sx0 = wx - M;
    int sy0 = wy - M + dy_off;
    int sx1 = wx + ww + M;
    int sy1 = wy + wh + M + dy_off;

    if (sc_w > 0 && sc_h > 0) {
        if (sx0 < sc_x) sx0 = sc_x;
        if (sy0 < sc_y) sy0 = sc_y;
        if (sx1 > sc_x + sc_w) sx1 = sc_x + sc_w;
        if (sy1 > sc_y + sc_h) sy1 = sc_y + sc_h;
    }

    if (sx0 < 0) sx0 = 0;
    if (sy0 < 0) sy0 = 0;
    if (sx1 > g_w) sx1 = g_w;
    if (sy1 > g_h) sy1 = g_h;

    if (sx0 >= sx1 || sy0 >= sy1) return;

    int corner_lx = wx + R;
    int corner_rx = wx + ww - R;
    int corner_ty = wy + R;
    int corner_by = wy + wh - R;

    for (int py = sy0; py < sy1; py++) {
        uint32_t *dst_row = g_fb + (size_t)py * (size_t)g_w;
        int in_win_y = (py >= win_y0 && py < win_y1);
        int top_half = (py < corner_ty);
        int bot_half = (py >= corner_by);

        int dy_edge = 0;
        if (py < win_y0) {
            dy_edge = (win_y0 - py);
        } else if (py >= win_y1) {
            dy_edge = (py - win_y1 + 1);
        }

        /* Fast skip window interior for mid-body scanlines */
        if (in_win_y && !top_half && !bot_half) {
            int lx1 = (win_x0 < sx1) ? win_x0 : sx1;
            for (int px = sx0; px < lx1; px++) {
                int d = win_x0 - px;
                if (d >= 0 && d < 16) {
                    uint8_t sa = g_shadow_lut[d];
                    if (sa) BLEND_SHADOW_PX(&dst_row[px], sa);
                }
            }
            int rx0 = (win_x1 > sx0) ? win_x1 : sx0;
            for (int px = rx0; px < sx1; px++) {
                int d = px - win_x1 + 1;
                if (d >= 0 && d < 16) {
                    uint8_t sa = g_shadow_lut[d];
                    if (sa) BLEND_SHADOW_PX(&dst_row[px], sa);
                }
            }
            continue;
        }

        /* Top, bottom, or corner scanlines */
        for (int px = sx0; px < sx1; px++) {
            if (in_win_y && px >= corner_lx && px < corner_rx) {
                px = corner_rx - 1;
                continue;
            }

            if (in_win_y && px >= win_x0 && px < win_x1) {
                int lcol = (px < corner_lx);
                int cx = lcol ? corner_lx : corner_rx;
                int cy = top_half ? corner_ty : corner_by;
                int cdx = px - cx;
                int cdy = py - cy;
                if (cdx * cdx + cdy * cdy <= R * R) {
                    if (lcol) px = corner_lx - 1;
                    continue;
                }
            }

            int dx_edge = 0;
            if (px < win_x0) {
                dx_edge = win_x0 - px;
            } else if (px >= win_x1) {
                dx_edge = px - win_x1 + 1;
            }

            uint8_t sa = 0;
            if (dx_edge == 0 && dy_edge > 0) {
                if (dy_edge < 16) sa = g_shadow_lut[dy_edge];
            } else if (dy_edge == 0 && dx_edge > 0) {
                if (dx_edge < 16) sa = g_shadow_lut[dx_edge];
            } else {
                int lcol = (px < corner_lx);
                int cx = lcol ? corner_lx : corner_rx;
                int cy = top_half ? corner_ty : corner_by;
                int qx = abs(px - cx);
                int qy = abs(py - cy);
                if (qx > 33) qx = 33;
                if (qy > 33) qy = 33;
                sa = g_corner_lut[qy][qx];
            }

            if (sa) {
                BLEND_SHADOW_PX(&dst_row[px], sa);
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
        /* Fallback CPU desktop clear with authentic radial vignette */
        fill_rect_bg(0, 0, g_w, g_h);
        for (l = 0; l < kCGNumReservedWindowLevels; l++) {
            WSWindow *w;
            for (w = g_levels[l]; w; w = w->next) {
                if (win_visible(w))
                    composite_window(w);
            }
        }
        draw_terminal_window();
        draw_about_window();
        draw_menubar();
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
            wrpd.clear_color = mtl_make_float4(32.0f / 255.0f, 32.0f / 255.0f, 34.0f / 255.0f, 1.0f);
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

    draw_terminal_window();
    draw_about_window();
    draw_menubar();

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

static int g_shift_down = 0;
static const char g_scancode_normal[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0', [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p', [0x1A] = '[', [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l', [0x27] = ';', [0x28] = '\'', [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm', [0x33] = ',',
    [0x34] = '.', [0x35] = '/',
    [0x39] = ' '
};

static const char g_scancode_shift[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$',
    [0x06] = '%', [0x07] = '^', [0x08] = '&', [0x09] = '*',
    [0x0A] = '(', [0x0B] = ')', [0x0C] = '_', [0x0D] = '+',
    [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R',
    [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P', [0x1A] = '{', [0x1B] = '}',
    [0x1C] = '\n',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F',
    [0x22] = 'G', [0x23] = 'H', [0x24] = 'J', [0x25] = 'K',
    [0x26] = 'L', [0x27] = ':', [0x28] = '"', [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V',
    [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<',
    [0x34] = '>', [0x35] = '?',
    [0x39] = ' '
};

struct ws2_mouse_state {
    int mouse_x;
    int mouse_y;
    int buttons;
    int has_moved;
    int key_scancode;
    int key_count;
    uint8_t key_buf[16];
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

    /* Process all hardware keyboard scancodes immediately at top of poll */
    if (ms.key_count > 0 || ms.key_scancode != 0) {
        int kcnt = ms.key_count > 0 ? ms.key_count : 1;
        for (int ki = 0; ki < kcnt; ki++) {
            uint8_t sc = ms.key_count > 0 ? ms.key_buf[ki] : (uint8_t)(ms.key_scancode & 0xff);
            if (sc == 0x2A || sc == 0x36) {
                g_shift_down = 1;
            } else if (sc == 0xAA || sc == 0xB6) {
                g_shift_down = 0;
            } else if ((sc & 0x80) == 0 && sc < 128) {
                char c = g_shift_down ? g_scancode_shift[sc] : g_scancode_normal[sc];
                wslog("WS_KBD: sc=0x%02x char='%c' (term=%d)", sc, c ? c : '?', g_show_terminal);
                if (!g_show_terminal && (c == 't' || c == 'T')) {
                    g_show_terminal = 1;
                    g_term_focused = 1;
                    g_term_x = (g_w - (g_term_body_w + TERM_PAD * 2)) / 2;
                    g_term_y = 60;
                    g_term_redraw_needed = 1;
                    g_dirty = 1;
                } else if (g_show_terminal && c != 0) {
                    term_handle_char(c);
                    g_dirty = 1;
                }
            }
        }
    }

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
        if (g_show_terminal) {
            double win_body_x = px - (g_term_x + (double)TERM_PAD);
            double win_body_y = ((double)g_h - py) - (g_term_y + (double)TERM_PAD);

            /* Traffic Lights */
            if (win_body_y >= 4.0 && win_body_y <= 24.0) {
                /* Red Close */
                if (win_body_x >= 10.0 && win_body_x <= 28.0) {
                    g_show_terminal = 0;
                    g_term_dragging = 0;
                    g_term_resize_mode = TERM_RESIZE_NONE;
                    g_dirty = 1;
                    lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                    pthread_mutex_unlock(&g_lock);
                    return;
                }
                /* Yellow Minimize */
                if (win_body_x >= 30.0 && win_body_x <= 48.0) {
                    g_show_terminal = 0;
                    g_term_dragging = 0;
                    g_term_resize_mode = TERM_RESIZE_NONE;
                    g_dirty = 1;
                    lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                    pthread_mutex_unlock(&g_lock);
                    return;
                }
                /* Green Zoom (Toggle standard / expanded) */
                if (win_body_x >= 50.0 && win_body_x <= 68.0) {
                    if (!g_term_is_zoomed) {
                        g_term_prev_x = g_term_x;
                        g_term_prev_y = g_term_y;
                        g_term_prev_w = g_term_body_w;
                        g_term_prev_h = g_term_body_h;
                        g_term_x = 30;
                        g_term_y = MENUBAR_H + 8;
                        g_term_body_w = g_w - 60 - TERM_PAD * 2;
                        g_term_body_h = g_h - MENUBAR_H - 24 - TERM_PAD * 2;
                        g_term_is_zoomed = 1;
                    } else {
                        g_term_x = g_term_prev_x;
                        g_term_y = g_term_prev_y;
                        g_term_body_w = g_term_prev_w;
                        g_term_body_h = g_term_prev_h;
                        g_term_is_zoomed = 0;
                    }
                    g_term_redraw_needed = 1;
                    g_dirty = 1;
                    lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                    pthread_mutex_unlock(&g_lock);
                    return;
                }
            }

            /* Resizing Edge & Corner Hit-Testing (ONLY Terminal window is resizable) */
            int near_right = (win_body_x >= g_term_body_w - 10.0 && win_body_x <= g_term_body_w + 14.0);
            int near_bottom = (win_body_y >= g_term_body_h - 10.0 && win_body_y <= g_term_body_h + 14.0);
            int near_left = (win_body_x >= -14.0 && win_body_x <= 8.0);

            if (near_right && near_bottom) {
                g_term_resize_mode = TERM_RESIZE_BOTTOM_RIGHT;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            } else if (near_left && near_bottom) {
                g_term_resize_mode = TERM_RESIZE_BOTTOM_LEFT;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            } else if (near_right && win_body_y >= 0.0 && win_body_y <= g_term_body_h) {
                g_term_resize_mode = TERM_RESIZE_RIGHT;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            } else if (near_bottom && win_body_x >= 0.0 && win_body_x <= g_term_body_w) {
                g_term_resize_mode = TERM_RESIZE_BOTTOM;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            } else if (near_left && win_body_y >= 0.0 && win_body_y <= g_term_body_h) {
                g_term_resize_mode = TERM_RESIZE_LEFT;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            }

            /* Titlebar dragging */
            if (win_body_y >= 0.0 && win_body_y <= 28.0 && win_body_x >= 0.0 && win_body_x <= g_term_body_w) {
                g_term_dragging = 1;
                g_term_focused = 1;
                g_dirty = 1;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            }

            /* Terminal Window Body Click (Terminal focus/interaction - do not let click bleed through to Recovery!) */
            if (win_body_x >= 0.0 && win_body_x <= g_term_body_w &&
                win_body_y >= 0.0 && win_body_y <= g_term_body_h) {
                g_term_focused = 1;
                g_dirty = 1;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            }
        }

        if (g_show_about) {
            double win_body_x = px - (g_about_x + 28.0);
            double win_body_y = ((double)g_h - py) - (g_about_y + 28.0);
            int inside_body = (win_body_x >= 0 && win_body_x < ABOUT_WINDOW_BODY_W &&
                               win_body_y >= 0 && win_body_y < ABOUT_WINDOW_BODY_H);

            if (inside_body) {
                double rdx = win_body_x - 18.0;
                double rdy = win_body_y - 16.0;
                if (rdx * rdx + rdy * rdy <= 49.0) {
                    g_show_about = 0;
                    g_about_dragging = 0;
                    g_dirty = 1;
                    lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                    pthread_mutex_unlock(&g_lock);
                    return;
                }
                g_about_dragging = 1;
                g_dirty = 1;
                lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
                pthread_mutex_unlock(&g_lock);
                return;
            }
        }

        if (py >= (double)(g_h - MENUBAR_H)) {
            int clicked_menu = -1;
            int is_term_menu = (g_show_terminal && g_term_focused);
            if (is_term_menu) {
                if (px >= 6 && px < 35) clicked_menu = 10;
                else if (px >= 35 && px < 98) clicked_menu = 0;
                else if (px >= 98 && px < 138) clicked_menu = 1;
                else if (px >= 138 && px < 172) clicked_menu = 2;
                else if (px >= 172 && px < 212) clicked_menu = 3;
                else if (px >= 212 && px < 272) clicked_menu = 4;
                else if (px >= 272 && px < 315) clicked_menu = 5;
            } else {
                if (px >= 6 && px < 35) clicked_menu = 10;
                else if (px >= 35 && px < 100) clicked_menu = 0;
                else if (px >= 100 && px < 134) clicked_menu = 1;
                else if (px >= 134 && px < 172) clicked_menu = 2;
                else if (px >= 172 && px < 226) clicked_menu = 3;
                else if (px >= 226 && px < 285) clicked_menu = 4;
                else if (px >= 285 && px < 330) clicked_menu = 5;
            }

            g_active_menu = (g_active_menu == clicked_menu) ? -1 : clicked_menu;
            g_hovered_row = -1;
            g_dirty = 1;
            lastx = ms.mouse_x; lasty = ms.mouse_y; lastbuttons = buttons;
            pthread_mutex_unlock(&g_lock);
            return;
        } else if (g_active_menu >= 0) {
            int is_term_menu = (g_show_terminal && g_term_focused);
            int drop_x = 0, drop_y = 0, drop_w = 0, drop_h = 0;
            if (g_active_menu == 10) {
                drop_x = DROPDOWN_APPLE_X; drop_y = DROPDOWN_APPLE_Y;
                drop_w = DROPDOWN_APPLE_W; drop_h = DROPDOWN_APPLE_H;
            } else if (is_term_menu) {
                if (g_active_menu == 0) { drop_x = DROPDOWN_TERM_APP_X; drop_y = DROPDOWN_TERM_APP_Y; drop_w = DROPDOWN_TERM_APP_W; drop_h = DROPDOWN_TERM_APP_H; }
                else if (g_active_menu == 1) { drop_x = DROPDOWN_TERM_SHELL_X; drop_y = DROPDOWN_TERM_SHELL_Y; drop_w = DROPDOWN_TERM_SHELL_W; drop_h = DROPDOWN_TERM_SHELL_H; }
                else if (g_active_menu == 2) { drop_x = DROPDOWN_TERM_EDIT_X; drop_y = DROPDOWN_TERM_EDIT_Y; drop_w = DROPDOWN_TERM_EDIT_W; drop_h = DROPDOWN_TERM_EDIT_H; }
                else if (g_active_menu == 3) { drop_x = DROPDOWN_TERM_VIEW_X; drop_y = DROPDOWN_TERM_VIEW_Y; drop_w = DROPDOWN_TERM_VIEW_W; drop_h = DROPDOWN_TERM_VIEW_H; }
                else if (g_active_menu == 4) { drop_x = DROPDOWN_TERM_WINDOW_X; drop_y = DROPDOWN_TERM_WINDOW_Y; drop_w = DROPDOWN_TERM_WINDOW_W; drop_h = DROPDOWN_TERM_WINDOW_H; }
                else if (g_active_menu == 5) { drop_x = DROPDOWN_TERM_HELP_X; drop_y = DROPDOWN_TERM_HELP_Y; drop_w = DROPDOWN_TERM_HELP_W; drop_h = DROPDOWN_TERM_HELP_H; }
            } else {
                if (g_active_menu == 0) { drop_x = DROPDOWN_RECOVERY_X; drop_y = DROPDOWN_RECOVERY_Y; drop_w = DROPDOWN_RECOVERY_W; drop_h = DROPDOWN_RECOVERY_H; }
                else if (g_active_menu == 1) { drop_x = DROPDOWN_FILE_X; drop_y = DROPDOWN_FILE_Y; drop_w = DROPDOWN_FILE_W; drop_h = DROPDOWN_FILE_H; }
                else if (g_active_menu == 2) { drop_x = DROPDOWN_EDIT_X; drop_y = DROPDOWN_EDIT_Y; drop_w = DROPDOWN_EDIT_W; drop_h = DROPDOWN_EDIT_H; }
                else if (g_active_menu == 3) { drop_x = DROPDOWN_UTILITIES_X; drop_y = DROPDOWN_UTILITIES_Y; drop_w = DROPDOWN_UTILITIES_W; drop_h = DROPDOWN_UTILITIES_H; }
                else if (g_active_menu == 4) { drop_x = DROPDOWN_WINDOW_X; drop_y = DROPDOWN_WINDOW_Y; drop_w = DROPDOWN_WINDOW_W; drop_h = DROPDOWN_WINDOW_H; }
                else if (g_active_menu == 5) { drop_x = DROPDOWN_HELP_X; drop_y = DROPDOWN_HELP_Y; drop_w = DROPDOWN_HELP_W; drop_h = DROPDOWN_HELP_H; }
            }

            double mx = px - drop_x;
            double my = ((double)g_h - py) - drop_y;
            int clicked_row = -1;
            if (mx >= 0 && mx < drop_w && my >= 0 && my < drop_h) {
                if (g_active_menu == 10) {
                    if (my >= 4 && my < 25) clicked_row = 0;
                    else if (my >= 32 && my < 53) clicked_row = 2;
                    else if (my >= 53 && my < 74) clicked_row = 3;
                } else if (is_term_menu) {
                    if (g_active_menu == 0) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                        else if (my >= 32 && my < 53) clicked_row = 2;
                        else if (my >= 60 && my < 81) clicked_row = 4;
                        else if (my >= 88 && my < 109) clicked_row = 6;
                    } else if (g_active_menu == 1) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                        else if (my >= 25 && my < 46) clicked_row = 1;
                    } else if (g_active_menu == 2) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                        else if (my >= 25 && my < 46) clicked_row = 1;
                        else if (my >= 46 && my < 67) clicked_row = 2;
                    } else if (g_active_menu == 3) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                        else if (my >= 25 && my < 46) clicked_row = 1;
                    } else if (g_active_menu == 4) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                        else if (my >= 25 && my < 46) clicked_row = 1;
                        else if (my >= 53 && my < 74) clicked_row = 3;
                    } else if (g_active_menu == 5) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                    }
                } else {
                    if (g_active_menu == 0) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                    } else if (g_active_menu == 1) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                    } else if (g_active_menu == 3) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                        else if (my >= 25 && my < 46) clicked_row = 1;
                        else if (my >= 46 && my < 67) clicked_row = 2;
                        else if (my >= 67 && my < 88) clicked_row = 3;
                    } else if (g_active_menu == 4) {
                        if (my >= 53 && my < 74) clicked_row = 3;
                    } else if (g_active_menu == 5) {
                        if (my >= 4 && my < 25) clicked_row = 0;
                    }
                }
            }
            if (clicked_row < 0) clicked_row = g_hovered_row;

            /* Execute action of selected menu row */
            if (g_active_menu == 10) {
                if (clicked_row == 2) {
                    wslog("WindowServer: Apple Menu -> Restart clicked! Syncing and rebooting...");
                    sync();
                    reboot(0);
                } else if (clicked_row == 3) {
                    wslog("WindowServer: Apple Menu -> Shut Down clicked! Syncing and halting...");
                    sync();
                    reboot(0x08); /* RB_HALT */
                }
            } else if (is_term_menu) {
                if (g_active_menu == 0) {
                    if (clicked_row == 0) {
                        g_show_about = 1; g_about_x = ABOUT_WINDOW_X; g_about_y = ABOUT_WINDOW_Y;
                    } else if (clicked_row == 4) {
                        term_exec_command("clear");
                    } else if (clicked_row == 6) {
                        g_show_terminal = 0; g_term_focused = 0;
                    }
                } else if (g_active_menu == 1) {
                    if (clicked_row == 0) {
                        g_term_x = (g_w - (g_term_body_w + TERM_PAD * 2)) / 2; g_term_y = 60; g_term_redraw_needed = 1;
                    } else if (clicked_row == 1) {
                        g_show_terminal = 0; g_term_focused = 0;
                    }
                } else if (g_active_menu == 4) {
                    if (clicked_row == 0) {
                        g_show_terminal = 0; g_term_focused = 0;
                    } else if (clicked_row == 1) {
                        /* Toggle zoom */
                        if (!g_term_is_zoomed) {
                            g_term_prev_x = g_term_x; g_term_prev_y = g_term_y;
                            g_term_prev_w = g_term_body_w; g_term_prev_h = g_term_body_h;
                            g_term_x = 30; g_term_y = MENUBAR_H + 8;
                            g_term_body_w = g_w - 60 - TERM_PAD * 2; g_term_body_h = g_h - MENUBAR_H - 24 - TERM_PAD * 2;
                            g_term_is_zoomed = 1;
                        } else {
                            g_term_x = g_term_prev_x; g_term_y = g_term_prev_y;
                            g_term_body_w = g_term_prev_w; g_term_body_h = g_term_prev_h;
                            g_term_is_zoomed = 0;
                        }
                        g_term_redraw_needed = 1;
                    }
                }
            } else {
                if (g_active_menu == 0) {
                    if (clicked_row == 0) {
                        g_show_about = 1;
                        g_about_x = ABOUT_WINDOW_X;
                        g_about_y = ABOUT_WINDOW_Y;
                    }
                } else if (g_active_menu == 3) {
                    if (clicked_row == 2) {
                        wslog("WindowServer: Utilities -> Terminal clicked! Launching Terminal...");
                        g_show_terminal = 1;
                        g_term_focused = 1;
                        g_term_x = (g_w - (g_term_body_w + TERM_PAD * 2)) / 2;
                        g_term_y = 60;
                        g_term_redraw_needed = 1;
                    }
                }
            }
            g_active_menu = -1;
            g_hovered_row = -1;
            g_dirty = 1;
        }

        if (under && under->surface) {
            g_term_focused = 0;
            g_dirty = 1;
            int win_x = (int)(px - under->x);
            int win_y = (int)((under->y + under->h) - py);
            if (win_x >= 0 && win_x < RECOVERY_UI_W && win_y >= 0 && win_y < RECOVERY_UI_H) {
                int clicked_row = -1;
                if (win_y >= 16 && win_y < 90) clicked_row = 0;
                else if (win_y >= 90 && win_y < 164) clicked_row = 1;
                else if (win_y >= 164 && win_y < 238) clicked_row = 2;
                else if (win_y >= 238 && win_y < 312) clicked_row = 3;

                if (clicked_row >= 0) {
                    g_recovery_selected_row = clicked_row;
                    memcpy(under->surface, g_recovery_states[clicked_row], (size_t)RECOVERY_UI_W * (size_t)RECOVERY_UI_H * 4);
                    under->mtl_tex = NULL;
                    g_dirty = 1;
                } else if (win_x >= 324 && win_x <= 408 && win_y >= 328 && win_y <= 360) {
                    memcpy(under->surface, g_recovery_state_4, (size_t)RECOVERY_UI_W * (size_t)RECOVERY_UI_H * 4);
                    under->mtl_tex = NULL;
                    g_dirty = 1;
                }
            }
        }

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
    } else if (!(buttons & 1)) {
        g_about_dragging = 0;
        g_term_dragging = 0;
        g_term_resize_mode = TERM_RESIZE_NONE;
    }

    /* Terminal Dragging */
    if (g_show_terminal && g_term_dragging && (dx != 0 || dy != 0)) {
        g_term_x += dx;
        g_term_y -= dy;
        int max_x = g_w - (g_term_body_w + TERM_PAD);
        int max_y = g_h - (g_term_body_h + TERM_PAD);
        if (g_term_x < -TERM_PAD) g_term_x = -TERM_PAD;
        if (g_term_x > max_x) g_term_x = max_x;
        if (g_term_y < MENUBAR_H - TERM_PAD) g_term_y = MENUBAR_H - TERM_PAD;
        if (g_term_y > max_y) g_term_y = max_y;
        g_dirty = 1;
    }

    /* Terminal Resizing (ONLY Terminal window expands/stretches) */
    if (g_show_terminal && g_term_resize_mode != TERM_RESIZE_NONE && (dx != 0 || dy != 0)) {
        if (g_term_resize_mode == TERM_RESIZE_RIGHT) {
            g_term_body_w += (int)dx;
        } else if (g_term_resize_mode == TERM_RESIZE_BOTTOM) {
            g_term_body_h -= (int)dy;
        } else if (g_term_resize_mode == TERM_RESIZE_BOTTOM_RIGHT) {
            g_term_body_w += (int)dx;
            g_term_body_h -= (int)dy;
        } else if (g_term_resize_mode == TERM_RESIZE_LEFT) {
            g_term_x += dx;
            g_term_body_w -= (int)dx;
        } else if (g_term_resize_mode == TERM_RESIZE_BOTTOM_LEFT) {
            g_term_x += dx;
            g_term_body_w -= (int)dx;
            g_term_body_h -= (int)dy;
        }

        if (g_term_body_w < 380) g_term_body_w = 380;
        if (g_term_body_w > g_w - 40 - TERM_PAD * 2) g_term_body_w = g_w - 40 - TERM_PAD * 2;
        if (g_term_body_h < 180) g_term_body_h = 180;
        if (g_term_body_h > g_h - MENUBAR_H - 24 - TERM_PAD * 2) g_term_body_h = g_h - MENUBAR_H - 24 - TERM_PAD * 2;

        g_term_redraw_needed = 1;
        g_dirty = 1;
    }

    /* Dynamic Cursor Update on Hover over Terminal Window */
    if (g_show_terminal) {
        double win_body_x = px - (g_term_x + (double)TERM_PAD);
        double win_body_y = ((double)g_h - py) - (g_term_y + (double)TERM_PAD);
        if (win_body_x >= -8.0 && win_body_x <= g_term_body_w + 12.0 &&
            win_body_y >= -8.0 && win_body_y <= g_term_body_h + 12.0) {
            int near_r = (win_body_x >= g_term_body_w - 8.0);
            int near_b = (win_body_y >= g_term_body_h - 8.0);
            int near_l = (win_body_x <= 6.0);

            int target_cursor = WSCURSOR_ARROW;
            if (near_r && near_b) target_cursor = WSCURSOR_RESIZE_NWSE;
            else if (near_l && near_b) target_cursor = WSCURSOR_RESIZE_NESW;
            else if (near_r || near_l) target_cursor = WSCURSOR_RESIZE_LEFTRIGHT;
            else if (near_b) target_cursor = WSCURSOR_RESIZE_UPDOWN;
            else if (win_body_y > 28.0) target_cursor = WSCURSOR_IBEAM;

            if (g_current_cursor_id != target_cursor) {
                cursor_restore();
                g_current_cursor_id = target_cursor;
                composite_cursor();
                g_dirty = 1;
            }
        } else if (g_current_cursor_id != WSCURSOR_ARROW && !g_term_dragging && g_term_resize_mode == TERM_RESIZE_NONE) {
            cursor_restore();
            g_current_cursor_id = WSCURSOR_ARROW;
            composite_cursor();
            g_dirty = 1;
        }
    }

    if (g_show_about && g_about_dragging && (dx != 0 || dy != 0)) {
        g_about_x += dx;
        g_about_y -= dy;
        if (g_about_x < 0) g_about_x = 0;
        if (g_about_x > g_w - ABOUT_WINDOW_W) g_about_x = g_w - ABOUT_WINDOW_W;
        if (g_about_y < MENUBAR_H) g_about_y = MENUBAR_H;
        if (g_about_y > g_h - ABOUT_WINDOW_H) g_about_y = g_h - ABOUT_WINDOW_H;
        g_dirty = 1;
    }

    if (g_active_menu >= 0) {
        int is_term_menu = (g_show_terminal && g_term_focused);
        /* Real macOS menubar scrubbing across all 7 menus */
        if (py >= (double)(g_h - MENUBAR_H)) {
            int scrub = -1;
            if (is_term_menu) {
                if (px >= 6 && px < 35) scrub = 10;
                else if (px >= 35 && px < 98) scrub = 0;
                else if (px >= 98 && px < 138) scrub = 1;
                else if (px >= 138 && px < 172) scrub = 2;
                else if (px >= 172 && px < 212) scrub = 3;
                else if (px >= 212 && px < 272) scrub = 4;
                else if (px >= 272 && px < 315) scrub = 5;
            } else {
                if (px >= 6 && px < 35) scrub = 10;
                else if (px >= 35 && px < 100) scrub = 0;
                else if (px >= 100 && px < 134) scrub = 1;
                else if (px >= 134 && px < 172) scrub = 2;
                else if (px >= 172 && px < 226) scrub = 3;
                else if (px >= 226 && px < 285) scrub = 4;
                else if (px >= 285 && px < 330) scrub = 5;
            }

            if (scrub >= 0 && scrub != g_active_menu) {
                g_active_menu = scrub;
                g_hovered_row = -1;
                g_dirty = 1;
            }
        }

        int drop_x = 0, drop_y = 0, drop_w = 0, drop_h = 0;
        if (g_active_menu == 10) {
            drop_x = DROPDOWN_APPLE_X; drop_y = DROPDOWN_APPLE_Y;
            drop_w = DROPDOWN_APPLE_W; drop_h = DROPDOWN_APPLE_H;
        } else if (is_term_menu) {
            if (g_active_menu == 0) { drop_x = DROPDOWN_TERM_APP_X; drop_y = DROPDOWN_TERM_APP_Y; drop_w = DROPDOWN_TERM_APP_W; drop_h = DROPDOWN_TERM_APP_H; }
            else if (g_active_menu == 1) { drop_x = DROPDOWN_TERM_SHELL_X; drop_y = DROPDOWN_TERM_SHELL_Y; drop_w = DROPDOWN_TERM_SHELL_W; drop_h = DROPDOWN_TERM_SHELL_H; }
            else if (g_active_menu == 2) { drop_x = DROPDOWN_TERM_EDIT_X; drop_y = DROPDOWN_TERM_EDIT_Y; drop_w = DROPDOWN_TERM_EDIT_W; drop_h = DROPDOWN_TERM_EDIT_H; }
            else if (g_active_menu == 3) { drop_x = DROPDOWN_TERM_VIEW_X; drop_y = DROPDOWN_TERM_VIEW_Y; drop_w = DROPDOWN_TERM_VIEW_W; drop_h = DROPDOWN_TERM_VIEW_H; }
            else if (g_active_menu == 4) { drop_x = DROPDOWN_TERM_WINDOW_X; drop_y = DROPDOWN_TERM_WINDOW_Y; drop_w = DROPDOWN_TERM_WINDOW_W; drop_h = DROPDOWN_TERM_WINDOW_H; }
            else if (g_active_menu == 5) { drop_x = DROPDOWN_TERM_HELP_X; drop_y = DROPDOWN_TERM_HELP_Y; drop_w = DROPDOWN_TERM_HELP_W; drop_h = DROPDOWN_TERM_HELP_H; }
        } else {
            if (g_active_menu == 0) { drop_x = DROPDOWN_RECOVERY_X; drop_y = DROPDOWN_RECOVERY_Y; drop_w = DROPDOWN_RECOVERY_W; drop_h = DROPDOWN_RECOVERY_H; }
            else if (g_active_menu == 1) { drop_x = DROPDOWN_FILE_X; drop_y = DROPDOWN_FILE_Y; drop_w = DROPDOWN_FILE_W; drop_h = DROPDOWN_FILE_H; }
            else if (g_active_menu == 2) { drop_x = DROPDOWN_EDIT_X; drop_y = DROPDOWN_EDIT_Y; drop_w = DROPDOWN_EDIT_W; drop_h = DROPDOWN_EDIT_H; }
            else if (g_active_menu == 3) { drop_x = DROPDOWN_UTILITIES_X; drop_y = DROPDOWN_UTILITIES_Y; drop_w = DROPDOWN_UTILITIES_W; drop_h = DROPDOWN_UTILITIES_H; }
            else if (g_active_menu == 4) { drop_x = DROPDOWN_WINDOW_X; drop_y = DROPDOWN_WINDOW_Y; drop_w = DROPDOWN_WINDOW_W; drop_h = DROPDOWN_WINDOW_H; }
            else if (g_active_menu == 5) { drop_x = DROPDOWN_HELP_X; drop_y = DROPDOWN_HELP_Y; drop_w = DROPDOWN_HELP_W; drop_h = DROPDOWN_HELP_H; }
        }

        double mx = px - drop_x;
        double my = ((double)g_h - py) - drop_y;
        int new_hover = -1;
        if (mx >= 5 && mx < drop_w - 5 && my >= 4 && my < drop_h - 4) {
            if (g_active_menu == 10) {
                if (my >= 4 && my < 25) new_hover = 0;
                else if (my >= 32 && my < 53) new_hover = 2;
                else if (my >= 53 && my < 74) new_hover = 3;
            } else if (is_term_menu) {
                if (g_active_menu == 0) {
                    if (my >= 4 && my < 25) new_hover = 0;
                    else if (my >= 32 && my < 53) new_hover = 2;
                    else if (my >= 60 && my < 81) new_hover = 4;
                    else if (my >= 88 && my < 109) new_hover = 6;
                } else if (g_active_menu == 1) {
                    if (my >= 4 && my < 25) new_hover = 0;
                    else if (my >= 25 && my < 46) new_hover = 1;
                } else if (g_active_menu == 2) {
                    if (my >= 4 && my < 25) new_hover = 0;
                    else if (my >= 25 && my < 46) new_hover = 1;
                    else if (my >= 46 && my < 67) new_hover = 2;
                } else if (g_active_menu == 3) {
                    if (my >= 4 && my < 25) new_hover = 0;
                    else if (my >= 25 && my < 46) new_hover = 1;
                } else if (g_active_menu == 4) {
                    if (my >= 4 && my < 25) new_hover = 0;
                    else if (my >= 25 && my < 46) new_hover = 1;
                    else if (my >= 53 && my < 74) new_hover = 3;
                } else if (g_active_menu == 5) {
                    if (my >= 4 && my < 25) new_hover = 0;
                }
            } else {
                if (g_active_menu == 0) {
                    if (my >= 4 && my < 25) new_hover = 0;
                } else if (g_active_menu == 1) {
                    if (my >= 4 && my < 25) new_hover = 0;
                } else if (g_active_menu == 3) {
                    if (my >= 4 && my < 25) new_hover = 0;
                    else if (my >= 25 && my < 46) new_hover = 1;
                    else if (my >= 46 && my < 67) new_hover = 2;
                    else if (my >= 67 && my < 88) new_hover = 3;
                } else if (g_active_menu == 4) {
                    if (my >= 53 && my < 74) new_hover = 3;
                } else if (g_active_menu == 5) {
                    if (my >= 4 && my < 25) new_hover = 0;
                }
            }
        }
        if (new_hover != g_hovered_row) {
            g_hovered_row = new_hover;
            g_dirty = 1;
        }
    } else {
        if (g_hovered_row != -1) {
            g_hovered_row = -1;
            g_dirty = 1;
        }
    }

    if (!(buttons & 1) && (lastbuttons & 1)) {
        /* left up */
        if (under && under->surface) {
            int win_x = (int)(px - under->x);
            int win_y = (int)((under->y + under->h) - py);
            if (win_x >= 324 && win_x <= 408 && win_y >= 328 && win_y <= 360) {
                memcpy(under->surface, g_recovery_states[g_recovery_selected_row], (size_t)RECOVERY_UI_W * (size_t)RECOVERY_UI_H * 4);
                under->mtl_tex = NULL;
                g_dirty = 1;
            }
        }
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
    }

    if ((buttons & 2) && !(lastbuttons & 2)) {
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
            fill[k] = 0xff292929u;
        if (d->w == RECOVERY_UI_W && d->h == RECOVERY_UI_H) {
            memcpy(fill, g_recovery_ui_pixels, (size_t)RECOVERY_UI_W * (size_t)RECOVERY_UI_H * 4);
        }
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
        if (g_dirty != 0 || frames < 60) {
            frames++;
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

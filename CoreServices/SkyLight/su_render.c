/* su_render.c — canvas, text and symbol rendering. */
#include "su_render.h"
#include "su_atlas.h"

/* ------------------------------------------------------------------ canvas */
void su_canvas_init(SUCanvas *c, int w, int h)
{
    c->w = w; c->h = h; c->stride = w;
    c->px = (uint32_t *)calloc((size_t)w * (size_t)h, 4);
    c->cx0 = 0; c->cy0 = 0; c->cx1 = w; c->cy1 = h;
}
void su_canvas_bind(SUCanvas *c, uint32_t *px, int w, int h, int stride)
{
    c->w = w; c->h = h; c->stride = stride > 0 ? stride : w;
    c->px = px;
    c->cx0 = 0; c->cy0 = 0; c->cx1 = w; c->cy1 = h;
}
void su_canvas_free(SUCanvas *c) { free(c->px); c->px = NULL; }
void su_reset_clip(SUCanvas *c) { c->cx0 = 0; c->cy0 = 0; c->cx1 = c->w; c->cy1 = c->h; }
void su_set_clip(SUCanvas *c, float x, float y, float w, float h)
{
    int x0 = (int)floorf(x), y0 = (int)floorf(y);
    int x1 = (int)ceilf(x + w), y1 = (int)ceilf(y + h);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > c->w) x1 = c->w;
    if (y1 > c->h) y1 = c->h;
    c->cx0 = x0; c->cy0 = y0; c->cx1 = x1; c->cy1 = y1;
}

uint32_t su_rgba(int r, int g, int b, int a)
{
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    if (a < 0) a = 0; if (a > 255) a = 255;
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}
uint32_t su_gray(int v, int a) { return su_rgba(v, v, v, a); }

static inline void blend(SUCanvas *c, int x, int y, uint32_t s, float cov)
{
    if (x < c->cx0 || x >= c->cx1 || y < c->cy0 || y >= c->cy1) return;
    unsigned sa = (unsigned)(((s >> 24) & 255) * cov);
    if (sa == 0) return;
    uint32_t *d = &c->px[(size_t)y * (size_t)c->stride + (size_t)x];
    unsigned da = (*d >> 24) & 255;
    unsigned sr = (s >> 16) & 255, sg = (s >> 8) & 255, sb = s & 255;

    if (da == 0) {
        *d = (sa << 24) | (sr << 16) | (sg << 8) | sb;
        return;
    }

    unsigned inv = 255u - sa;
    unsigned out_a = sa + (da * inv + 127u) / 255u;
    if (out_a > 255) out_a = 255;
    if (out_a == 0) { *d = 0; return; }

    unsigned dr = (*d >> 16) & 255, dg = (*d >> 8) & 255, db = *d & 255;
    unsigned r = (sr * sa + ((dr * da * inv + 127u) / 255u)) / out_a;
    unsigned g = (sg * sa + ((dg * da * inv + 127u) / 255u)) / out_a;
    unsigned b = (sb * sa + ((db * da * inv + 127u) / 255u)) / out_a;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    *d = (out_a << 24) | (r << 16) | (g << 8) | b;
}

void su_fill(SUCanvas *c, uint32_t argb)
{
    int y, x;
    for (y = c->cy0; y < c->cy1; y++)
        for (x = c->cx0; x < c->cx1; x++)
            c->px[(size_t)y * c->stride + x] = argb;
}

/* signed distance to a continuous squircle (Apple superellipse n=2.6) */
static inline float sd_rrect(float px, float py, float x, float y, float w, float h, float r)
{
    if (r <= 0.0f) {
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        float hx = w * 0.5f, hy = h * 0.5f;
        float dx = fabsf(px - cx) - hx, dy = fabsf(py - cy) - hy;
        float ax = dx > 0 ? dx : 0, ay = dy > 0 ? dy : 0;
        float m = dx > dy ? dx : dy;
        return sqrtf(ax * ax + ay * ay) + (m < 0 ? m : 0);
    }
    float cx = x + w * 0.5f, cy = y + h * 0.5f;
    float hx = w * 0.5f - r, hy = h * 0.5f - r;
    float dx = fabsf(px - cx) - hx, dy = fabsf(py - cy) - hy;
    if (dx <= 0.0f && dy <= 0.0f) {
        float m = dx > dy ? dx : dy;
        return m - r;
    }
    if (dx <= 0.0f) return dy - r;
    if (dy <= 0.0f) return dx - r;
    return sqrtf(dx * dx + dy * dy) - r;
}

void su_fill_pie_slice(SUCanvas *c, float cx, float cy, float r_in, float r_out,
                       float a0, float a1, uint32_t argb)
{
    int x0 = (int)floorf(cx - r_out) - 1;
    int y0 = (int)floorf(cy - r_out) - 1;
    int x1 = (int)ceilf(cx + r_out) + 1;
    int y1 = (int)ceilf(cy + r_out) + 1;
    int px, py;
    const float two_pi = 6.28318530718f;

    while (a1 < a0) a1 += two_pi;

    for (py = y0; py <= y1; py++) {
        for (px = x0; px <= x1; px++) {
            float dx = (px + 0.5f) - cx;
            float dy = (py + 0.5f) - cy;
            float r = sqrtf(dx * dx + dy * dy);
            float d_outer = r - r_out;
            float d_inner = r_in - r;
            float cov_r = 0.5f - (d_outer > d_inner ? d_outer : d_inner);
            if (cov_r <= 0.0f) continue;
            if (cov_r > 1.0f) cov_r = 1.0f;

            float ang = atan2f(dy, dx);
            if (ang < 0.0f) ang += two_pi;
            int inside = 0;
            if (a1 - a0 >= two_pi - 0.001f) {
                inside = 1;
            } else {
                float a = ang;
                if (a < a0) a += two_pi;
                if (a >= a0 && a <= a1) inside = 1;
            }
            if (inside) {
                blend(c, px, py, argb, cov_r);
            }
        }
    }
}

void su_fill_rrect(SUCanvas *c, float x, float y, float w, float h, float r, uint32_t argb)
{
    int x0 = (int)floorf(x) - 1, y0 = (int)floorf(y) - 1;
    int x1 = (int)ceilf(x + w) + 1, y1 = (int)ceilf(y + h) + 1;
    int px, py;
    if (r > w * 0.5f) r = w * 0.5f;
    if (r > h * 0.5f) r = h * 0.5f;
    for (py = y0; py <= y1; py++)
        for (px = x0; px <= x1; px++) {
            float d = sd_rrect(px + 0.5f, py + 0.5f, x, y, w, h, r);
            float cov = 0.5f - d;
            if (cov <= 0) continue;
            if (cov > 1) cov = 1;
            blend(c, px, py, argb, cov);
        }
}

void su_fill_rect(SUCanvas *c, float x, float y, float w, float h, uint32_t argb)
{
    su_fill_rrect(c, x, y, w, h, 0, argb);
}

void su_stroke_rrect(SUCanvas *c, float x, float y, float w, float h, float r,
                     float lw, uint32_t argb)
{
    int x0 = (int)floorf(x - lw) - 1, y0 = (int)floorf(y - lw) - 1;
    int x1 = (int)ceilf(x + w + lw) + 1, y1 = (int)ceilf(y + h + lw) + 1;
    int px, py;
    float half = lw * 0.5f;
    if (r > w * 0.5f) r = w * 0.5f;
    if (r > h * 0.5f) r = h * 0.5f;
    for (py = y0; py <= y1; py++)
        for (px = x0; px <= x1; px++) {
            float d = fabsf(sd_rrect(px + 0.5f, py + 0.5f, x + half, y + half,
                                     w - lw, h - lw, r - half));
            float cov = 0.5f + half - d;
            if (cov <= 0) continue;
            if (cov > 1) cov = 1;
            blend(c, px, py, argb, cov);
        }
}

void su_stroke_circle(SUCanvas *c, float cx, float cy, float r, float lw, uint32_t argb)
{
    su_stroke_rrect(c, cx - r, cy - r, r * 2.0f, r * 2.0f, r, lw, argb);
}

void su_fill_circle(SUCanvas *c, float cx, float cy, float r, uint32_t argb)
{
    su_fill_rrect(c, cx - r, cy - r, r * 2, r * 2, r, argb);
}

void su_shadow_rrect(SUCanvas *c, float x, float y, float w, float h, float r,
                     float blur, float dy, uint32_t argb)
{
    int pad = (int)ceilf(blur * 2.5f) + 2;
    int x0 = (int)floorf(x) - pad, y0 = (int)floorf(y + dy) - pad;
    int x1 = (int)ceilf(x + w) + pad, y1 = (int)ceilf(y + h + dy) + pad;
    int bw = x1 - x0, bh = y1 - y0;
    float *m, *t;
    int px, py, i;
    if (bw <= 0 || bh <= 0) return;
    m = (float *)calloc((size_t)bw * bh, sizeof(float));
    t = (float *)calloc((size_t)bw * bh, sizeof(float));
    for (py = 0; py < bh; py++)
        for (px = 0; px < bw; px++) {
            float d = sd_rrect(x0 + px + 0.5f, y0 + py + 0.5f, x, y + dy, w, h, r);
            float cov = 0.5f - d;
            m[(size_t)py * bw + px] = cov <= 0 ? 0 : (cov > 1 ? 1 : cov);
        }
    /* 3 box passes ~ gaussian */
    for (i = 0; i < 3; i++) {
        int rad = (int)(blur * 0.6f) + 1, k;
        float norm = 1.0f / (float)(rad * 2 + 1);
        for (py = 0; py < bh; py++) {
            float acc = 0;
            for (k = -rad; k <= rad; k++) {
                int xx = k < 0 ? 0 : (k >= bw ? bw - 1 : k);
                acc += m[(size_t)py * bw + xx];
            }
            for (px = 0; px < bw; px++) {
                t[(size_t)py * bw + px] = acc * norm;
                int add = px + rad + 1, rem = px - rad;
                if (add >= bw) add = bw - 1;
                if (rem < 0) rem = 0;
                acc += m[(size_t)py * bw + add] - m[(size_t)py * bw + rem];
            }
        }
        for (px = 0; px < bw; px++) {
            float acc = 0;
            for (k = -rad; k <= rad; k++) {
                int yy = k < 0 ? 0 : (k >= bh ? bh - 1 : k);
                acc += t[(size_t)yy * bw + px];
            }
            for (py = 0; py < bh; py++) {
                m[(size_t)py * bw + px] = acc * norm;
                int add = py + rad + 1, rem = py - rad;
                if (add >= bh) add = bh - 1;
                if (rem < 0) rem = 0;
                acc += t[(size_t)add * bw + px] - t[(size_t)rem * bw + px];
            }
        }
    }
    {
        unsigned sa = (argb >> 24) & 255;
        for (py = 0; py < bh; py++)
            for (px = 0; px < bw; px++) {
                float a = m[(size_t)py * bw + px] * (float)sa / 255.0f;
                if (a <= 0.002f) continue;
                blend(c, x0 + px, y0 + py, argb, a);
            }
    }
    free(m); free(t);
}

void su_vgradient_rrect(SUCanvas *c, float x, float y, float w, float h, float r,
                        uint32_t top, uint32_t bottom)
{
    int y0 = (int)floorf(y) - 1, y1 = (int)ceilf(y + h) + 1;
    int py;
    for (py = y0; py <= y1; py++) {
        float t = (py - y) / (h > 0 ? h : 1);
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        {
            int ar = (int)(((top >> 16) & 255) + t * (((bottom >> 16) & 255) - ((top >> 16) & 255)));
            int ag = (int)(((top >> 8) & 255) + t * (((bottom >> 8) & 255) - ((top >> 8) & 255)));
            int ab = (int)((top & 255) + t * ((bottom & 255) - (top & 255)));
            int aa = (int)(((top >> 24) & 255) + t * (((bottom >> 24) & 255) - ((top >> 24) & 255)));
            su_fill_rrect(c, x, (float)py, w, 1.0f, 0, su_rgba(ar, ag, ab, aa));
        }
    }
    (void)r;
}

void su_blit_rgba(SUCanvas *c, float dx, float dy, float dw, float dh,
                  const uint8_t *rgba, int sw, int sh, uint32_t tint)
{
    int x, y;
    float sx = (float)sw / (dw > 0 ? dw : 1);
    float sy = (float)sh / (dh > 0 ? dh : 1);
    unsigned tr = (tint >> 16) & 255, tg = (tint >> 8) & 255, tb = tint & 255;
    for (y = 0; y < (int)dh; y++)
        for (x = 0; x < (int)dw; x++) {
            int ix = (int)(x * sx), iy = (int)(y * sy);
            const uint8_t *p;
            unsigned a;
            if (ix >= sw || iy >= sh) continue;
            p = rgba + ((size_t)iy * sw + ix) * 4;
            a = p[3];
            if (!a) continue;
            if (tint) {
                blend(c, (int)dx + x, (int)dy + y,
                      su_rgba((p[0] * tr) / 255, (p[1] * tg) / 255, (p[2] * tb) / 255, a), 1.0f);
            } else {
                blend(c, (int)dx + x, (int)dy + y, su_rgba(p[0], p[1], p[2], a), 1.0f);
            }
        }
}

/* -------------------------------------------------------------------- text */
const SUFont SU_REG11 = {"reg11"}, SU_MED11 = {"med11"}, SU_SEM11 = {"sem11"};
const SUFont SU_REG12 = {"reg12"}, SU_MED12 = {"med12"}, SU_SEM12 = {"sem12"};
const SUFont SU_REG13 = {"reg13"}, SU_MED13 = {"med13"}, SU_SEM13 = {"sem13"};
const SUFont SU_REG15 = {"reg15"}, SU_MED15 = {"med15"}, SU_SEM15 = {"sem15"};
const SUFont SU_MED17 = {"med17"};
const SUFont SU_MED22 = {"med22"}, SU_BOLD22 = {"bold22"}, SU_BOLD26 = {"bold26"},
             SU_BOLD28 = {"bold28"}, SU_MED28 = {"med28"};

static const SUGlyph *find_glyph(const char *font, unsigned int ch)
{
    int i;
    for (i = 0; i < SU_GLYPH_COUNT; i++)
        if (su_glyphs[i].ch == ch && strcmp(su_glyphs[i].font, font) == 0)
            return &su_glyphs[i];
    return NULL;
}

/* utf8 -> codepoint, returns bytes consumed */
static int utf8_next(const char *s, unsigned int *cp)
{
    unsigned char c = (unsigned char)s[0];
    if (!c) { *cp = 0; return 0; }
    if (c < 0x80) { *cp = c; return 1; }
    if ((c & 0xE0) == 0xC0) { *cp = ((c & 0x1F) << 6) | ((unsigned char)s[1] & 0x3F); return 2; }
    if ((c & 0xF0) == 0xE0) {
        *cp = ((c & 0x0F) << 12) | (((unsigned char)s[1] & 0x3F) << 6) |
              ((unsigned char)s[2] & 0x3F);
        return 3;
    }
    *cp = ((c & 0x07) << 18) | (((unsigned char)s[1] & 0x3F) << 12) |
          (((unsigned char)s[2] & 0x3F) << 6) | ((unsigned char)s[3] & 0x3F);
    return 4;
}

static float su_trk = 0.0f;
void su_track(float px) { su_trk = px; }

/* The exported atlas records a 2 px advance for U+0020 in every cut, which
 * collapses inter-word spacing ("Mount Point:" -> "MountPoint:"). SF Pro's
 * word space is ~0.26 em, so widen it while keeping the recorded value as a
 * floor for the small cuts. */
static float adv_of(const SUGlyph *g, unsigned int cp)
{
    if (cp == 32) {
        float a = (float)g->size * 0.26f;
        return a > g->adv ? a : g->adv;
    }
    return g->adv;
}

void su_text(SUCanvas *c, SUFont f, float x, float y, const char *utf8, uint32_t argb)
{
    const char *s = utf8;
    float pen = x;
    while (*s) {
        unsigned int cp;
        int n = utf8_next(s, &cp);
        const SUGlyph *g;
        if (!n) break;
        s += n;
        g = find_glyph(f.name, cp);
        if (!g) { pen += 6.0f; continue; }
        {
            int gy;
            for (gy = 0; gy < g->h; gy++) {
                int gx;
                for (gx = 0; gx < g->w; gx++) {
                    unsigned a = su_glyph_px[(size_t)(g->y + gy) * SU_ATLAS_W + (g->x + gx)];
                    if (!a) continue;
                    blend(c, (int)(pen + g->ox + gx), (int)(y + g->oy + gy), argb, a / 255.0f);
                }
            }
        }
        pen += adv_of(g, cp) + su_trk;
    }
}

float su_text_width(SUFont f, const char *utf8)
{
    const char *s = utf8;
    float w = 0;
    while (*s) {
        unsigned int cp;
        int n = utf8_next(s, &cp);
        const SUGlyph *g;
        if (!n) break;
        s += n;
        g = find_glyph(f.name, cp);
        w += (g ? adv_of(g, cp) : 6.0f) + su_trk;
    }
    return w > 0 ? w - su_trk : 0;
}

float su_font_size(SUFont f)
{
    int i;
    for (i = 0; i < SU_GLYPH_COUNT; i++)
        if (strcmp(su_glyphs[i].font, f.name) == 0) return (float)su_glyphs[i].size;
    return 13;
}
float su_font_ascent(SUFont f)
{
    int i;
    for (i = 0; i < SU_GLYPH_COUNT; i++)
        if (strcmp(su_glyphs[i].font, f.name) == 0) return (float)su_glyphs[i].asc;
    return 10;
}
float su_font_cap(SUFont f) { return su_font_size(f) * 0.72f; }

void su_text_top(SUCanvas *c, SUFont f, float x, float capTop, const char *utf8, uint32_t argb)
{
    su_text(c, f, x, capTop - (su_font_ascent(f) - su_font_cap(f)), utf8, argb);
}
void su_text_mid(SUCanvas *c, SUFont f, float x, float cy, const char *utf8, uint32_t argb)
{
    su_text_top(c, f, x, cy - su_font_cap(f) * 0.5f, utf8, argb);
}
void su_text_right(SUCanvas *c, SUFont f, float xRight, float capTop, const char *utf8,
                   uint32_t argb)
{
    su_text_top(c, f, xRight - su_text_width(f, utf8), capTop, utf8, argb);
}
void su_text_clipped(SUCanvas *c, SUFont f, float x, float capTop, float maxw, const char *utf8,
                     uint32_t argb)
{
    if (su_text_width(f, utf8) <= maxw) {
        su_text_top(c, f, x, capTop, utf8, argb);
        return;
    }
    {
        char buf[512];
        size_t n = strlen(utf8);
        size_t cut;
        if (n >= sizeof(buf)) n = sizeof(buf) - 1;
        for (cut = n; cut > 0; cut--) {
            memcpy(buf, utf8, cut);
            buf[cut] = 0;
            strcat(buf, "\xe2\x80\xa6");
            if (su_text_width(f, buf) <= maxw) {
                su_text_top(c, f, x, capTop, buf, argb);
                return;
            }
        }
    }
}

void su_text_center(SUCanvas *c, SUFont f, float cx, float capTop, const char *utf8,
                    uint32_t argb)
{
    su_text_top(c, f, cx - su_text_width(f, utf8) * 0.5f, capTop, utf8, argb);
}

void su_text_clipped_mid(SUCanvas *c, SUFont f, float x, float cy, float maxw, const char *utf8, uint32_t argb)
{
    su_text_clipped(c, f, x, cy - su_font_cap(f) * 0.5f, maxw, utf8, argb);
}

void su_text_right_mid(SUCanvas *c, SUFont f, float xRight, float cy, const char *utf8, uint32_t argb)
{
    su_text_right(c, f, xRight, cy - su_font_cap(f) * 0.5f, utf8, argb);
}

void su_text_center_mid(SUCanvas *c, SUFont f, float cx, float cy, const char *utf8,
                        uint32_t argb)
{
    su_text_mid(c, f, cx - su_text_width(f, utf8) * 0.5f, cy, utf8, argb);
}

/* ----------------------------------------------------------------- symbols */
/* Sprites are stored as pre-rendered mip levels named "<symbol>@<level>", so a
 * blit at the natural size never resamples the artwork. */
static const SUSprite *pick_sprite(const char *name, float want, int by_height)
{
    const SUSprite *best = NULL;
    float bestd = 1e9f;
    size_t len = strlen(name);
    int i;
    for (i = 0; i < SU_SPRITE_COUNT; i++) {
        const SUSprite *sp = &su_sprites[i];
        float d;
        if (strncmp(sp->name, name, len) != 0 || sp->name[len] != '@') continue;
        if (by_height)
            d = fabsf((float)sp->h - want);
        else
            d = fabsf((float)(sp->w > sp->h ? sp->w : sp->h) - want);
        if (d < bestd) { bestd = d; best = sp; }
    }
    return best;
}

static void blit_sprite(SUCanvas *c, const SUSprite *s, int dx, int dy, uint32_t argb)
{
    int tint = (argb != 0);
    unsigned tr = 0, tg = 0, tb = 0, ta = 255;
    int x, y;
    if (tint) {
        tr = (argb >> 16) & 255; tg = (argb >> 8) & 255; tb = argb & 255;
        ta = (argb >> 24) & 255;
    }
    for (y = 0; y < s->h; y++)
        for (x = 0; x < s->w; x++) {
            uint32_t p = su_spr_px[(size_t)(s->y + y) * SU_SPR_W + (s->x + x)];
            unsigned a = ((p >> 24) & 255) * ta / 255;
            if (!a) continue;
            if (tint)
                blend(c, dx + x, dy + y, su_rgba(tr, tg, tb, a), 1.0f);
            else
                blend(c, dx + x, dy + y,
                      su_rgba((p >> 16) & 255, (p >> 8) & 255, p & 255, a), 1.0f);
        }
}

/* Area-average resample of a sprite into an arbitrary box. Box filtering (not
 * bilinear) is what keeps a 21 px master crisp when it is drawn at 17 px,
 * which is the common case: SF Symbols are sized by *height*, and the
 * pre-rendered mip levels can never land on every integer size. */
static void blit_scaled(SUCanvas *c, const SUSprite *s, int dx, int dy, int dw, int dh,
                        uint32_t argb)
{
    int tint = (argb != 0);
    unsigned tr = 0, tg = 0, tb = 0, ta = 255;
    float kx, ky;
    int ix, iy;

    if (dw <= 0 || dh <= 0) return;
    if (dw == s->w && dh == s->h) { blit_sprite(c, s, dx, dy, argb); return; }
    if (tint) {
        tr = (argb >> 16) & 255; tg = (argb >> 8) & 255; tb = argb & 255;
        ta = (argb >> 24) & 255;
    }
    kx = (float)s->w / (float)dw;
    ky = (float)s->h / (float)dh;

    for (iy = 0; iy < dh; iy++)
        for (ix = 0; ix < dw; ix++) {
            float fx0 = (float)ix * kx, fx1 = fx0 + kx;
            float fy0 = (float)iy * ky, fy1 = fy0 + ky;
            int sx0 = (int)floorf(fx0), sx1 = (int)ceilf(fx1) - 1;
            int sy0 = (int)floorf(fy0), sy1 = (int)ceilf(fy1) - 1;
            float r = 0, g = 0, b = 0, al = 0, wsum = 0;
            int sx, sy;

            if (sx1 < sx0) sx1 = sx0;
            if (sy1 < sy0) sy1 = sy0;
            for (sy = sy0; sy <= sy1; sy++)
                for (sx = sx0; sx <= sx1; sx++) {
                    float wx = fminf(fx1, (float)sx + 1) - fmaxf(fx0, (float)sx);
                    float wy = fminf(fy1, (float)sy + 1) - fmaxf(fy0, (float)sy);
                    float wgt;
                    uint32_t p;
                    float pa;
                    int tx, ty;
                    if (wx <= 0 || wy <= 0) continue;
                    wgt = wx * wy;
                    tx = s->x + sx; ty = s->y + sy;
                    if (tx < s->x || tx >= s->x + s->w) continue;
                    if (ty < s->y || ty >= s->y + s->h) continue;
                    p = su_spr_px[(size_t)ty * SU_SPR_W + tx];
                    pa = (float)((p >> 24) & 255);
                    al += pa * wgt;
                    r += (float)((p >> 16) & 255) * pa * wgt;
                    g += (float)((p >> 8) & 255) * pa * wgt;
                    b += (float)(p & 255) * pa * wgt;
                    wsum += wgt;
                }
            if (wsum <= 0) continue;
            al /= wsum;
            if (al < 0.4f) continue;
            {
                unsigned a = (unsigned)(al * ta / 255.0f);
                if (!a) continue;
                if (tint)
                    blend(c, dx + ix, dy + iy, su_rgba(tr, tg, tb, a), 1.0f);
                else
                    blend(c, dx + ix, dy + iy,
                          su_rgba((int)(r / al), (int)(g / al), (int)(b / al), a), 1.0f);
            }
        }
}

/* stretch-blit into an explicit box (artwork with no matching mip level) */
void su_symbol_rect(SUCanvas *c, const char *name, float x, float y, float w, float h,
                    uint32_t argb)
{
    const SUSprite *s = pick_sprite(name, w > h ? w : h, 0);
    if (!s) return;
    blit_scaled(c, s, (int)floorf(x), (int)floorf(y), (int)(w + 0.5f), (int)(h + 0.5f), argb);
}

/* `size` is the longest side of the ink box (legacy behaviour, exact size) */
void su_symbol(SUCanvas *c, const char *name, float cx, float cy, float size, uint32_t argb)
{
    const SUSprite *s = pick_sprite(name, size, 0);
    int dmax = (int)(size + 0.5f), dw, dh;
    if (!s || dmax < 1) return;
    if (s->w >= s->h) {
        dw = dmax;
        dh = (int)((float)dmax * s->h / s->w + 0.5f);
    } else {
        dh = dmax;
        dw = (int)((float)dmax * s->w / s->h + 0.5f);
    }
    if (dw < 1) dw = 1;
    if (dh < 1) dh = 1;
    blit_scaled(c, s, (int)floorf(cx - dw * 0.5f), (int)floorf(cy - dh * 0.5f), dw, dh, argb);
}

/* SF Symbols are specified by height; the width follows the glyph's aspect */
void su_symbol_h(SUCanvas *c, const char *name, float cx, float cy, float h, uint32_t argb)
{
    const SUSprite *s = pick_sprite(name, h, 1);
    int dh = (int)(h + 0.5f), dw;
    if (!s || dh < 1) return;
    dw = (int)((float)dh * s->w / s->h + 0.5f);
    if (dw < 1) dw = 1;
    blit_scaled(c, s, (int)floorf(cx - dw * 0.5f), (int)floorf(cy - dh * 0.5f), dw, dh, argb);
}

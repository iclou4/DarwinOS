/*
 * su_render.h — SwiftUI render core (portable C, no external dependencies).
 *
 * Canvas with signed-distance-field rounded rects, box-blur shadows, an
 * SF Pro glyph atlas with CoreText-style tracking, and pre-rendered SF Symbol
 * mip levels. This is the single drawing backend used by both the SwiftUI
 * framework and the WindowServer's Disk Utility.
 */
#ifndef SU_RENDER_H
#define SU_RENDER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    uint32_t *px;              /* 0xAARRGGBB */
    int w, h;
    int stride;                /* pixels per scanline (>= w) */
    int cx0, cy0, cx1, cy1;    /* clip rect (exclusive) */
} SUCanvas;

void su_canvas_init(SUCanvas *c, int w, int h);
/* draw straight into an existing surface (e.g. the WindowServer framebuffer) */
void su_canvas_bind(SUCanvas *c, uint32_t *px, int w, int h, int stride);
void su_canvas_free(SUCanvas *c);
void su_set_clip(SUCanvas *c, float x, float y, float w, float h);
void su_reset_clip(SUCanvas *c);

void su_fill(SUCanvas *c, uint32_t argb);
void su_fill_rect(SUCanvas *c, float x, float y, float w, float h, uint32_t argb);
void su_fill_rrect(SUCanvas *c, float x, float y, float w, float h, float r, uint32_t argb);
void su_stroke_rrect(SUCanvas *c, float x, float y, float w, float h, float r,
                     float lw, uint32_t argb);
void su_stroke_circle(SUCanvas *c, float cx, float cy, float r, float lw, uint32_t argb);
void su_fill_circle(SUCanvas *c, float cx, float cy, float r, uint32_t argb);
void su_fill_pie_slice(SUCanvas *c, float cx, float cy, float r_in, float r_out, float a0, float a1, uint32_t argb);
void su_shadow_rrect(SUCanvas *c, float x, float y, float w, float h, float r,
                     float blur, float dy, uint32_t argb);
void su_vgradient_rrect(SUCanvas *c, float x, float y, float w, float h, float r,
                        uint32_t top, uint32_t bottom);
void su_blit_rgba(SUCanvas *c, float dx, float dy, float dw, float dh,
                  const uint8_t *rgba, int sw, int sh, uint32_t tint);

uint32_t su_rgba(int r, int g, int b, int a);
uint32_t su_gray(int v, int a);

typedef struct { const char *name; } SUFont;
extern const SUFont SU_REG11, SU_MED11, SU_SEM11, SU_REG12, SU_MED12, SU_SEM12,
       SU_REG13, SU_MED13, SU_SEM13, SU_REG15, SU_MED15, SU_SEM15,
       SU_MED17, SU_MED22, SU_BOLD22, SU_BOLD26, SU_BOLD28, SU_MED28;

/* letter-spacing applied to every following text run (SF Pro Text tracking) */
void  su_track(float px);
void  su_text(SUCanvas *c, SUFont f, float x, float y, const char *utf8, uint32_t argb);
float su_text_width(SUFont f, const char *utf8);
float su_font_size(SUFont f);
float su_font_ascent(SUFont f);
float su_font_cap(SUFont f);

/* y is the *cap top* of the run instead of the ascender line */
void su_text_top(SUCanvas *c, SUFont f, float x, float capTop, const char *utf8, uint32_t argb);
/* vertically centre the cap box on cy */
void su_text_mid(SUCanvas *c, SUFont f, float x, float cy, const char *utf8, uint32_t argb);
/* right-align the run so that it ends at xRight */
void su_text_right(SUCanvas *c, SUFont f, float xRight, float capTop, const char *utf8,
                   uint32_t argb);
/* truncate with an ellipsis so the run never exceeds maxw */
void su_text_clipped(SUCanvas *c, SUFont f, float x, float capTop, float maxw, const char *utf8,
                     uint32_t argb);
/* horizontally centre the run on cx */
void su_text_center(SUCanvas *c, SUFont f, float cx, float capTop, const char *utf8,
                    uint32_t argb);
void su_text_clipped_mid(SUCanvas *c, SUFont f, float x, float cy, float maxw, const char *utf8, uint32_t argb);
void su_text_right_mid(SUCanvas *c, SUFont f, float xRight, float cy, const char *utf8, uint32_t argb);
void su_text_center_mid(SUCanvas *c, SUFont f, float cx, float cy, const char *utf8,
                        uint32_t argb);

void su_symbol(SUCanvas *c, const char *name, float cx, float cy, float size, uint32_t argb);
void su_symbol_rect(SUCanvas *c, const char *name, float x, float y, float w, float h,
                    uint32_t argb);
/* symbol whose height is h, width follows the natural aspect ratio */
void su_symbol_h(SUCanvas *c, const char *name, float cx, float cy, float h, uint32_t argb);

#endif

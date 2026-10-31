/*
 * disk_utility_view.c — macOS Tahoe 26 Recovery Disk Utility
 * Assembled with declarative SwiftUI component design tokens and squircle curvature.
 */
#include "disk_utility_view.h"
#include "su_render.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ---------------------------------------------------------- design tokens */
#define C_WINDOW      su_gray(30, 255)
#define C_CARD        su_gray(40, 255)
#define C_CARD_LIST   su_gray(32, 255)
#define C_CARD_TOP    su_gray(56, 255)
#define C_TEXT        su_rgba(240, 240, 240, 255)
#define C_TEXT2       su_rgba(160, 160, 160, 255)
#define C_HEADER      su_rgba(150, 150, 150, 255)
#define C_LABEL       su_rgba(145, 145, 145, 255)
#define C_VALUE       su_rgba(235, 235, 235, 255)
#define C_ACCENT      su_rgba(52, 122, 240, 255)
#define C_ACCENT_BG   su_rgba(36, 87, 201, 255)
#define C_GLASS       su_rgba(255, 255, 255, 30)
#define C_GLASS_EDGE  su_rgba(255, 255, 255, 60)
#define C_BORDER      su_rgba(255, 255, 255, 50)
#define C_HAIR        su_rgba(255, 255, 255, 35)
#define C_BOX         su_rgba(255, 255, 255, 14)
#define C_GRID        su_rgba(255, 255, 255, 12)
#define C_POPOVER     su_gray(48, 255)

/* ------------------------------------------------------------- formatting */
static const char *const FORMAT_NAMES[DU_FMT_COUNT] = {
    "APFS",
    "APFS (Encrypted)",
    "APFS (Case-sensitive)",
    "APFS (Case-sensitive, Encrypted)",
    "Mac OS Extended (Journaled)",
    "Mac OS Extended (Case-sensitive, Journaled)",
    "ExFAT",
    "MS-DOS (FAT)"
};

static const char *const SCHEME_NAMES[DU_SCHEME_COUNT] = {
    "GUID Partition Map",
    "Master Boot Record (MBR)",
    "Apple Partition Map"
};

const char *du_format_name(int i)
{
    if (i < 0 || i >= DU_FMT_COUNT) i = 0;
    return FORMAT_NAMES[i];
}

const char *du_scheme_name(int i)
{
    if (i < 0 || i >= DU_FMT_COUNT) i = 0;
    return SCHEME_NAMES[i];
}

static void fmt_size(char *out, size_t n, float gb)
{
    if (gb < 0.0f) gb = 0.0f;
    if (gb >= 1000.0f)
        snprintf(out, n, "%.2f TB", (double)(gb / 1000.0f));
    else if (gb >= 1.0f)
        snprintf(out, n, "%.2f GB", (double)gb);
    else
        snprintf(out, n, "%.1f MB", (double)(gb * 1024.0f));
}

/* ---------------------------------------------------------------- layout */
typedef struct {
    float w, h;
    float card_x, card_y, card_w, card_h, card_r;
    float band_h;

    float tl_x, tl_cy, tl_pitch, tl_r;
    float bar_cx, bar_cy;
    float lab_top;
    float view_x, view_w, view_y, view_h;
    float title_tx;

    float cx0, cx1;
    float icon_x, icon_y, icon_w, icon_h;
    float title_x, title_top, sub_top;
    float chip_x, chip_y, chip_w, chip_h;
    float sep_y, bar_y, bar_h;
    float leg_y, leg_v_y, leg1_x, leg2_x, leg3_x;
    float grid_y, grid_h;

    int   visible_rows;
    int   row_vol_idx[DU_MAX_VOL];
    float row_y[DU_MAX_VOL];
    float row_x, row_w, row_h;
} DULayout;

static DULayout du_layout(float w, float h, const DUState *s)
{
    DULayout L;
    memset(&L, 0, sizeof(L));
    L.w = w; L.h = h;

    L.card_x = 8.0f;
    L.card_y = 7.0f;
    L.card_w = 265.0f;
    L.card_h = h - 14.0f;
    L.card_r = 12.0f;
    L.band_h = 57.0f;

    L.tl_x = 24.0f; L.tl_cy = 25.0f; L.tl_pitch = 22.0f; L.tl_r = 7.0f;
    L.bar_cx = w - 28.0f; L.bar_cy = 25.0f;
    L.lab_top = 49.0f;

    L.cx0 = s->sidebar ? (L.card_x + L.card_w + 24.0f) : 28.0f;
    L.cx1 = w - 28.0f;

    L.view_x = s->sidebar ? (L.cx0 - 18.0f) : 96.0f;
    L.view_w = 46.0f; L.view_y = L.bar_cy - 18.0f; L.view_h = 36.0f;
    L.title_tx = s->sidebar ? (L.cx0 + 42.0f) : (L.view_x + L.view_w + 18.0f);

    L.row_x = L.card_x + 8.0f;
    L.row_w = L.card_w - 16.0f;
    L.row_h = 28.0f;

    /* Build visible rows */
    L.visible_rows = 0;
    float cur_y = 66.0f;
    for (int i = 0; i < s->count && L.visible_rows < DU_MAX_VOL; i++) {
        const DUVolume *v = &s->vol[i];
        if (s->view_mode == DU_VIEW_VOLUMES_ONLY && v->type != DU_NODE_VOLUME && v->type != DU_NODE_EXTERNAL && v->type != DU_NODE_IMAGE) {
            continue;
        }
        L.row_vol_idx[L.visible_rows] = i;
        L.row_y[L.visible_rows] = cur_y;
        L.visible_rows++;
        cur_y += L.row_h + 3.0f;
    }

    L.icon_x = L.cx0; L.icon_y = 86.0f; L.icon_w = 56.0f; L.icon_h = 60.0f;
    L.title_x = L.cx0 + 72.0f; L.title_top = 92.0f; L.sub_top = 120.0f;
    L.chip_w = 140.0f; L.chip_h = 42.0f;
    L.chip_x = L.cx1 - L.chip_w; L.chip_y = 92.0f;

    L.sep_y = 176.0f;
    L.bar_y = 204.0f; L.bar_h = 22.0f;
    L.leg_y = 238.0f; L.leg_v_y = 260.0f;
    L.leg1_x = L.cx0 + 4.0f;
    L.leg2_x = L.cx0 + (L.cx1 - L.cx0) * 0.38f;
    L.leg3_x = L.cx0 + (L.cx1 - L.cx0) * 0.76f;

    L.grid_y = 296.0f; L.grid_h = 132.0f;

    return L;
}

/* ============================================================ primitives */

static void window_chrome(SUCanvas *c, float w, float h)
{
    su_fill_rrect(c, 0.0f, 0.0f, w, h, 18.0f, C_WINDOW);
    su_stroke_rrect(c, 0.5f, 0.5f, w - 1.0f, h - 1.0f, 18.0f, 1.0f, su_rgba(255, 255, 255, 32));
}

static void traffic_lights(SUCanvas *c, const DULayout *L)
{
    static const int col[3][3] = {{255, 95, 87}, {254, 188, 46}, {40, 200, 64}};
    for (int i = 0; i < 3; i++) {
        float cx = L->tl_x + i * L->tl_pitch;
        su_fill_circle(c, cx, L->tl_cy, L->tl_r, su_rgba(col[i][0], col[i][1], col[i][2], 255));
        su_fill_circle(c, cx, L->tl_cy - 1.5f, L->tl_r - 2.0f, su_rgba((col[i][0] + 255) / 2, (col[i][1] + 255) / 2, (col[i][2] + 255) / 2, 45));
    }
}

static void glass_capsule(SUCanvas *c, float x, float y, float w, float h)
{
    float r = h * 0.5f;
    su_shadow_rrect(c, x, y + 1.0f, w, h, r, 4.0f, 2.0f, su_rgba(0, 0, 0, 80));
    su_fill_rrect(c, x, y, w, h, r, C_GLASS);
    su_stroke_rrect(c, x, y, w, h, r, 1.0f, C_GLASS_EDGE);
}

static void glass_circle(SUCanvas *c, float cx, float cy, float d)
{
    float r = d * 0.5f;
    su_shadow_rrect(c, cx - r, cy - r + 1.0f, d, d, r, 4.0f, 2.0f, su_rgba(0, 0, 0, 80));
    su_fill_circle(c, cx, cy, r, C_GLASS);
    su_stroke_rrect(c, cx - r, cy - r, d, d, r, 1.0f, C_GLASS_EDGE);
}

/* ============================================================ toolbar */

typedef struct {
    const char *label, *symbol;
    float glyph_h;
    float cap_w;
    float pitch;
} ToolItem;

static const ToolItem TOOLS[6] = {
    {"First Aid", "stethoscope",              17.0f, 38.0f, 58.0f},
    {"Partition", "chart.pie",                16.0f, 38.0f, 54.0f},
    {"Erase", "externaldrive.badge.xmark",    15.0f, 38.0f, 52.0f},
    {"Restore", "arrow.counterclockwise",     16.0f, 38.0f, 56.0f},
    {"Unmount", "eject",                      17.0f, 38.0f, 58.0f},
    {"Info", "info.circle",                   16.5f, 38.0f,  0.0f},
};

static void tool_centres(const DULayout *L, float *cx, int n)
{
    cx[n - 1] = L->bar_cx;
    for (int i = n - 2; i >= 0; i--) cx[i] = cx[i + 1] - TOOLS[i].pitch;
}

static void toolbar(SUCanvas *c, const DULayout *L, const DUState *s)
{
    float cx[6];
    tool_centres(L, cx, 6);

    /* View Button */
    glass_capsule(c, L->view_x, L->view_y, L->view_w, L->view_h);
    su_symbol_h(c, "sidebar.leading", L->view_x + 14.0f, L->bar_cy, 13.0f, C_TEXT);
    su_symbol_h(c, "chevron.down", L->view_x + 33.0f, L->bar_cy + 1.0f, 6.0f, C_TEXT2);
    su_text_center(c, SU_SEM11, L->view_x + L->view_w * 0.5f, L->lab_top, "View", C_LABEL);

    /* Window Title */
    su_text_top(c, SU_SEM15, L->title_tx, 19.0f, "Disk Utility", C_TEXT);

    /* Volume segmented control (+ / -) */
    float pill_w = 76.0f;
    float pill_x = (cx[0] - TOOLS[0].cap_w * 0.5f) - 18.0f - pill_w;
    glass_capsule(c, pill_x, L->bar_cy - 18.0f, pill_w, 36.0f);
    su_symbol(c, "plus", pill_x + 21.0f, L->bar_cy, 12.0f, C_TEXT);
    su_symbol(c, "minus", pill_x + 55.0f, L->bar_cy, 12.0f, s->count > 1 ? C_TEXT : C_LABEL);
    su_fill_rect(c, pill_x + 38.0f, L->bar_cy - 9.0f, 1.0f, 18.0f, C_HAIR);
    su_text_center(c, SU_SEM11, pill_x + pill_w * 0.5f, L->lab_top, "Volume", C_LABEL);

    /* Toolbar action circle buttons */
    for (int i = 0; i < 6; i++) {
        glass_circle(c, cx[i], L->bar_cy, TOOLS[i].cap_w);
        const char *sym = TOOLS[i].symbol;
        const char *lbl = TOOLS[i].label;
        if (i == 4) {
            /* Unmount toggle */
            int mounted = (s->selected >= 0 && s->selected < s->count) ? s->vol[s->selected].mounted : 1;
            lbl = mounted ? "Unmount" : "Mount";
        }
        su_symbol_h(c, sym, cx[i], L->bar_cy, TOOLS[i].glyph_h, C_TEXT);
        su_text_center(c, SU_SEM11, cx[i], L->lab_top, lbl, C_LABEL);
    }
}

/* ============================================================ sidebar */

static void sidebar(SUCanvas *c, const DULayout *L, const DUState *s)
{
    float r = L->card_r;
    su_fill_rrect(c, L->card_x, L->card_y, L->card_w, L->card_h, r, C_CARD);
    su_stroke_rrect(c, L->card_x, L->card_y, L->card_w, L->card_h, r, 1.0f, su_rgba(255, 255, 255, 20));

    /* Header text */
    su_text_top(c, SU_SEM11, L->card_x + 14.0f, 48.0f, "Storage Devices", C_HEADER);

    /* Render hierarchical items */
    for (int i = 0; i < L->visible_rows; i++) {
        int v_idx = L->row_vol_idx[i];
        const DUVolume *v = &s->vol[v_idx];
        float ry = L->row_y[i];
        int sel = (v_idx == s->selected);

        if (sel) {
            su_fill_rrect(c, L->row_x, ry, L->row_w, L->row_h, 7.0f, C_ACCENT_BG);
        }

        float ind_x = L->row_x + 8.0f + (float)v->indent * 14.0f;

        /* Chevron if has children */
        if (v->has_children) {
            su_symbol_h(c, "chevron.down", ind_x + 2.0f, ry + L->row_h * 0.5f, 7.0f, sel ? C_TEXT : C_TEXT2);
        }

        /* Symbol */
        const char *sym = "internaldrive";
        if (v->type == DU_NODE_VOLUME) sym = "internaldrive.fill";
        else if (v->type == DU_NODE_EXTERNAL) sym = "externaldrive";
        else if (v->type == DU_NODE_IMAGE) sym = "externaldrive.fill";

        float sym_x = ind_x + (v->has_children ? 18.0f : 6.0f);
        su_symbol_h(c, sym, sym_x, ry + L->row_h * 0.5f, 13.0f, sel ? su_rgba(255, 255, 255, 255) : C_TEXT);

        /* Label */
        float txt_x = sym_x + 14.0f;
        uint32_t fg = sel ? su_rgba(255, 255, 255, 255) : C_TEXT;
        float max_w = (L->row_x + L->row_w - (v->type == DU_NODE_EXTERNAL ? 28.0f : 10.0f)) - txt_x;
        su_text_clipped(c, SU_MED13, txt_x, ry + 6.0f, max_w, v->name, fg);

        if (v->type == DU_NODE_EXTERNAL) {
            su_symbol_h(c, "eject.fill", L->row_x + L->row_w - 14.0f, ry + L->row_h * 0.5f, 11.0f, sel ? su_rgba(255, 255, 255, 255) : C_TEXT2);
        }

        /* Eject button for external */
        if (v->type == DU_NODE_EXTERNAL) {
            su_symbol_h(c, "eject.fill", L->row_x + L->row_w - 14.0f, ry + L->row_h * 0.5f, 9.0f, C_TEXT2);
        }
    }
}

/* ============================================================ detail content */

static void capacity_chip(SUCanvas *c, float x, float y, float w, float h, const char *size_str, int shared)
{
    su_fill_rrect(c, x, y, w, h, 8.0f, C_BOX);
    su_stroke_rrect(c, x, y, w, h, 8.0f, 1.0f, C_BORDER);
    su_text_center(c, SU_MED17, x + w * 0.5f, y + 6.0f, size_str, C_VALUE);
    char sub[40];
    if (shared > 1) snprintf(sub, sizeof sub, "SHARED BY %d VOLUMES", shared);
    else snprintf(sub, sizeof sub, "ONE VOLUME");
    su_text_center(c, SU_SEM11, x + w * 0.5f, y + 25.0f, sub, C_LABEL);
}

static void capacity_bar(SUCanvas *c, float x, float y, float w, float h, float used_frac, float other_frac)
{
    if (used_frac < 0.0f) used_frac = 0.0f;
    if (used_frac > 1.0f) used_frac = 1.0f;
    if (other_frac < 0.0f) other_frac = 0.0f;
    if (used_frac + other_frac > 1.0f) other_frac = 1.0f - used_frac;

    su_fill_rrect(c, x, y, w, h, 6.0f, su_rgba(255, 255, 255, 12));
    su_stroke_rrect(c, x, y, w, h, 6.0f, 1.0f, su_rgba(255, 255, 255, 60));

    float inner_w = w - 2.0f;
    float used_w = inner_w * used_frac;
    float other_w = inner_w * other_frac;

    if (used_w >= 2.0f) {
        su_fill_rrect(c, x + 1.0f, y + 1.0f, used_w, h - 2.0f, 5.0f, su_rgba(10, 132, 255, 255));
    }
    if (other_w >= 2.0f) {
        su_fill_rrect(c, x + 1.0f + used_w, y + 1.0f, other_w, h - 2.0f, 5.0f, su_rgba(100, 100, 105, 255));
    }
}

static void legend_dot(SUCanvas *c, float x, float capTop, float valTop, const char *label, const char *value, uint32_t dot_color, int outline)
{
    float cy = capTop + 5.0f;
    if (outline) {
        su_fill_circle(c, x, cy, 4.5f, su_rgba(220, 220, 220, 255));
        su_fill_circle(c, x, cy, 3.0f, C_WINDOW);
    } else {
        su_fill_circle(c, x, cy, 4.5f, dot_color);
    }
    su_text_top(c, SU_SEM13, x + 12.0f, capTop, label, C_VALUE);
    su_text_top(c, SU_REG12, x + 12.0f, valTop, value, C_TEXT2);
}

typedef struct { const char *k, *v; } GridItem;

static void detail_grid(SUCanvas *c, float x, float y, float w, float h, const GridItem *left, const GridItem *right, int rows)
{
    float div = x + w * 0.5f;
    float pad = 12.0f;
    su_fill_rrect(c, x, y, w, h, 8.0f, C_GRID);
    su_stroke_rrect(c, x, y, w, h, 8.0f, 1.0f, C_BORDER);
    su_fill_rect(c, div, y + 8.0f, 1.0f, h - 16.0f, C_HAIR);

    for (int i = 0; i < rows; i++) {
        float row_y = y + 10.0f + i * 29.0f;
        if (i > 0) {
            su_fill_rect(c, x + 10.0f, row_y - 2.0f, div - x - 20.0f, 1.0f, C_HAIR);
            su_fill_rect(c, div + 10.0f, row_y - 2.0f, x + w - div - 20.0f, 1.0f, C_HAIR);
        }
        su_text_top(c, SU_SEM12, x + pad, row_y, left[i].k, C_LABEL);
        su_text_right(c, SU_SEM12, div - pad, row_y, left[i].v, C_VALUE);
        su_text_top(c, SU_SEM12, div + pad, row_y, right[i].k, C_LABEL);
        su_text_right(c, SU_SEM12, x + w - pad, row_y, right[i].v, C_VALUE);
    }
}

/* ============================================================ modal sheets */

static void sheet_field(SUCanvas *c, float x, float y, float w, float h, const char *val, const char *sym, int focused, int caret)
{
    uint32_t edge = focused ? C_ACCENT : C_BORDER;
    su_fill_rrect(c, x, y, w, h, 6.0f, su_rgba(255, 255, 255, 14));
    su_stroke_rrect(c, x, y, w, h, 6.0f, focused ? 2.0f : 1.0f, edge);
    su_text_mid(c, SU_REG13, x + 10.0f, y + h * 0.5f, val, C_VALUE);
    if (focused && caret) {
        float tw = su_text_width(SU_REG13, val);
        su_fill_rect(c, x + 11.0f + tw, y + 5.0f, 1.5f, h - 10.0f, C_ACCENT);
    }
    if (sym) {
        su_symbol_h(c, sym, x + w - 14.0f, y + h * 0.5f, 9.0f, C_TEXT2);
    }
}

/* --- Sheet 1: Erase (Reference 1) --- */
static void render_erase_sheet(SUCanvas *c, const DULayout *L, const DUState *s)
{
    float w = 480.0f, h = 260.0f;
    float x = (L->w - w) * 0.5f, y = (L->h - h) * 0.5f;
    const DUVolume *v = &s->vol[s->selected];

    su_shadow_rrect(c, x, y, w, h, 14.0f, 24.0f, 8.0f, su_rgba(0, 0, 0, 200));
    su_fill_rrect(c, x, y, w, h, 14.0f, su_gray(48, 255));
    su_stroke_rrect(c, x, y, w, h, 14.0f, 1.0f, su_rgba(255, 255, 255, 40));

    su_symbol_rect(c, "apple_internal_disk", x + 24.0f, y + 24.0f, 48.0f, 52.0f, 0);

    char title[96];
    snprintf(title, sizeof title, "Erase \xe2\x80\x9c%s\xe2\x80\x9d?", v->name);
    su_text_top(c, SU_SEM15, x + 86.0f, y + 24.0f, title, C_TEXT);
    su_text_top(c, SU_REG12, x + 86.0f, y + 48.0f, "Erasing this volume will permanently erase all data stored on it.", C_TEXT2);
    su_text_top(c, SU_REG12, x + 86.0f, y + 64.0f, "You can\xe2\x80\x99t undo this action.", C_TEXT2);

    /* Form Rows */
    float fy = y + 104.0f;
    su_text_mid(c, SU_SEM13, x + 24.0f, fy + 14.0f, "Name:", C_LABEL);
    sheet_field(c, x + 86.0f, fy, w - 110.0f, 28.0f, s->edit_name[0] ? s->edit_name : "Untitled", NULL, s->field == 0, s->caret);

    fy += 38.0f;
    su_text_mid(c, SU_SEM13, x + 24.0f, fy + 14.0f, "Format:", C_LABEL);
    sheet_field(c, x + 86.0f, fy, w - 110.0f, 28.0f, du_format_name(s->fmt), "chevron.up.chevron.down", s->field == 1, 0);

    /* Buttons */
    float by = y + h - 44.0f;
    float bw = 88.0f, bh = 28.0f;
    /* Cancel */
    su_fill_rrect(c, x + w - bw * 2.0f - 24.0f, by, bw, bh, 7.0f, su_rgba(255, 255, 255, 24));
    su_stroke_rrect(c, x + w - bw * 2.0f - 24.0f, by, bw, bh, 7.0f, 1.0f, C_GLASS_EDGE);
    su_text_center(c, SU_REG13, x + w - bw * 1.5f - 24.0f, by + 6.0f, "Cancel", C_TEXT);

    /* Erase */
    su_fill_rrect(c, x + w - bw - 14.0f, by, bw, bh, 7.0f, C_ACCENT);
    su_text_center(c, SU_SEM13, x + w - bw * 0.5f - 14.0f, by + 6.0f, "Erase", su_rgba(255, 255, 255, 255));
}

/* --- Sheet 2: First Aid (Reference 2) --- */
static void render_first_aid_sheet(SUCanvas *c, const DULayout *L, const DUState *s)
{
    float w = 440.0f, h = 220.0f;
    float x = (L->w - w) * 0.5f, y = (L->h - h) * 0.5f;
    const DUVolume *v = &s->vol[s->selected];

    su_shadow_rrect(c, x, y, w, h, 14.0f, 24.0f, 8.0f, su_rgba(0, 0, 0, 200));
    su_fill_rrect(c, x, y, w, h, 14.0f, su_gray(48, 255));
    su_stroke_rrect(c, x, y, w, h, 14.0f, 1.0f, su_rgba(255, 255, 255, 40));

    su_symbol_rect(c, "apple_internal_disk", x + 24.0f, y + 24.0f, 48.0f, 52.0f, 0);

    char title[96];
    snprintf(title, sizeof title, "Run First Aid on \xe2\x80\x9c%s\xe2\x80\x9d?", v->name);
    su_text_top(c, SU_SEM15, x + 86.0f, y + 24.0f, title, C_TEXT);
    su_text_top(c, SU_REG12, x + 86.0f, y + 50.0f, "First Aid will check the volume for errors.", C_TEXT2);
    su_text_top(c, SU_REG12, x + 86.0f, y + 68.0f, "It will then repair the volume if necessary.", C_TEXT2);

    /* Buttons */
    float by = y + h - 46.0f;
    float bw = 88.0f, bh = 28.0f;
    su_fill_rrect(c, x + w - bw * 2.0f - 24.0f, by, bw, bh, 7.0f, su_rgba(255, 255, 255, 24));
    su_stroke_rrect(c, x + w - bw * 2.0f - 24.0f, by, bw, bh, 7.0f, 1.0f, C_GLASS_EDGE);
    su_text_center(c, SU_REG13, x + w - bw * 1.5f - 24.0f, by + 6.0f, "Cancel", C_TEXT);

    su_fill_rrect(c, x + w - bw - 14.0f, by, bw, bh, 7.0f, C_ACCENT);
    su_text_center(c, SU_SEM13, x + w - bw * 0.5f - 14.0f, by + 6.0f, "Run", su_rgba(255, 255, 255, 255));
}

/* --- Sheet 3: Partition with Pie Chart (Reference 5) --- */
static void render_partition_sheet(SUCanvas *c, const DULayout *L, const DUState *s)
{
    float w = 720.0f, h = 480.0f;
    float x = (L->w - w) * 0.5f, y = (L->h - h) * 0.5f;
    const DUVolume *v = &s->vol[s->selected];

    su_shadow_rrect(c, x, y, w, h, 14.0f, 28.0f, 10.0f, su_rgba(0, 0, 0, 220));
    su_fill_rrect(c, x, y, w, h, 14.0f, su_gray(46, 255));
    su_stroke_rrect(c, x, y, w, h, 14.0f, 1.0f, su_rgba(255, 255, 255, 45));

    su_symbol_rect(c, "apple_internal_disk", x + 24.0f, y + 20.0f, 44.0f, 48.0f, 0);

    char title[96];
    snprintf(title, sizeof title, "Partition device \xe2\x80\x9c%s\xe2\x80\x9d?", v->name);
    su_text_top(c, SU_SEM15, x + 80.0f, y + 20.0f, title, C_TEXT);
    su_text_top(c, SU_REG12, x + 80.0f, y + 42.0f, "Click add (+) to increase the number of partitions on the device. Click on each", C_TEXT2);
    su_text_top(c, SU_REG12, x + 80.0f, y + 58.0f, "partition to change name/format. Drag the resize control and click Apply.", C_TEXT2);

    /* Left Pane: Interactive Pie Chart */
    float pc_cx = x + 180.0f, pc_cy = y + 240.0f, pc_r = 110.0f;
    /* Pie slices */
    float a0 = -1.5707963f; /* start from top */
    float total_gb = 0.0f;
    for (int si = 0; si < s->slice_count; si++) total_gb += s->slices[si].size_gb;
    if (total_gb <= 0.0f) total_gb = 64.0f;

    for (int si = 0; si < s->slice_count; si++) {
        float span = (s->slices[si].size_gb / total_gb) * 6.2831853f;
        float a1 = a0 + span;
        uint32_t col = (si == s->selected_slice) ? C_ACCENT : su_rgba(180, 195, 220, 255);
        su_fill_pie_slice(c, pc_cx, pc_cy, 0.0f, pc_r, a0, a1, col);

        /* Divider radial line */
        float rx = pc_cx + cosf(a1) * pc_r;
        float ry = pc_cy + sinf(a1) * pc_r;
        su_fill_circle(c, rx, ry, 4.0f, su_rgba(255, 255, 255, 255));

        /* Label inside slice */
        float mid_a = a0 + span * 0.5f;
        float lx = pc_cx + cosf(mid_a) * (pc_r * 0.55f);
        float ly = pc_cy + sinf(mid_a) * (pc_r * 0.55f);
        su_text_center(c, SU_SEM11, lx, ly - 6.0f, s->slices[si].name, su_rgba(255, 255, 255, 255));

        a0 = a1;
    }
    /* Outer circle ring */
    su_stroke_rrect(c, pc_cx - pc_r, pc_cy - pc_r, pc_r * 2.0f, pc_r * 2.0f, pc_r, 1.5f, su_rgba(255, 255, 255, 80));

    /* +/- segmented control under pie chart */
    float pm_x = pc_cx - 36.0f, pm_y = pc_cy + pc_r + 18.0f;
    glass_capsule(c, pm_x, pm_y, 72.0f, 26.0f);
    su_symbol(c, "plus", pm_x + 18.0f, pm_y + 13.0f, 10.0f, C_TEXT);
    su_symbol(c, "minus", pm_x + 54.0f, pm_y + 13.0f, 10.0f, s->slice_count > 1 ? C_TEXT : C_LABEL);
    su_fill_rect(c, pm_x + 36.0f, pm_y + 4.0f, 1.0f, 18.0f, C_HAIR);

    /* Right Pane: Device & Partition Information */
    float ix = x + 350.0f;
    su_text_top(c, SU_SEM13, ix, y + 100.0f, "Device Information", C_TEXT);
    su_text_top(c, SU_SEM12, ix, y + 124.0f, "Device:", C_LABEL);
    su_text_top(c, SU_REG12, ix + 80.0f, y + 124.0f, v->name, C_VALUE);
    su_text_top(c, SU_SEM12, ix, y + 144.0f, "Scheme:", C_LABEL);
    su_text_top(c, SU_REG12, ix + 80.0f, y + 144.0f, "GUID Partition Map", C_VALUE);
    su_text_top(c, SU_SEM12, ix, y + 164.0f, "Size:", C_LABEL);
    char cap_str[32]; fmt_size(cap_str, sizeof cap_str, v->cap_gb);
    su_text_top(c, SU_REG12, ix + 80.0f, y + 164.0f, cap_str, C_VALUE);

    su_text_top(c, SU_SEM13, ix, y + 204.0f, "Partition Information", C_TEXT);
    su_text_mid(c, SU_SEM12, ix, y + 242.0f, "Name:", C_LABEL);
    sheet_field(c, ix + 80.0f, y + 228.0f, 240.0f, 28.0f, s->edit_name[0] ? s->edit_name : "Media", NULL, s->field == 0, s->caret);

    su_text_mid(c, SU_SEM12, ix, y + 282.0f, "Format:", C_LABEL);
    sheet_field(c, ix + 80.0f, y + 268.0f, 240.0f, 28.0f, du_format_name(s->fmt), "chevron.up.chevron.down", s->field == 1, 0);

    su_text_mid(c, SU_SEM12, ix, y + 322.0f, "Size:", C_LABEL);
    sheet_field(c, ix + 80.0f, y + 308.0f, 150.0f, 28.0f, s->edit_size[0] ? s->edit_size : "24", NULL, s->field == 2, 0);
    glass_capsule(c, ix + 240.0f, y + 308.0f, 80.0f, 28.0f);
    su_text_center(c, SU_REG12, ix + 276.0f, y + 314.0f, "GB", C_TEXT);
    su_symbol_h(c, "chevron.up.chevron.down", ix + 308.0f, y + 322.0f, 8.0f, C_TEXT2);

    su_text_top(c, SU_REG11, ix + 80.0f, y + 344.0f, "This partition will be created.", C_LABEL);

    /* Bottom Bar */
    float by = y + h - 46.0f;
    float bw = 88.0f, bh = 28.0f;
    /* Help */
    glass_circle(c, x + 34.0f, by + bh * 0.5f, bh);
    su_text_center(c, SU_SEM13, x + 34.0f, by + 6.0f, "?", C_TEXT);

    /* Revert */
    su_fill_rrect(c, x + 64.0f, by, bw, bh, 7.0f, su_rgba(255, 255, 255, 24));
    su_stroke_rrect(c, x + 64.0f, by, bw, bh, 7.0f, 1.0f, C_GLASS_EDGE);
    su_text_center(c, SU_REG13, x + 64.0f + bw * 0.5f, by + 6.0f, "Revert", C_TEXT);

    /* Cancel & Apply */
    su_fill_rrect(c, x + w - bw * 2.0f - 24.0f, by, bw, bh, 7.0f, su_rgba(255, 255, 255, 24));
    su_stroke_rrect(c, x + w - bw * 2.0f - 24.0f, by, bw, bh, 7.0f, 1.0f, C_GLASS_EDGE);
    su_text_center(c, SU_REG13, x + w - bw * 1.5f - 24.0f, by + 6.0f, "Cancel", C_TEXT);

    su_fill_rrect(c, x + w - bw - 14.0f, by, bw, bh, 7.0f, C_ACCENT);
    su_text_center(c, SU_SEM13, x + w - bw * 0.5f - 14.0f, by + 6.0f, "Apply", su_rgba(255, 255, 255, 255));
}

/* --- Sheet 4: Alert Sheet (Reference 4) --- */
static void render_alert_sheet(SUCanvas *c, const DULayout *L, const DUState *s)
{
    float w = 380.0f, h = 210.0f;
    float x = (L->w - w) * 0.5f, y = (L->h - h) * 0.5f;

    su_shadow_rrect(c, x, y, w, h, 14.0f, 24.0f, 8.0f, su_rgba(0, 0, 0, 200));
    su_fill_rrect(c, x, y, w, h, 14.0f, su_gray(48, 255));
    su_stroke_rrect(c, x, y, w, h, 14.0f, 1.0f, su_rgba(255, 255, 255, 40));

    /* Warning Triangle Badge */
    float badge_cx = x + w * 0.5f, badge_cy = y + 42.0f;
    su_fill_circle(c, badge_cx, badge_cy, 18.0f, su_rgba(255, 180, 0, 255));
    su_text_center(c, SU_MED17, badge_cx, badge_cy - 10.0f, "!", su_rgba(0, 0, 0, 255));

    su_text_center(c, SU_SEM13, x + w * 0.5f, y + 80.0f, s->alert_title[0] ? s->alert_title : "Could not mount volume.", C_TEXT);
    su_text_center(c, SU_REG12, x + w * 0.5f, y + 106.0f, s->alert_msg[0] ? s->alert_msg : "(com.apple.DiskManagement error 53 249)", C_TEXT2);

    /* OK Button */
    float by = y + h - 46.0f;
    float bw = 240.0f, bh = 28.0f;
    float bx = x + (w - bw) * 0.5f;
    su_fill_rrect(c, bx, by, bw, bh, 7.0f, C_ACCENT);
    su_text_center(c, SU_SEM13, bx + bw * 0.5f, by + 6.0f, "OK", su_rgba(255, 255, 255, 255));
}

/* --- View Menu Popover --- */
static void view_menu(SUCanvas *c, const DULayout *L, const DUState *s)
{
    static const char *items[3] = {"Show Sidebar", "Show All Devices", "Show Only Volumes"};
    float x = L->view_x, y = L->view_y + L->view_h + 6.0f;
    float w = 180.0f, ih = 26.0f;

    su_shadow_rrect(c, x, y, w, ih * 3.0f + 8.0f, 8.0f, 14.0f, 5.0f, su_rgba(0, 0, 0, 180));
    su_fill_rrect(c, x, y, w, ih * 3.0f + 8.0f, 8.0f, C_POPOVER);
    su_stroke_rrect(c, x, y, w, ih * 3.0f + 8.0f, 8.0f, 1.0f, su_rgba(255, 255, 255, 45));

    for (int i = 0; i < 3; i++) {
        float iy = y + 4.0f + i * ih;
        int checked = 0;
        if (i == 0 && s->sidebar) checked = 1;
        if (i == 1 && s->view_mode == DU_VIEW_ALL_DEVICES) checked = 1;
        if (i == 2 && s->view_mode == DU_VIEW_VOLUMES_ONLY) checked = 1;

        if (checked) {
            su_text_mid(c, SU_SEM12, x + 12.0f, iy + ih * 0.5f, "\xe2\x9c\x93", C_TEXT);
        }
        su_text_mid(c, SU_REG13, x + 28.0f, iy + ih * 0.5f, items[i], C_TEXT);
    }
}

/* ============================================================ main body */

void du_render_state(SUCanvas *c, const DUState *s)
{
    DULayout L = du_layout((float)c->w, (float)c->h, s);
    if (s->count <= 0) return;

    const DUVolume *v = (s->selected >= 0 && s->selected < s->count) ? &s->vol[s->selected] : &s->vol[0];

    /* Clear to transparent so squircle corners have zero black artifacts */
    su_fill(c, 0x00000000u);
    window_chrome(c, L.w, L.h);

    if (s->sidebar) {
        sidebar(c, &L, s);
    }
    traffic_lights(c, &L);
    toolbar(c, &L, s);

    /* Content Pane */
    su_set_clip(c, s->sidebar ? (L.card_x + L.card_w) : 0, 0, s->sidebar ? (L.w - L.card_x - L.card_w) : L.w, L.h);

    char cap_str[32], used_str[32], free_str[32], avail_str[32];
    fmt_size(cap_str, sizeof cap_str, v->cap_gb);
    fmt_size(used_str, sizeof used_str, v->used_gb);
    float free_gb = v->cap_gb - v->used_gb - v->other_gb;
    if (free_gb < 0.0f) free_gb = 0.0f;
    fmt_size(free_str, sizeof free_str, free_gb);
    fmt_size(avail_str, sizeof avail_str, free_gb);

    su_symbol_rect(c, "apple_internal_disk", L.icon_x, L.icon_y, L.icon_w, L.icon_h, 0);
    su_text_clipped(c, SU_BOLD26, L.title_x, L.title_top, (L.chip_x - 16.0f) - L.title_x, v->name, C_TEXT);

    char sub[128];
    snprintf(sub, sizeof sub, "%s \xc2\xb7 %s", v->kind, v->fs);
    su_text_clipped(c, SU_MED13, L.title_x, L.sub_top, (L.chip_x - 16.0f) - L.title_x, sub, C_TEXT2);

    capacity_chip(c, L.chip_x, L.chip_y, L.chip_w, L.chip_h, cap_str, v->shared_vols);

    su_fill_rect(c, L.cx0, L.sep_y, L.cx1 - L.cx0, 1.0f, C_HAIR);

    float used_ratio = (v->cap_gb > 0.0f) ? (v->used_gb / v->cap_gb) : 0.0f;
    float other_ratio = (v->cap_gb > 0.0f) ? (v->other_gb / v->cap_gb) : 0.0f;
    capacity_bar(c, L.cx0, L.bar_y, L.cx1 - L.cx0, L.bar_h, used_ratio, other_ratio);

    legend_dot(c, L.leg1_x, L.leg_y, L.leg_v_y, "Used", used_str, su_rgba(10, 132, 255, 255), 0);
    char other_str[32];
    if (v->other_gb > 0.0f) fmt_size(other_str, sizeof other_str, v->other_gb);
    else snprintf(other_str, sizeof other_str, "--");
    legend_dot(c, L.leg2_x, L.leg_y, L.leg_v_y, "Other Volumes", other_str, su_rgba(100, 100, 105, 255), 0);
    legend_dot(c, L.leg3_x, L.leg_y, L.leg_v_y, "Free", free_str, 0, 1);

    /* Detail Grid */
    GridItem left[6], right[6];
    left[0].k = "Mount Point:"; left[0].v = v->mounted ? v->mount : "Not Mounted";
    left[1].k = "Capacity:";    left[1].v = cap_str;
    left[2].k = "Available:";   left[2].v = avail_str;
    left[3].k = "Used:";        left[3].v = used_str;
    right[0].k = "Type:";       right[0].v = v->kind;
    right[1].k = "Owners:";     right[1].v = v->owners[0] ? v->owners : "Disabled";
    right[2].k = "Connection:"; right[2].v = v->connection[0] ? v->connection : "Apple Fabric";
    right[3].k = "Device:";     right[3].v = v->device;

    int rows = 4;
    if (s->info) {
        left[4].k = "Snapshot Name:"; left[4].v = v->snapshot_name[0] ? v->snapshot_name : "None";
        left[5].k = "S.M.A.R.T. Status:"; left[5].v = "Verified";
        right[4].k = "Snapshot UUID:"; right[4].v = v->snapshot_uuid[0] ? v->snapshot_uuid : "None";
        right[5].k = "Format:"; right[5].v = v->fs;
        rows = 6;
    }
    float gh = L.grid_h + (float)(rows - 4) * 29.0f;
    detail_grid(c, L.cx0, L.grid_y, L.cx1 - L.cx0, gh, left, right, rows);

    /* Status banner */
    if (s->status_str[0] && s->status != DU_STATUS_NONE) {
        su_text_top(c, SU_MED12, L.cx0, L.grid_y + gh + 14.0f, s->status_str, C_TEXT2);
    }
    su_reset_clip(c);

    /* View Menu Popover */
    if (s->view_menu) {
        view_menu(c, &L, s);
    }

    /* Modal Sheets */
    if (s->sheet != DU_SHEET_NONE) {
        su_fill_rrect(c, 0.0f, 0.0f, L.w, L.h, 18.0f, su_rgba(0, 0, 0, 110)); /* modal scrim constrained to window squircle */
        if (s->sheet == DU_SHEET_ERASE || s->sheet == DU_SHEET_NEW_VOLUME) {
            render_erase_sheet(c, &L, s);
        } else if (s->sheet == DU_SHEET_FIRST_AID) {
            render_first_aid_sheet(c, &L, s);
        } else if (s->sheet == DU_SHEET_PARTITION) {
            render_partition_sheet(c, &L, s);
        } else if (s->sheet == DU_SHEET_ALERT) {
            render_alert_sheet(c, &L, s);
        }
    }
}

void du_render_state_buf(uint32_t *px, int w, int h, int stride, const DUState *s)
{
    SUCanvas c;
    su_canvas_bind(&c, px, w, h, stride);
    du_render_state(&c, s);
}

/* ============================================================ hit testing */

static int in_rect(float x, float y, float rx, float ry, float rw, float rh)
{
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

int du_hit_state(const DUState *s, int x, int y, int w, int h)
{
    DULayout L = du_layout((float)w, (float)h, s);
    float fx = (float)x, fy = (float)y;

    if (s->sheet != DU_SHEET_NONE) {
        if (s->sheet == DU_SHEET_ERASE || s->sheet == DU_SHEET_NEW_VOLUME) {
            float sw = 480.0f, sh = 260.0f;
            float sx = (L.w - sw) * 0.5f, sy = (L.h - sh) * 0.5f;
            float by = sy + sh - 44.0f, bw = 88.0f, bh = 28.0f;
            if (in_rect(fx, fy, sx + sw - bw * 2.0f - 24.0f, by, bw, bh)) return DU_HIT_SHEET_CANCEL;
            if (in_rect(fx, fy, sx + sw - bw - 14.0f, by, bw, bh)) return DU_HIT_SHEET_OK;
            if (in_rect(fx, fy, sx + 86.0f, sy + 104.0f, sw - 110.0f, 28.0f)) return DU_HIT_SHEET_FIELD_0;
            if (in_rect(fx, fy, sx + 86.0f, sy + 142.0f, sw - 110.0f, 28.0f)) return DU_HIT_SHEET_FIELD_1;
            return DU_HIT_NONE;
        } else if (s->sheet == DU_SHEET_FIRST_AID) {
            float sw = 440.0f, sh = 220.0f;
            float sx = (L.w - sw) * 0.5f, sy = (L.h - sh) * 0.5f;
            float by = sy + sh - 46.0f, bw = 88.0f, bh = 28.0f;
            if (in_rect(fx, fy, sx + sw - bw * 2.0f - 24.0f, by, bw, bh)) return DU_HIT_SHEET_CANCEL;
            if (in_rect(fx, fy, sx + sw - bw - 14.0f, by, bw, bh)) return DU_HIT_SHEET_OK;
            return DU_HIT_NONE;
        } else if (s->sheet == DU_SHEET_PARTITION) {
            float sw = 720.0f, sh = 480.0f;
            float sx = (L.w - sw) * 0.5f, sy = (L.h - sh) * 0.5f;
            float by = sy + sh - 46.0f, bw = 88.0f, bh = 28.0f;
            if (in_rect(fx, fy, sx + 64.0f, by, bw, bh)) return DU_HIT_SHEET_REVERT;
            if (in_rect(fx, fy, sx + sw - bw * 2.0f - 24.0f, by, bw, bh)) return DU_HIT_SHEET_CANCEL;
            if (in_rect(fx, fy, sx + sw - bw - 14.0f, by, bw, bh)) return DU_HIT_SHEET_OK;
            /* +/- under pie */
            float pc_cx = sx + 180.0f, pc_cy = sy + 240.0f, pc_r = 110.0f;
            float pm_x = pc_cx - 36.0f, pm_y = pc_cy + pc_r + 18.0f;
            if (in_rect(fx, fy, pm_x, pm_y, 36.0f, 26.0f)) return DU_HIT_SHEET_SLICE_PLUS;
            if (in_rect(fx, fy, pm_x + 36.0f, pm_y, 36.0f, 26.0f)) return DU_HIT_SHEET_SLICE_MINUS;
            float ix = sx + 350.0f;
            if (in_rect(fx, fy, ix + 80.0f, sy + 228.0f, 240.0f, 28.0f)) return DU_HIT_SHEET_FIELD_0;
            if (in_rect(fx, fy, ix + 80.0f, sy + 268.0f, 240.0f, 28.0f)) return DU_HIT_SHEET_FIELD_1;
            if (in_rect(fx, fy, ix + 80.0f, sy + 308.0f, 150.0f, 28.0f)) return DU_HIT_SHEET_FIELD_2;
            return DU_HIT_NONE;
        } else if (s->sheet == DU_SHEET_ALERT) {
            float sw = 380.0f, sh = 210.0f;
            float sx = (L.w - sw) * 0.5f, sy = (L.h - sh) * 0.5f;
            float by = sy + sh - 46.0f, bw = 240.0f, bh = 28.0f;
            float bx = sx + (sw - bw) * 0.5f;
            if (in_rect(fx, fy, bx, by, bw, bh)) return DU_HIT_SHEET_OK;
            return DU_HIT_NONE;
        }
    }

    if (s->view_menu) {
        float mx = L.view_x, my = L.view_y + L.view_h + 6.0f, mw = 180.0f, ih = 26.0f;
        if (in_rect(fx, fy, mx, my + 4.0f, mw, ih)) return DU_HIT_VIEWMENU_SIDEBAR;
        if (in_rect(fx, fy, mx, my + 4.0f + ih, mw, ih)) return DU_HIT_VIEWMENU_ALL;
        if (in_rect(fx, fy, mx, my + 4.0f + ih * 2.0f, mw, ih)) return DU_HIT_VIEWMENU_VOLUMES;
        return DU_HIT_NONE;
    }

    /* Close Traffic Light */
    float dtl_x = fx - L.tl_x, dtl_y = fy - L.tl_cy;
    if (dtl_x * dtl_x + dtl_y * dtl_y <= 81.0f) return DU_HIT_CLOSE;

    /* Sidebar rows */
    if (s->sidebar) {
        for (int i = 0; i < L.visible_rows; i++) {
            if (in_rect(fx, fy, L.row_x, L.row_y[i], L.row_w, L.row_h)) {
                return DU_HIT_ROW_FIRST + L.row_vol_idx[i];
            }
        }
    }

    /* Toolbar buttons */
    float cx[6];
    tool_centres(&L, cx, 6);
    for (int i = 0; i < 6; i++) {
        float t_dx = fx - cx[i], t_dy = fy - L.bar_cy;
        if (t_dx * t_dx + t_dy * t_dy <= 400.0f) return DU_HIT_TOOL_FIRST + i;
    }

    /* View Button */
    if (in_rect(fx, fy, L.view_x, L.view_y, L.view_w, L.view_h)) return DU_HIT_VIEW;

    /* Volume + / - */
    float pill_w = 76.0f;
    float pill_x = (cx[0] - TOOLS[0].cap_w * 0.5f) - 18.0f - pill_w;
    if (in_rect(fx, fy, pill_x, L.bar_cy - 18.0f, 38.0f, 36.0f)) return DU_HIT_PM_PLUS;
    if (in_rect(fx, fy, pill_x + 38.0f, L.bar_cy - 18.0f, 38.0f, 36.0f)) return DU_HIT_PM_MINUS;

    if (fy < 57.0f) return DU_HIT_TITLEBAR;

    return DU_HIT_NONE;
}

/* ============================================================ actions & events */

int du_click_state(DUState *s, int x, int y, int w, int h)
{
    int hit = du_hit_state(s, x, y, w, h);
    if (hit == DU_HIT_NONE) {
        if (s->view_menu) s->view_menu = 0;
        return DU_ACT_NONE;
    }

    if (hit == DU_HIT_CLOSE) return DU_ACT_CLOSE;
    if (hit == DU_HIT_TITLEBAR) return DU_ACT_DRAG;

    if (hit >= DU_HIT_ROW_FIRST && hit <= DU_HIT_ROW_LAST) {
        int idx = hit - DU_HIT_ROW_FIRST;
        if (idx >= 0 && idx < s->count) {
            s->selected = idx;
        }
        return DU_ACT_NONE;
    }

    if (hit == DU_HIT_VIEW) {
        s->view_menu = !s->view_menu;
        return DU_ACT_NONE;
    }

    if (hit == DU_HIT_VIEWMENU_SIDEBAR) {
        s->sidebar = !s->sidebar;
        s->view_menu = 0;
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_VIEWMENU_ALL) {
        s->view_mode = DU_VIEW_ALL_DEVICES;
        s->view_menu = 0;
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_VIEWMENU_VOLUMES) {
        s->view_mode = DU_VIEW_VOLUMES_ONLY;
        s->view_menu = 0;
        return DU_ACT_NONE;
    }

    /* Toolbar Tools */
    if (hit == DU_HIT_TOOL_FA) {
        s->sheet = DU_SHEET_FIRST_AID;
        s->fa_stage = 0;
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_TOOL_PARTITION) {
        s->sheet = DU_SHEET_PARTITION;
        s->field = 0;
        snprintf(s->edit_name, sizeof s->edit_name, "Media");
        snprintf(s->edit_size, sizeof s->edit_size, "24");
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_TOOL_ERASE) {
        s->sheet = DU_SHEET_ERASE;
        s->field = 0;
        strncpy(s->edit_name, s->vol[s->selected].name, sizeof(s->edit_name) - 1); s->edit_name[sizeof(s->edit_name) - 1] = 0;
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_TOOL_RESTORE) {
        s->status = DU_STATUS_MOUNTED;
        s->status_ttl = DU_STATUS_TTL;
        snprintf(s->status_str, sizeof s->status_str, "Restore completed \xe2\x80\x94 “%s” was restored from base snapshot.", s->vol[s->selected].name);
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_TOOL_MOUNT) {
        DUVolume *v = &s->vol[s->selected];
        v->mounted = !v->mounted;
        s->status = v->mounted ? DU_STATUS_MOUNTED : DU_STATUS_UNMOUNTED;
        s->status_ttl = DU_STATUS_TTL;
        if (v->mounted) {
            snprintf(v->mount, sizeof v->mount, "/Volumes/%s", v->name);
            snprintf(s->status_str, sizeof s->status_str, "“%s” was mounted.", v->name);
        } else {
            snprintf(v->mount, sizeof v->mount, "Not Mounted");
            snprintf(s->status_str, sizeof s->status_str, "“%s” was unmounted.", v->name);
        }
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_TOOL_INFO) {
        s->info = !s->info;
        return DU_ACT_NONE;
    }

    if (hit == DU_HIT_PM_PLUS) {
        /* Add APFS volume */
        s->sheet = DU_SHEET_NEW_VOLUME;
        s->field = 0;
        snprintf(s->edit_name, sizeof s->edit_name, "Untitled");
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_PM_MINUS) {
        if (s->count > 1 && s->vol[s->selected].type == DU_NODE_VOLUME) {
            char del_name[64];
            snprintf(del_name, sizeof del_name, "%s", s->vol[s->selected].name);
            for (int i = s->selected; i < s->count - 1; i++) {
                s->vol[i] = s->vol[i + 1];
            }
            s->count--;
            if (s->selected >= s->count) s->selected = s->count - 1;
            s->status = DU_STATUS_VOLUME_ADDED;
            s->status_ttl = DU_STATUS_TTL;
            snprintf(s->status_str, sizeof s->status_str, "Volume “%s” was deleted from the container.", del_name);
        }
        return DU_ACT_NONE;
    }

    /* Sheet Controls */
    if (hit == DU_HIT_SHEET_CANCEL) {
        s->sheet = DU_SHEET_NONE;
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_SHEET_OK) {
        if (s->sheet == DU_SHEET_ERASE) {
            DUVolume *v = &s->vol[s->selected];
            char vname[64], vfs[48];
            if (s->edit_name[0]) { size_t nl = strlen(s->edit_name); if (nl >= sizeof(v->name)) nl = sizeof(v->name)-1; memcpy(v->name, s->edit_name, nl); v->name[nl] = 0; }
            snprintf(v->fs, sizeof v->fs, "%s", du_format_name(s->fmt));
            v->used_gb = 0.05f;
            s->status = DU_STATUS_ERASED;
            s->status_ttl = DU_STATUS_TTL;
            snprintf(vname, sizeof vname, "%s", v->name);
            snprintf(vfs, sizeof vfs, "%s", v->fs);
            snprintf(s->status_str, sizeof s->status_str, "“%s” was erased and formatted as %s.", vname, vfs);
        } else if (s->sheet == DU_SHEET_FIRST_AID) {
            DUVolume *v = &s->vol[s->selected];
            s->status = DU_STATUS_VERIFIED;
            s->status_ttl = DU_STATUS_TTL;
            char vname[64]; snprintf(vname, sizeof vname, "%s", s->vol[s->selected].name);
            snprintf(s->status_str, sizeof s->status_str, "First Aid completed — “%s” appears to be OK.", vname);
        } else if (s->sheet == DU_SHEET_NEW_VOLUME) {
            if (s->count < DU_MAX_VOL) {
                DUVolume *nv = &s->vol[s->count];
                memset(nv, 0, sizeof(*nv));
                snprintf(nv->name, sizeof nv->name, "%s", s->edit_name[0] ? s->edit_name : "Untitled");
                snprintf(nv->kind, sizeof nv->kind, "APFS Volume");
                snprintf(nv->fs, sizeof nv->fs, "%s", du_format_name(s->fmt));
                snprintf(nv->device, sizeof nv->device, "disk2s%d", s->count + 1);
                snprintf(nv->mount, sizeof nv->mount, "/Volumes/%s", nv->name);
                nv->cap_gb = 63.50f;
                nv->used_gb = 0.02f;
                nv->other_gb = 23.60f;
                nv->shared_vols = 8;
                nv->mounted = 1;
                nv->type = DU_NODE_VOLUME;
                nv->indent = 3;
                nv->visible = 1;
                s->selected = s->count;
                s->count++;
                s->status = DU_STATUS_VOLUME_ADDED;
                s->status_ttl = DU_STATUS_TTL;
                snprintf(s->status_str, sizeof s->status_str, "APFS Volume “%s” was added to Container disk2.", nv->name);
            }
        } else if (s->sheet == DU_SHEET_PARTITION) {
            s->status = DU_STATUS_PARTITIONED;
            s->status_ttl = DU_STATUS_TTL;
            snprintf(s->status_str, sizeof s->status_str, "Partition map updated on “%s”.", s->vol[s->selected].name);
        }
        s->sheet = DU_SHEET_NONE;
        return DU_ACT_NONE;
    }

    if (hit == DU_HIT_SHEET_FIELD_0) { s->field = 0; return DU_ACT_NONE; }
    if (hit == DU_HIT_SHEET_FIELD_1) {
        s->field = 1;
        s->fmt = (s->fmt + 1) % DU_FMT_COUNT;
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_SHEET_FIELD_2) { s->field = 2; return DU_ACT_NONE; }

    if (hit == DU_HIT_SHEET_SLICE_PLUS) {
        if (s->slice_count < DU_MAX_SLICES) {
            DUPieSlice *sl = &s->slices[s->slice_count++];
            snprintf(sl->name, sizeof sl->name, "Untitled %d", s->slice_count);
            snprintf(sl->fs, sizeof sl->fs, "APFS");
            sl->size_gb = 16.0f;
            sl->color = su_rgba(100, 160, 240, 255);
        }
        return DU_ACT_NONE;
    }
    if (hit == DU_HIT_SHEET_SLICE_MINUS) {
        if (s->slice_count > 1) s->slice_count--;
        return DU_ACT_NONE;
    }

    return DU_ACT_NONE;
}

int du_key_state(DUState *s, int key)
{
    if (s->sheet == DU_SHEET_NONE) return 0;
    if (key == 27) { /* ESC */
        s->sheet = DU_SHEET_NONE;
        return 1;
    }
    if (key == '\n' || key == '\r') {
        du_click_state(s, 0, 0, 0, 0); /* OK button handled by caller */
        return 1;
    }
    char *buf = (s->field == 2) ? s->edit_size : s->edit_name;
    size_t len = strlen(buf);
    if (key == 8 || key == 127) { /* Backspace */
        if (len > 0) buf[len - 1] = 0;
        return 1;
    }
    if (key >= 32 && key <= 126 && len < 60) {
        buf[len] = (char)key;
        buf[len + 1] = 0;
        return 1;
    }
    return 0;
}

int du_tick(DUState *s)
{
    int dirty = 0;
    if (++s->caret_ticks >= DU_CARET_TICKS) {
        s->caret_ticks = 0;
        s->caret = !s->caret;
        if (s->sheet != DU_SHEET_NONE) dirty = 1;
    }
    if (s->status_ttl > 0) {
        s->status_ttl--;
        if (s->status_ttl == 0) {
            s->status = DU_STATUS_NONE;
            s->status_str[0] = 0;
            dirty = 1;
        }
    }
    return dirty;
}

/* ============================================================ initialization */

void du_state_init(DUState *s)
{
    memset(s, 0, sizeof(*s));
    s->sidebar = 1;
    s->view_mode = DU_VIEW_ALL_DEVICES;
    s->count = 7;
    s->selected = 3; /* Macintosh HD volume */

    /* Index 0: Real 8.59 GB Physical Drive (disk0) */
    DUVolume *v0 = &s->vol[0];
    snprintf(v0->name, sizeof v0->name, "APPLE SSD AP1024Q Media");
    snprintf(v0->kind, sizeof v0->kind, "SATA Internal Physical Disk");
    snprintf(v0->fs, sizeof v0->fs, "GUID Partition Map");
    snprintf(v0->device, sizeof v0->device, "disk0");
    snprintf(v0->connection, sizeof v0->connection, "Apple Fabric");
    snprintf(v0->owners, sizeof v0->owners, "Disabled");
    v0->cap_gb = 8.59f;
    v0->used_gb = 8.00f;
    v0->type = DU_NODE_DISK;
    v0->indent = 0;
    v0->has_children = 1;
    v0->expanded = 1;
    v0->visible = 1;

    /* Index 1: APFS Container */
    DUVolume *v1 = &s->vol[1];
    snprintf(v1->name, sizeof v1->name, "Container disk2");
    snprintf(v1->kind, sizeof v1->kind, "APFS Container • 2 Volumes");
    snprintf(v1->fs, sizeof v1->fs, "Apple File System");
    snprintf(v1->device, sizeof v1->device, "disk2");
    snprintf(v1->connection, sizeof v1->connection, "Apple Fabric");
    snprintf(v1->owners, sizeof v1->owners, "Disabled");
    v1->cap_gb = 7.50f;
    v1->used_gb = 2.60f;
    v1->type = DU_NODE_CONTAINER;
    v1->indent = 1;
    v1->has_children = 1;
    v1->expanded = 1;
    v1->visible = 1;

    /* Index 2: Volume Group */
    DUVolume *v2 = &s->vol[2];
    snprintf(v2->name, sizeof v2->name, "Macintosh HD volumes");
    snprintf(v2->kind, sizeof v2->kind, "APFS Volume Group • APFS (Encrypted)");
    snprintf(v2->fs, sizeof v2->fs, "APFS Volume Group");
    snprintf(v2->device, sizeof v2->device, "disk2s1");
    snprintf(v2->connection, sizeof v2->connection, "Apple Fabric");
    snprintf(v2->owners, sizeof v2->owners, "Disabled");
    v2->cap_gb = 7.50f;
    v2->used_gb = 2.60f;
    v2->shared_vols = 2;
    v2->type = DU_NODE_VOLGROUP;
    v2->indent = 2;
    v2->has_children = 1;
    v2->expanded = 1;
    v2->visible = 1;

    /* Index 3: System Volume (Selected) */
    DUVolume *v3 = &s->vol[3];
    snprintf(v3->name, sizeof v3->name, "Macintosh HD");
    snprintf(v3->kind, sizeof v3->kind, "APFS Volume Group • APFS (Encrypted)");
    snprintf(v3->fs, sizeof v3->fs, "APFS (Encrypted)");
    snprintf(v3->device, sizeof v3->device, "disk2s1s1");
    snprintf(v3->mount, sizeof v3->mount, "/");
    snprintf(v3->snapshot_name, sizeof v3->snapshot_name, "com.apple.os.update-4DBE2E89");
    snprintf(v3->snapshot_uuid, sizeof v3->snapshot_uuid, "2404D211-8422-4302-B839-F5A93D0E8112");
    snprintf(v3->connection, sizeof v3->connection, "Apple Fabric");
    snprintf(v3->owners, sizeof v3->owners, "Disabled");
    v3->cap_gb = 7.50f;
    v3->used_gb = 2.15f;
    v3->other_gb = 0.45f;
    v3->shared_vols = 2;
    v3->mounted = 1;
    v3->type = DU_NODE_VOLUME;
    v3->indent = 3;
    v3->has_children = 0;
    v3->visible = 1;

    /* Index 4: Data Volume */
    DUVolume *v4 = &s->vol[4];
    snprintf(v4->name, sizeof v4->name, "Macintosh HD - Data");
    snprintf(v4->kind, sizeof v4->kind, "APFS Volume");
    snprintf(v4->fs, sizeof v4->fs, "APFS");
    snprintf(v4->device, sizeof v4->device, "disk2s2");
    snprintf(v4->mount, sizeof v4->mount, "/System/Volumes/Data");
    snprintf(v4->connection, sizeof v4->connection, "Apple Fabric");
    snprintf(v4->owners, sizeof v4->owners, "Enabled");
    v4->cap_gb = 7.50f;
    v4->used_gb = 0.45f;
    v4->other_gb = 2.15f;
    v4->shared_vols = 2;
    v4->mounted = 1;
    v4->type = DU_NODE_VOLUME;
    v4->indent = 3;
    v4->has_children = 0;
    v4->visible = 1;

    /* Index 5: External Media */
    DUVolume *v5 = &s->vol[5];
    snprintf(v5->name, sizeof v5->name, "Install darwinOS Recovery Media");
    snprintf(v5->kind, sizeof v5->kind, "USB External Physical Volume");
    snprintf(v5->fs, sizeof v5->fs, "Mac OS Extended");
    snprintf(v5->device, sizeof v5->device, "disk1");
    snprintf(v5->mount, sizeof v5->mount, "/Volumes/Install darwinOS");
    snprintf(v5->connection, sizeof v5->connection, "USB");
    snprintf(v5->owners, sizeof v5->owners, "Enabled");
    v5->cap_gb = 1.20f;
    v5->used_gb = 1.20f;
    v5->mounted = 1;
    v5->type = DU_NODE_EXTERNAL;
    v5->indent = 0;
    v5->has_children = 0;
    v5->visible = 1;

    /* Index 6: Disk Image */
    DUVolume *v6 = &s->vol[6];
    snprintf(v6->name, sizeof v6->name, "macOS Base System");
    snprintf(v6->kind, sizeof v6->kind, "Disk Image Volume");
    snprintf(v6->fs, sizeof v6->fs, "Mac OS Extended");
    snprintf(v6->device, sizeof v6->device, "disk3");
    snprintf(v6->mount, sizeof v6->mount, "/System/Volumes/BaseSystem");
    snprintf(v6->connection, sizeof v6->connection, "Disk Image");
    snprintf(v6->owners, sizeof v6->owners, "Enabled");
    v6->cap_gb = 2.40f;
    v6->used_gb = 2.10f;
    v6->mounted = 1;
    v6->type = DU_NODE_IMAGE;
    v6->indent = 0;
    v6->has_children = 0;
    v6->visible = 1;

    /* Partition Pie Slices */
    s->slice_count = 2;
    snprintf(s->slices[0].name, sizeof s->slices[0].name, "Media");
    snprintf(s->slices[0].fs, sizeof s->slices[0].fs, "APFS");
    s->slices[0].size_gb = 24.0f;
    s->slices[0].color = C_ACCENT;

    snprintf(s->slices[1].name, sizeof s->slices[1].name, "Macintosh HD");
    snprintf(s->slices[1].fs, sizeof s->slices[1].fs, "APFS");
    s->slices[1].size_gb = 40.0f;
    s->slices[1].color = su_rgba(180, 195, 220, 255);
}

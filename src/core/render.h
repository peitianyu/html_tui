#ifndef RENDER_H
#define RENDER_H

#include "layout.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t ch;        /* Unicode codepoint */
    int fg_r, fg_g, fg_b;
    int bg_r, bg_g, bg_b;
    bool bold;
    bool underline;
} Cell;

typedef struct {
    Cell* cells;
    int rows;
    int cols;
    int scroll_x, scroll_y;
} Screen;

Screen* screen_create(int cols, int rows);
void screen_free(Screen* s);
void screen_clear(Screen* s);
void screen_render_node(Screen* s, LayoutNode* node);
void screen_render_tree(Screen* s, LayoutNode* root);
void screen_flush(Screen* s);
void screen_scroll_clamp(Screen* s, int content_w, int content_h);

/** Draw a vertical scrollbar on the right edge of the screen. */
void screen_draw_vscrollbar(Screen* s, int content_h, int scroll_y);
/** Draw a horizontal scrollbar at the bottom (above status bar). */
void screen_draw_hscrollbar(Screen* s, int content_w, int scroll_x);

/* ======================== Interactive Loop (termbox2) ======================== */

/** Initialize terminal (termbox2 raw mode + truecolor + mouse). Returns 0 on success. */
int render_init(void);

/** Restore terminal. */
void render_shutdown(void);

/** Get current viewport size (from termbox2). */
void render_size(int* w, int* h);

/* ─── Internal helpers exposed for interact.h ─── */

/** Set a character cell (wraps Screen internal). */
void screen_scr_set(Screen* s, int c, int r, uint32_t ch);
/** Set foreground color for a cell. */
void screen_scr_fg(Screen* s, int c, int r, int R, int G, int B);
/** Set background color for a cell. */
void screen_scr_bg(Screen* s, int c, int r, int R, int G, int B);
/** Set bold flag for a cell. */
void screen_scr_bold(Screen* s, int c, int r, bool b);
/** Compute absolute screen position of a layout node (walking parent chain). */
void node_abs_box(LayoutNode* n, int scroll_x, int scroll_y,
                  int* out_x, int* out_y, int* out_w, int* out_h);
/** Find a styled node by GumboNode pointer. */
/* (declared in styletree.h — implementation moved there) */

#ifdef __cplusplus
}
#endif

#ifdef RENDER_IMPLEMENTATION
#ifndef RENDER_IMPLEMENTED
#define RENDER_IMPLEMENTED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Screen* screen_create(int cols, int rows) {
    Screen* s = (Screen*)calloc(1, sizeof(Screen));
    if (!s) return NULL;
    s->cols = cols; s->rows = rows;
    s->cells = (Cell*)calloc((size_t)(rows * cols), sizeof(Cell));
    if (!s->cells) { free(s); return NULL; }
    for (int i = 0; i < rows * cols; i++) {
        s->cells[i].ch = 0x0020;
        s->cells[i].fg_r = 255; s->cells[i].fg_g = 255; s->cells[i].fg_b = 255;
    }
    return s;
}

void screen_free(Screen* s) { if (!s) return; free(s->cells); free(s); }

void screen_clear(Screen* s) {
    if (!s) return;
    for (int i = 0; i < s->rows * s->cols; i++) {
        s->cells[i].ch = 0x0020;
        s->cells[i].fg_r = 255; s->cells[i].fg_g = 255; s->cells[i].fg_b = 255;
        s->cells[i].bg_r = 0; s->cells[i].bg_g = 0; s->cells[i].bg_b = 0;
        s->cells[i].bold = false;
        s->cells[i].underline = false;
    }
}

static inline bool scr_vis(Screen* s, int c, int r) { return c>=0 && c<s->cols && r>=0 && r<s->rows; }
static inline Cell* scr_at(Screen* s, int c, int r) { return &s->cells[r*s->cols + c]; }

/* Public wrappers for interact.h */
void screen_scr_set(Screen* s, int c, int r, uint32_t ch) { if(scr_vis(s,c,r)) scr_at(s,c,r)->ch = ch; }
void screen_scr_fg(Screen* s, int c, int r, int R, int G, int B) { if(scr_vis(s,c,r)){Cell*x=scr_at(s,c,r);x->fg_r=R;x->fg_g=G;x->fg_b=B;} }
void screen_scr_bg(Screen* s, int c, int r, int R, int G, int B) { if(scr_vis(s,c,r)){Cell*x=scr_at(s,c,r);x->bg_r=R;x->bg_g=G;x->bg_b=B;} }
void screen_scr_bold(Screen* s, int c, int r, bool b) { if(scr_vis(s,c,r)) scr_at(s,c,r)->bold=b; }

/* ─── Keep static inline for internal use ─── */
static void scr_set(Screen* s, int c, int r, uint32_t ch) { screen_scr_set(s,c,r,ch); }
static void scr_fg(Screen* s, int c, int r, int R, int G, int B) { screen_scr_fg(s,c,r,R,G,B); }
static void scr_bg(Screen* s, int c, int r, int R, int G, int B) { screen_scr_bg(s,c,r,R,G,B); }
static void scr_bold(Screen* s, int c, int r, bool b) { screen_scr_bold(s,c,r,b); }
static void scr_uline(Screen* s, int c, int r, bool u) { if(scr_vis(s,c,r)) scr_at(s,c,r)->underline=u; }

/* ─── Compute absolute screen position of a layout node ──────── */
void node_abs_box(LayoutNode* n, int scroll_x, int scroll_y,
                         int* out_x, int* out_y, int* out_w, int* out_h) {
    *out_x = n->abs_x - n->border_left - n->padding_left - scroll_x;
    *out_y = n->abs_y - n->border_top - n->padding_top - scroll_y;
    *out_w = n->width + n->padding_left + n->padding_right + n->border_left + n->border_right;
    *out_h = n->height + n->padding_top + n->padding_bottom + n->border_top + n->border_bottom;
}

/* ─── Border character lookup table ──────────────────────────── */
typedef struct { uint32_t h, v, tl, tr, bl, br; } BorderChars;

static BorderChars get_border_chars(int style) {
    static const BorderChars styles[7] = {
        /* 0: none */   {0x2500,0x2502,0x250C,0x2510,0x2514,0x2518},
        /* 1: solid */  {0x2500,0x2502,0x250C,0x2510,0x2514,0x2518},
        /* 2: dashed */ {0x254C,0x254E,0x250C,0x2510,0x2514,0x2518},
        /* 3: dotted */ {0x2504,0x2506,0x250C,0x2510,0x2514,0x2518},
        /* 4: double */ {0x2550,0x2551,0x2554,0x2557,0x255A,0x255D},
        /* 5: heavy */  {0x2501,0x2503,0x250F,0x2513,0x2517,0x251B},
        /* 6: rounded */{0x2500,0x2502,0x256D,0x256E,0x2570,0x256F},
    };
    if (style < 0 || style > 6) style = 1;
    return styles[style];
}

static void draw_border(Screen* s, int x, int y, int w, int h, ResolvedColor color, int style) {
    if (w < 2 || h < 2) return;
    int r = x + w - 1, b = y + h - 1;
    BorderChars ch = get_border_chars(style);
    scr_set(s,x,y,ch.tl); scr_set(s,r,y,ch.tr);
    scr_set(s,x,b,ch.bl); scr_set(s,r,b,ch.br);
    for (int cx = x+1; cx < r; cx++) { scr_set(s,cx,y,ch.h); scr_set(s,cx,b,ch.h); }
    for (int cy = y+1; cy < b; cy++) { scr_set(s,x,cy,ch.v); scr_set(s,r,cy,ch.v); }
    if (color.valid) {
        for (int cx = x; cx <= r; cx++) { scr_fg(s,cx,y,color.r,color.g,color.b); scr_fg(s,cx,b,color.r,color.g,color.b); }
        for (int cy = y; cy <= b; cy++) { scr_fg(s,x,cy,color.r,color.g,color.b); scr_fg(s,r,cy,color.r,color.g,color.b); }
    }
}

void screen_render_node(Screen* s, LayoutNode* n) {
    if (!n || !s || n->display == DISPLAY_NONE) return;
    int sx = s->scroll_x, sy = s->scroll_y;
    int box_x = n->x - n->border_left - n->padding_left;
    int box_y = n->y - n->border_top - n->padding_top;
    int scr_x = box_x - sx, scr_y = box_y - sy;
    int tw = n->width + n->padding_left + n->padding_right + n->border_left + n->border_right;
    int th = n->height + n->padding_top + n->padding_bottom + n->border_top + n->border_bottom;
    if (scr_x+tw < 0 || scr_x >= s->cols || scr_y+th < 0 || scr_y >= s->rows) return;

    /* visibility: hidden — element occupies space but is invisible */
    bool vis_hidden = n->visibility_hidden;

    if (!vis_hidden && n->bg_color.valid) {
        int x1=scr_x>0?scr_x:0, y1=scr_y>0?scr_y:0, x2=scr_x+tw<s->cols?scr_x+tw:s->cols, y2=scr_y+th<s->rows?scr_y+th:s->rows;
        for (int row=y1; row<y2; row++) for (int col=x1; col<x2; col++) scr_bg(s,col,row,n->bg_color.r,n->bg_color.g,n->bg_color.b);
    }
    if (!vis_hidden && n->border_top > 0 && n->border_style > 0) {
        /* Skip border drawing for table cells - table draws a unified grid */
        bool skip = false;
        if (n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT) {
            GumboTag t = n->styled->node->v.element.tag;
            skip = (t == GUMBO_TAG_TABLE || t == GUMBO_TAG_TD || t == GUMBO_TAG_TH);
        }
        if (!skip) {
            ResolvedColor bc = n->border_color; if (!bc.valid && n->color.valid) bc = n->color; bc.valid = true;
            draw_border(s, scr_x, scr_y, tw, th, bc, n->border_style);
        }
    }
    /* outline: draw outside the border (similar to border but at outline_width) */
    if (!vis_hidden && n->outline_width > 0) {
        ResolvedColor oc = n->outline_color;
        if (!oc.valid && n->color.valid) oc = n->color;
        oc.valid = true;
        int ow = n->outline_width;
        draw_border(s, scr_x - ow, scr_y - ow, tw + 2*ow, th + 2*ow, oc, 1);
    }
    /* <hr>: draw horizontal line across content width */
    if (!vis_hidden && n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT &&
        n->styled->node->v.element.tag == GUMBO_TAG_HR && n->color.valid) {
        int hx = scr_x + n->border_left + n->padding_left;
        int hy = scr_y + n->border_top + n->padding_top;
        int w = n->width; if (w < 0) w = 0;
        int cw = 0x2500; /* ─ */
        for (int ci = 0; ci < w && hx + ci < s->cols; ci++) {
            int col = hx + ci;
            if (col >= 0) { scr_set(s, col, hy, cw); scr_fg(s,col,hy,n->color.r,n->color.g,n->color.b); }
        }
    }
    if (!vis_hidden && n->text_content && n->color.valid) {
        int lnx = scr_x + n->border_left + n->padding_left;
        int cy = scr_y + n->border_top + n->padding_top, mw = n->width; if (mw<1) mw=1;
        int lh = n->line_height > 0 ? n->line_height : 1;
        const char* p = n->text_content;

        /* Render list marker if applicable */
        int cx = lnx;
        if (n->list_marker > 0) {
            if (n->list_marker == 1) {
                if (cx >= 0 && cx < s->cols) {
                    scr_set(s, cx, cy, 0x2022);
                    scr_fg(s, cx, cy, n->color.r, n->color.g, n->color.b);
                }
                cx += 2;
            } else if (n->list_marker == 2) {
                char num[16];
                int nlen = snprintf(num, sizeof(num), "%d. ", n->list_index);
                for (int k = 0; k < nlen && k < 8; k++) {
                    if (cx >= 0 && cx < s->cols) {
                        scr_set(s, cx, cy, (unsigned char)num[k]);
                        scr_fg(s, cx, cy, n->color.r, n->color.g, n->color.b);
                    }
                    cx++;
                }
            }
        }

        if (n->preserve_ws) {
            /* <pre> / <textarea> mode: preserve whitespace, no word-wrap, with scroll */
            /* Determine visible height for scroll clipping */
            int scroll_h = 0;
            bool is_textarea_for_scroll = false;
            if (n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT &&
                n->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
                scroll_h = n->height; /* content area height */
                is_textarea_for_scroll = true;
            }
            int scroll_skip = n->content_scroll_y;
            int scroll_left = n->content_scroll_x; /* chars to skip on the left */

            while (*p && cy < s->rows) {
                if (*p == '\n') {
                    if (is_textarea_for_scroll) {
                        if (scroll_skip > 0) {
                            scroll_skip--;
                            p++;
                            cx = lnx;
                            continue;
                        }
                        /* Check if we've filled the visible area */
                        if ((cy - (scr_y + n->border_top + n->padding_top)) / lh >= scroll_h)
                            break;
                    }
                    cy += lh; cx = lnx; p++; continue;
                }
                if (*p == '\t') { cx = ((cx - lnx) / 4 + 1) * 4 + lnx; p++; continue; }
                uint32_t cp = uc_dec(&p);
                if (cp == 0 || cp == 0xFFFD) continue;
                int w = uc_wid((int)cp);
                /* Skip characters on scrolled-off-the-top lines */
                if (is_textarea_for_scroll && scroll_skip > 0) {
                    cx += w + n->letter_spacing;
                    continue;
                }
                int render_cx = cx - scroll_left;
                /* Horizontal scroll: skip chars past visible width to the right */
                if (is_textarea_for_scroll && render_cx >= n->width) {
                    cx += w + n->letter_spacing;
                    continue;
                }
                /* Horizontal scroll: skip chars that are scrolled off to the left */
                if (is_textarea_for_scroll && render_cx + w <= 0) {
                    cx += w + n->letter_spacing;
                    continue;
                }
                if (render_cx >= 0 && render_cx < s->cols) {
                    scr_set(s, render_cx, cy, cp);
                    scr_fg(s, render_cx, cy, n->color.r, n->color.g, n->color.b);
                    if (n->font_bold) scr_bold(s, render_cx, cy, true);
                    if (n->font_underline == 1) scr_uline(s, render_cx, cy, true);
                    else if (n->font_underline == 2 && render_cx >= 0 && render_cx < s->cols) {
                        /* overline: macron above */
                        scr_set(s, render_cx, cy, 0x00AF);
                        scr_fg(s, render_cx, cy, n->color.r, n->color.g, n->color.b);
                    }
                    else if (n->font_underline == 3 && render_cx >= 0 && render_cx < s->cols) {
                        /* line-through: strike with - */
                        scr_set(s, render_cx, cy, 0x002D);
                        scr_fg(s, render_cx, cy, n->color.r, n->color.g, n->color.b);
                    }
                    if (w == 2 && render_cx + 1 < s->cols) scr_set(s, render_cx + 1, cy, 0);
                }
                cx += w + n->letter_spacing;
            }
            /* ── Draw textarea scrollbar if content overflows visible area ── */
            if (is_textarea_for_scroll) {
                int _total = 1;
                const char* _cp = n->text_content;
                while (_cp && *_cp) { if (*_cp == '\n') _total++; _cp++; }
                int _sb_right = scr_x + n->border_left + n->padding_left + n->width;
                if (_total > scroll_h && _sb_right >= 0 && _sb_right < s->cols) {
                    int _track_h = scroll_h;
                    int _thumb_h = _track_h * _track_h / _total;
                    if (_thumb_h < 1) _thumb_h = 1;
                    if (_thumb_h > _track_h) _thumb_h = _track_h;
                    int _scroll_range = _total - scroll_h;
                    int _thumb_pos = _scroll_range > 0 ? (n->content_scroll_y * (_track_h - _thumb_h)) / _scroll_range : 0;
                    if (_thumb_pos < 0) _thumb_pos = 0;
                    if (_thumb_pos > _track_h - _thumb_h) _thumb_pos = _track_h - _thumb_h;
                    int _sb_top = scr_y + n->border_top + n->padding_top;
                    for (int _ri = 0; _ri < _track_h && _sb_top + _ri < s->rows; _ri++) {
                        if (_sb_top + _ri >= 0) {
                            if (_ri >= _thumb_pos && _ri < _thumb_pos + _thumb_h) {
                                scr_set(s, _sb_right, _sb_top + _ri, 0x2592); /* ▒ thumb */
                                scr_fg(s, _sb_right, _sb_top + _ri, 120,180,255);
                                scr_bg(s, _sb_right, _sb_top + _ri, n->bg_color.valid ? n->bg_color.r : 0,
                                       n->bg_color.valid ? n->bg_color.g : 0,
                                       n->bg_color.valid ? n->bg_color.b : 0);
                            } else {
                                scr_set(s, _sb_right, _sb_top + _ri, 0x2502); /* │ track */
                                scr_fg(s, _sb_right, _sb_top + _ri, 60,90,130);
                            }
                        }
                    }
                }
            }
        } else {
            /* Normal word-wrap mode */

            /* Alignment: measure first line width for center/right */
            if (n->text_align == 1 || n->text_align == 2) {
                int line_w = 0;
                const char* q = p;
                while (*q && *q != '\n') {
                    uint32_t cp = uc_dec(&q);
                    if (cp == 0) break;
                    if (cp != ' ' && cp != '\t') line_w += uc_wid((int)cp);
                }
                if (n->text_align == 1 && line_w < mw) cx = lnx + (mw - line_w) / 2;
                else if (n->text_align == 2 && line_w < mw) cx = lnx + mw - line_w;
            }

            while (*p && cy < s->rows) {
                /* Skip leading spaces/tabs */
                while (*p == ' ' || *p == '\t') { cx++; p++; }
                if (!*p) break;
                if (*p == '\n') { cy += lh; cx = lnx; p++; continue; }

                /* Measure word display width */
                const char* we = p;
                int word_w = 0;
                while (*we && *we != ' ' && *we != '\t' && *we != '\n') {
                    uint32_t cp = uc_dec(&we);
                    if (cp == 0) break;
                    word_w += uc_wid((int)cp);
                }

                /* Word-wrap if needed */
                if (cx - lnx + word_w > mw && cx > lnx) {
                    if (n->truncate_overflow) break;  /* truncate, don't wrap */
                    cy += lh; cx = lnx;
                }
                if (cy >= s->rows) break;

                /* Render word characters */
                while (p < we && cy < s->rows) {
                    uint32_t cp = uc_dec(&p);
                    if (cp == 0 || cp == 0xFFFD) continue;
                    int w = uc_wid((int)cp);
                    /* Truncate if beyond content width */
                    if (n->truncate_overflow && cx - lnx + w > mw) {
                        /* Draw ellipsis '…' at the last visible position */
                        if (cx - lnx < mw && cx >= 0 && cx < s->cols) {
                            scr_set(s, cx, cy, 0x2026); /* … */
                            scr_fg(s, cx, cy, n->color.r, n->color.g, n->color.b);
                        }
                        /* Skip remaining characters */
                        p = we;
                        break;
                    }
                    if (cx >= 0 && cx < s->cols) {
                        scr_set(s, cx, cy, cp);
                        scr_fg(s, cx, cy, n->color.r, n->color.g, n->color.b);
                        if (n->font_bold) scr_bold(s, cx, cy, true);
                        if (n->font_underline) scr_uline(s, cx, cy, true);
                        if (w == 2 && cx + 1 < s->cols) scr_set(s, cx + 1, cy, 0);
                    }
                    cx += w + n->letter_spacing;
                    if (cx - lnx >= mw && p < we) {
                        if (n->truncate_overflow) { p = we; break; }
                        cy += lh; cx = lnx;
                    }
                }

                /* Skip trailing space */
                if (*p == ' ') { cx += 1 + n->word_spacing; p++; }
                /* Handle newlines */
                while (*p == '\n') { cy += lh; cx = lnx; p++; }
            }
        }
    }
}

/* ─── Table grid: draw unified grid for <table> ──────────────── */
static int sort_ints(const void* a, const void* b) { return *(const int*)a - *(const int*)b; }
static void add_unique(int** arr, size_t* n, size_t* cap, int val) {
    for (size_t i = 0; i < *n; i++) if ((*arr)[i] == val) return;
    if (*n >= *cap) { *cap *= 2; *arr = realloc(*arr, *cap * sizeof(int)); }
    (*arr)[(*n)++] = val;
}

/* Collect grid line positions from table cells (X from td/th, Y from tr rows) */
static void collect_grid_from_table(LayoutNode* node, int scroll_x, int scroll_y,
                                     int** xs, size_t* nx, size_t* cap_x,
                                     int** ys, size_t* ny, size_t* cap_y) {
    if (!node || node->display == DISPLAY_NONE) return;
    /* td/th: add their X box edges */
    if (node->styled && node->styled->node &&
        node->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboTag t = node->styled->node->v.element.tag;
        if (t == GUMBO_TAG_TD || t == GUMBO_TAG_TH) {
            int bx, by, bw, bh;
            node_abs_box(node, scroll_x, scroll_y, &bx, &by, &bw, &bh);
            add_unique(xs, nx, cap_x, bx);
            add_unique(xs, nx, cap_x, bx + bw);
            return;
        }
    }
    /* tr: add Y row edges (rows define grid line positions, not cells) */
    if (node->styled && node->styled->node &&
        node->styled->node->type == GUMBO_NODE_ELEMENT &&
        node->styled->node->v.element.tag == GUMBO_TAG_TR) {
        int bx, by, bw, bh;
        node_abs_box(node, scroll_x, scroll_y, &bx, &by, &bw, &bh);
        add_unique(ys, ny, cap_y, by);
        add_unique(ys, ny, cap_y, by + bh);
        /* Don't return — continue to collect X from td/th children */
    }
    for (size_t i = 0; i < node->num_children; i++)
        collect_grid_from_table(node->children[i], scroll_x, scroll_y, xs, nx, cap_x, ys, ny, cap_y);
}

/* Draw a unified grid for a <table> node */
static void draw_table_grid(Screen* s, LayoutNode* table) {
    int sx = s->scroll_x, sy = s->scroll_y;

    /* Collect column (x) and row (y) boundaries from cells only */
    size_t cap_x = 64, nx = 0; int* xs = malloc(cap_x * sizeof(int));
    size_t cap_y = 64, ny = 0; int* ys = malloc(cap_y * sizeof(int));

    collect_grid_from_table(table, sx, sy, &xs, &nx, &cap_x, &ys, &ny, &cap_y);

    if (nx < 2 || ny < 2) { free(xs); free(ys); return; }

    /* Sort */
    qsort(xs, nx, sizeof(int), sort_ints);
    qsort(ys, ny, sizeof(int), sort_ints);

    /* Determine line style from table; cells inherit same style */
    BorderChars bc_ch = get_border_chars(table->border_style);
    uint32_t ch_h = bc_ch.h, ch_v = bc_ch.v;
    uint32_t ch_tl = bc_ch.tl, ch_tr = bc_ch.tr;
    uint32_t ch_bl = bc_ch.bl, ch_br = bc_ch.br;

    ResolvedColor bc = table->border_color;
    if (!bc.valid) { bc.r = 180; bc.g = 180; bc.b = 180; bc.valid = true; }

    /* Draw horizontal grid lines — continuous from leftmost to rightmost */
    for (size_t yi = 0; yi < ny; yi++) {
        int y = ys[yi];
        if (y < 0 || y >= s->rows) continue;
        int x_start = xs[0] + 1;
        int x_end   = xs[nx - 1];
        if (x_start >= x_end) continue;
        for (int cx = x_start; cx < x_end && cx < s->cols; cx++) {
            if (cx >= 0) { scr_set(s, cx, y, ch_h); scr_fg(s,cx,y,bc.r,bc.g,bc.b); }
        }
    }

    /* Draw vertical grid lines — continuous from topmost to bottommost */
    for (size_t xi = 0; xi < nx; xi++) {
        int x = xs[xi];
        if (x < 0 || x >= s->cols) continue;
        int y_start = ys[0] + 1;
        int y_end   = ys[ny - 1];
        if (y_start >= y_end) continue;
        for (int cy = y_start; cy < y_end && cy < s->rows; cy++) {
            if (cy >= 0) { scr_set(s, x, cy, ch_v); scr_fg(s,x,cy,bc.r,bc.g,bc.b); }
        }
    }

    /* Draw intersection characters at each grid point */
    for (size_t yi = 0; yi < ny; yi++) {
        int y = ys[yi];
        if (y < 0 || y >= s->rows) continue;
        for (size_t xi = 0; xi < nx; xi++) {
            int x = xs[xi];
            if (x < 0 || x >= s->cols) continue;
            bool up    = yi > 0;
            bool down  = yi + 1 < ny;
            bool left  = xi > 0;
            bool right = xi + 1 < nx;

            uint32_t isect;
            if      (up && down && left && right) isect = 0x253C; /* ┼ */
            else if (up && down && left)          isect = 0x2524; /* ┤ */
            else if (up && down && right)         isect = 0x251C; /* ├ */
            else if (up && left && right)         isect = 0x2534; /* ┴ */
            else if (down && left && right)       isect = 0x252C; /* ┬ */
            else if (up && down)                  isect = 0x2502; /* │ */
            else if (left && right)               isect = 0x2500; /* ─ */
            else if (down && right)               isect = ch_tl;  /* top-left corner */
            else if (down && left)                isect = ch_tr;  /* top-right corner */
            else if (up && right)                 isect = ch_bl;  /* bottom-left corner */
            else if (up && left)                  isect = ch_br;  /* bottom-right corner */
            else                                  isect = 0x0020;

            scr_set(s, x, y, isect);
            scr_fg(s, x, y, bc.r, bc.g, bc.b);
        }
    }

    free(xs); free(ys);
}

/**
 * Render a node and its children with optional clip rect.
 * clip_x/y/w/h define the visible area (in screen coords after scroll).
 * clip_w <= 0 means no clipping.
 */
static void screen_render_offset(Screen* s, LayoutNode* n, int px, int py, int clip_x, int clip_y, int clip_w, int clip_h) {
    if (!n || n->display == DISPLAY_NONE || n->visibility_hidden) return;
    int svx=n->x, svy=n->y; n->x+=px; n->y+=py;

    /* Compute effective clip for children */
    int my_clip_x = clip_x, my_clip_y = clip_y, my_clip_w = clip_w, my_clip_h = clip_h;
    if (n->overflow_hidden) {
        int sx = n->x - n->border_left - n->padding_left;
        int sy = n->y - n->border_top - n->padding_top;
        int sw = n->width + n->padding_left + n->padding_right + n->border_left + n->border_right;
        int sh = n->height + n->padding_top + n->padding_bottom + n->border_top + n->border_bottom;
        int sx2 = sx + sw, sy2 = sy + sh;
        /* Intersect with parent clip */
        if (clip_w > 0) {
            int cx2 = clip_x + clip_w, cy2 = clip_y + clip_h;
            if (sx < clip_x) sx = clip_x;
            if (sy < clip_y) sy = clip_y;
            if (sx2 > cx2) sx2 = cx2;
            if (sy2 > cy2) sy2 = cy2;
        }
        my_clip_x = sx; my_clip_y = sy;
        my_clip_w = sx2 - sx > 0 ? sx2 - sx : 0;
        my_clip_h = sy2 - sy > 0 ? sy2 - sy : 0;
    }

    /* Check visibility against clip */
    {
        int sx = n->x - n->border_left - n->padding_left;
        int sy = n->y - n->border_top - n->padding_top;
        int sw = n->width + n->padding_left + n->padding_right + n->border_left + n->border_right;
        int sh = n->height + n->padding_top + n->padding_bottom + n->border_top + n->border_bottom;
        if (my_clip_w > 0) {
            if (sx + sw <= my_clip_x || sx >= my_clip_x + my_clip_w ||
                sy + sh <= my_clip_y || sy >= my_clip_y + my_clip_h) {
                /* Outside clip — still render children? No, skip entirely */
                n->x = svx; n->y = svy;
                return;
            }
        }
    }

    screen_render_node(s, n);
    int cpx=n->x, cpy=n->y; n->x=svx; n->y=svy;

    for (size_t i = 0; i < n->num_children; i++)
        screen_render_offset(s, n->children[i], cpx, cpy, my_clip_x, my_clip_y, my_clip_w, my_clip_h);

    /* Draw table grid if this is a <table> node */
    if (n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT &&
        n->styled->node->v.element.tag == GUMBO_TAG_TABLE) {
        draw_table_grid(s, n);
    }
}

void screen_render_tree(Screen* s, LayoutNode* root) {
    if (s && root) screen_render_offset(s, root, 0, 0, 0, 0, 0, 0);
}

void screen_flush(Screen* s) {
    if (!s) return;
    printf("\033[H");
    int pfr=-1,pfg=-1,pfb=-1,pbr=-1,pbg=-1,pbb=-1; bool pb=false,pu=false;
    for (int r=0; r<s->rows; r++) {
        for (int c=0; c<s->cols; c++) {
            Cell* cl = &s->cells[r*s->cols + c];
            if (cl->ch == 0) {
                /* Empty/wide-continuation cell — leave ANSI state as-is, just output space */
                putchar(' ');
                continue;
            }
            if (cl->fg_r!=pfr||cl->fg_g!=pfg||cl->fg_b!=pfb) { printf("\033[38;2;%d;%d;%dm",cl->fg_r,cl->fg_g,cl->fg_b); pfr=cl->fg_r;pfg=cl->fg_g;pfb=cl->fg_b; }
            if (cl->bg_r!=pbr||cl->bg_g!=pbg||cl->bg_b!=pbb) { printf("\033[48;2;%d;%d;%dm",cl->bg_r,cl->bg_g,cl->bg_b); pbr=cl->bg_r;pbg=cl->bg_g;pbb=cl->bg_b; }
            if (cl->bold!=pb) { printf(cl->bold?"\033[1m":"\033[22m"); pb=cl->bold; }
            if (cl->underline!=pu) { printf(cl->underline?"\033[4m":"\033[24m"); pu=cl->underline; }
            char buf[4];
            int n = uc_enc(cl->ch, buf);
            fwrite(buf, 1, (size_t)n, stdout);
            /* Skip next column if this was a double-width character */
            if (uc_wid((int)cl->ch) > 1) {
                c++; /* skip the 2nd cell which was zeroed by scr_set */
            }
        }
        if (r < s->rows-1) printf("\r\n");
    }
    printf("\033[0m"); fflush(stdout);
}

/* Scroll helpers */
void screen_scroll_clamp(Screen* s, int cw, int ch) {
    if(!s)return;
    if(s->scroll_x < 0) s->scroll_x=0;
    if(s->scroll_y < 0) s->scroll_y=0;
    int mx=cw-s->cols, my=ch-s->rows;
    if(s->scroll_x>mx) s->scroll_x = mx>0 ? mx : 0;
    if(s->scroll_y>my) s->scroll_y = my>0 ? my : 0;
}

/* ─── Global scrollbar drawing ────────────────────────────── */
void screen_draw_vscrollbar(Screen* s, int content_h, int scroll_y) {
    int track_h = s->rows - 1; /* leave last row for status bar */
    if (track_h < 2) return;
    int col = s->cols - 1;
    if (col < 0) return;
    if (content_h <= track_h) return;
    int thumb_h = track_h * track_h / content_h;
    if (thumb_h < 1) thumb_h = 1;
    if (thumb_h > track_h) thumb_h = track_h;
    int scroll_range = content_h - track_h;
    int thumb_pos = (scroll_y * (track_h - thumb_h)) / scroll_range;
    if (thumb_pos < 0) thumb_pos = 0;
    if (thumb_pos > track_h - thumb_h) thumb_pos = track_h - thumb_h;
    for (int r = 0; r < track_h; r++) {
        if (r >= thumb_pos && r < thumb_pos + thumb_h) {
            scr_set(s, col, r, 0x2592); /* ▒ thumb */
            scr_fg(s, col, r, 120, 180, 255);
            scr_bg(s, col, r, 30, 30, 50);
        } else {
            scr_set(s, col, r, 0x2502); /* │ track */
            scr_fg(s, col, r, 50, 80, 120);
            scr_bg(s, col, r, 0, 0, 0);
        }
    }
}

void screen_draw_hscrollbar(Screen* s, int content_w, int scroll_x) {
    int sb_row = s->rows - 2; /* row above status bar */
    if (sb_row < 0) return;
    int track_w = s->cols; /* full width; vscrollbar overwrites corner if visible */
    if (track_w < 2) return;
    if (content_w <= track_w) return;
    int thumb_w = track_w * track_w / content_w;
    if (thumb_w < 1) thumb_w = 1;
    if (thumb_w > track_w) thumb_w = track_w;
    int scroll_range = content_w - track_w;
    int thumb_pos = (scroll_x * (track_w - thumb_w)) / scroll_range;
    if (thumb_pos < 0) thumb_pos = 0;
    if (thumb_pos > track_w - thumb_w) thumb_pos = track_w - thumb_w;
    for (int c = 0; c < track_w; c++) {
        if (c >= thumb_pos && c < thumb_pos + thumb_w) {
            scr_set(s, c, sb_row, 0x2592); /* ▒ thumb */
            scr_fg(s, c, sb_row, 120, 180, 255);
            scr_bg(s, c, sb_row, 30, 30, 50);
        } else {
            scr_set(s, c, sb_row, 0x2500); /* ─ track */
            scr_fg(s, c, sb_row, 50, 80, 120);
            scr_bg(s, c, sb_row, 0, 0, 0);
        }
    }
}

#include <locale.h>
#include <errno.h>

#define TB_OPT_ATTR_W 32
#define TB_IMPL
#include "termbox2.h"

int render_init(void) {
    setlocale(LC_ALL, "");
    if (tb_init()) return -1;
    tb_set_output_mode(TB_OUTPUT_TRUECOLOR);
    tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
    return 0;
}

void render_shutdown(void) {
    tb_shutdown();
}

void render_size(int* w, int* h) {
    *w = tb_width();
    *h = tb_height();
}

/* find_styled_node moved to styletree.h */

#endif /* RENDER_IMPLEMENTED */
#endif /* RENDER_IMPLEMENTATION */
#endif /* RENDER_H */

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
void screen_scroll_to(Screen* s, int x, int y);
void screen_scroll_by(Screen* s, int dx, int dy);
void screen_scroll_clamp(Screen* s, int content_w, int content_h);

/* ======================== Interactive Loop (termbox2) ======================== */

/** Initialize terminal (termbox2 raw mode + truecolor + mouse). Returns 0 on success. */
int render_init(void);

/** Restore terminal. */
void render_shutdown(void);

/** Get current viewport size (from termbox2). */
void render_size(int* w, int* h);

/** Run interactive loop. Pass stylesheet + styled_root for :hover/:focus/:active support. */
void render_run_ex(LayoutNode* root, KatanaStylesheet* css, StyledNode* styled_root);

/** Set a status message displayed at the bottom of the screen. */
void render_set_status(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#ifdef RENDER_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Screen* screen_create(int cols, int rows) {
    Screen* s = (Screen*)calloc(1, sizeof(Screen));
    s->cols = cols; s->rows = rows;
    s->cells = (Cell*)calloc((size_t)(rows * cols), sizeof(Cell));
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
static void scr_set(Screen* s, int c, int r, uint32_t ch) { if(scr_vis(s,c,r)) scr_at(s,c,r)->ch = ch; }
static void scr_fg(Screen* s, int c, int r, int R, int G, int B) { if(scr_vis(s,c,r)){Cell*x=scr_at(s,c,r);x->fg_r=R;x->fg_g=G;x->fg_b=B;} }
static void scr_bg(Screen* s, int c, int r, int R, int G, int B) { if(scr_vis(s,c,r)){Cell*x=scr_at(s,c,r);x->bg_r=R;x->bg_g=G;x->bg_b=B;} }
static void scr_bold(Screen* s, int c, int r, bool b) { if(scr_vis(s,c,r)) scr_at(s,c,r)->bold=b; }
static void scr_uline(Screen* s, int c, int r, bool u) { if(scr_vis(s,c,r)) scr_at(s,c,r)->underline=u; }

/* ─── Compute absolute screen position of a layout node ──────── */
static void node_abs_box(LayoutNode* n, int scroll_x, int scroll_y,
                         int* out_x, int* out_y, int* out_w, int* out_h) {
    int ax = 0, ay = 0;
    LayoutNode* p = n;
    while (p) {
        ax += p->x;
        ay += p->y;
        p = p->parent;
    }
    *out_x = ax - n->border_left - n->padding_left - scroll_x;
    *out_y = ay - n->border_top - n->padding_top - scroll_y;
    *out_w = n->width + n->padding_left + n->padding_right + n->border_left + n->border_right;
    *out_h = n->height + n->padding_top + n->padding_bottom + n->border_top + n->border_bottom;
}

static void draw_border(Screen* s, int x, int y, int w, int h, ResolvedColor color, int style) {
    if (w < 2 || h < 2) return;
    int r = x + w - 1, b = y + h - 1;
    uint32_t ch_h, ch_v, ch_tl, ch_tr, ch_bl, ch_br;
    switch (style) {
        case 6: /* rounded */
            ch_h = 0x2500; ch_v = 0x2502;   /* ─ │ */
            ch_tl = 0x256D; ch_tr = 0x256E;
            ch_bl = 0x2570; ch_br = 0x256F; /* ╭ ╮ ╰ ╯ */
            break;
        case 5: /* heavy */
            ch_h = 0x2501; ch_v = 0x2503;   /* ━ ┃ */
            ch_tl = 0x250F; ch_tr = 0x2513;
            ch_bl = 0x2517; ch_br = 0x251B; /* ┏ ┓ ┗ ┛ */
            break;
        case 4: /* double */
            ch_h = 0x2550; ch_v = 0x2551;   /* ═ ║ */
            ch_tl = 0x2554; ch_tr = 0x2557;
            ch_bl = 0x255A; ch_br = 0x255D; /* ╔ ╗ ╚ ╝ */
            break;
        case 3: /* dotted */
            ch_h = 0x2504; ch_v = 0x2506;   /* ┄ ┆ */
            ch_tl = 0x250C; ch_tr = 0x2510;
            ch_bl = 0x2514; ch_br = 0x2518; /* ┌ ┐ └ ┘ */
            break;
        case 2: /* dashed */
            ch_h = 0x254C; ch_v = 0x254E;   /* ╌ ╎ */
            ch_tl = 0x250C; ch_tr = 0x2510;
            ch_bl = 0x2514; ch_br = 0x2518; /* ┌ ┐ └ ┘ */
            break;
        default: /* solid */
            ch_h = 0x2500; ch_v = 0x2502;   /* ─ │ */
            ch_tl = 0x250C; ch_tr = 0x2510;
            ch_bl = 0x2514; ch_br = 0x2518; /* ┌ ┐ └ ┘ */
            break;
    }
    scr_set(s,x,y,ch_tl); scr_set(s,r,y,ch_tr);
    scr_set(s,x,b,ch_bl); scr_set(s,r,b,ch_br);
    for (int cx = x+1; cx < r; cx++) { scr_set(s,cx,y,ch_h); scr_set(s,cx,b,ch_h); }
    for (int cy = y+1; cy < b; cy++) { scr_set(s,x,cy,ch_v); scr_set(s,r,cy,ch_v); }
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

    if (n->bg_color.valid) {
        int x1=scr_x>0?scr_x:0, y1=scr_y>0?scr_y:0, x2=scr_x+tw<s->cols?scr_x+tw:s->cols, y2=scr_y+th<s->rows?scr_y+th:s->rows;
        for (int row=y1; row<y2; row++) for (int col=x1; col<x2; col++) scr_bg(s,col,row,n->bg_color.r,n->bg_color.g,n->bg_color.b);
    }
    if (n->border_top > 0 && n->border_style > 0) {
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
    /* <hr>: draw horizontal line across content width */
    if (n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT &&
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
    if (n->text_content && n->color.valid) {
        int lnx = scr_x + n->border_left + n->padding_left;
        int cy = scr_y + n->border_top + n->padding_top, mw = n->width; if (mw<1) mw=1;
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
            /* <pre> mode: preserve whitespace, no word-wrap */
            while (*p && cy < s->rows) {
                if (*p == '\n') { cy++; cx = lnx; p++; continue; }
                if (*p == '\t') { cx = ((cx - lnx) / 4 + 1) * 4 + lnx; p++; continue; }
                uint32_t cp = uc_dec(&p);
                if (cp == 0 || cp == 0xFFFD) continue;
                int w = uc_wid((int)cp);
                if (cx >= 0 && cx < s->cols) {
                    scr_set(s, cx, cy, cp);
                    scr_fg(s, cx, cy, n->color.r, n->color.g, n->color.b);
                    if (n->font_bold) scr_bold(s, cx, cy, true);
                    if (n->font_underline) scr_uline(s, cx, cy, true);
                    if (w == 2 && cx + 1 < s->cols) scr_set(s, cx + 1, cy, 0);
                }
                cx += w;
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
                if (*p == '\n') { cy++; cx = lnx; p++; continue; }

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
                    cy++; cx = lnx;
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
                    cx += w;
                    if (cx - lnx >= mw && p < we) {
                        if (n->truncate_overflow) { p = we; break; }
                        cy++; cx = lnx;
                    }
                }

                /* Skip trailing space */
                if (*p == ' ') { cx++; p++; }
                /* Handle newlines */
                while (*p == '\n') { cy++; cx = lnx; p++; }
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
    uint32_t ch_h = 0x2500, ch_v = 0x2502;
    uint32_t ch_tl = 0x250C, ch_tr = 0x2510;
    uint32_t ch_bl = 0x2514, ch_br = 0x2518;
    switch (table->border_style) {
        case 6: /* rounded */
            ch_h = 0x2500; ch_v = 0x2502;
            ch_tl = 0x256D; ch_tr = 0x256E;
            ch_bl = 0x2570; ch_br = 0x256F;
            break;
        case 5: /* heavy */
            ch_h = 0x2501; ch_v = 0x2503;
            ch_tl = 0x250F; ch_tr = 0x2513;
            ch_bl = 0x2517; ch_br = 0x251B;
            break;
        case 4: /* double */
            ch_h = 0x2550; ch_v = 0x2551;
            ch_tl = 0x2554; ch_tr = 0x2557;
            ch_bl = 0x255A; ch_br = 0x255D;
            break;
        default: break;
    }

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
    if (!n || n->display == DISPLAY_NONE) return;
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
void screen_scroll_to(Screen* s, int x, int y) { if(s){s->scroll_x=x;s->scroll_y=y;} }
void screen_scroll_by(Screen* s, int dx, int dy) { if(s){s->scroll_x+=dx;s->scroll_y+=dy;} }
void screen_scroll_clamp(Screen* s, int cw, int ch) {
    if(!s)return;
    if(s->scroll_x < 0) s->scroll_x=0;
    if(s->scroll_y < 0) s->scroll_y=0;
    int mx=cw-s->cols, my=ch-s->rows;
    if(s->scroll_x>mx) s->scroll_x = mx>0 ? mx : 0;
    if(s->scroll_y>my) s->scroll_y = my>0 ? my : 0;
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

/* Helper: find styled node by GumboNode pointer */
static StyledNode* find_styled_node(StyledNode* st, GumboNode* gn) {
    if (!st || !gn) return NULL;
    if (st->node == gn) return st;
    for (size_t i = 0; i < st->num_children; i++) {
        StyledNode* r = find_styled_node(st->children[i], gn);
        if (r) return r;
    }
    return NULL;
}

// ─── Status message for interactive test ──────────────────────────
static char g_status_msg[256] = "";
void render_set_status(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_status_msg, sizeof(g_status_msg), fmt, ap);
    va_end(ap);
}

// ─── Track input buffers across rebuilds ────────────────────────
// Indexed by the same order as focus_list.
static char g_input_buf[256][64] = {{{0}}};
static int  g_input_buf_count = 0;

/* ─── Button click handler ────────────────────────────────────── */
/* Returns true if a layout rebuild is needed */
static void handle_button_click(LayoutNode* btn, LayoutNode** focus_list, int focus_count,
                                 char input_buf[][64], int* input_buf_count,
                                 LayoutNode* root, bool need_restyle) {
    (void)root; (void)need_restyle;
    /* Find button text content (trimmed) */
    const char* btxt = btn->text_content;
    if (!btxt) btxt = "(unnamed)";
    /* Trim surrounding whitespace/newlines for comparison */
    while (*btxt == ' ' || *btxt == '\n' || *btxt == '\r') btxt++;
    if (strcmp(btxt, "提交") == 0) {
        /* Collect all input values and show them */
        char vals[512] = "";
        int n = 0;
        for (int i = 0; i < focus_count && i < *input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                char tmp[80];
                snprintf(tmp, sizeof(tmp), "%s%s=%s", n>0?", ":"",
                    gumbo_get_attribute(&focus_list[i]->styled->node->v.element.attributes, "value") ? "inp" : "inp",
                    input_buf[i]);
                strncat(vals, tmp, sizeof(vals)-strlen(vals)-1);
                n++;
            }
        }
        render_set_status("✓ 提交: %s", n > 0 ? vals : "(无输入框)");
    } else if (strcmp(btxt, "重置") == 0) {
        for (int i = 0; i < focus_count && i < *input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                input_buf[i][0] = '\0';
            }
        }
        render_set_status("✓ 重置: 所有输入框已清空");
    } else if (strcmp(btxt, "✕ 关闭") == 0) {
        render_set_status("✓ 关闭按钮被点击 (演示功能)");
    } else if (strcmp(btxt, "清空A") == 0) {
        if (focus_count >= 1) { input_buf[0][0] = '\0'; render_set_status("✓ 输入框 A 已清空"); }
    } else if (strcmp(btxt, "清空B") == 0) {
        if (focus_count >= 2) { input_buf[1][0] = '\0'; render_set_status("✓ 输入框 B 已清空"); }
    } else if (strcmp(btxt, "清空C") == 0) {
        if (focus_count >= 3) { input_buf[2][0] = '\0'; render_set_status("✓ 输入框 C 已清空"); }
    } else if (strcmp(btxt, "Go") == 0) {
        render_set_status("✓ Go button clicked");
    } else {
        render_set_status("✓ Button clicked: '%s'", btxt);
    }

    /* Refresh input text from buffers after action */
    for (int fi = 0; fi < focus_count && fi < *input_buf_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT &&
            n->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
            char buf[128];
            const char* val = input_buf[fi];
            if (val && val[0]) snprintf(buf, sizeof(buf), "%s", val);
            else snprintf(buf, sizeof(buf), " ");
            if (n->text_content) free(n->text_content);
            n->text_content = strdup(buf);
        }
    }
}

void render_run_ex(LayoutNode* root, KatanaStylesheet* css, StyledNode* styled_root) {
    int vw, vh;
    render_size(&vw, &vh);

    Screen* s = screen_create(vw, vh);
    int scroll_x = 0, scroll_y = 0;
    bool running = true;
    struct tb_event ev;

    /* Collect interactive nodes (INPUT/BUTTON) for focus */
    LayoutNode* focus_list[256];
    int focus_count = 0;
    int focus_idx = -1;
    {
        LayoutNode* stack[256]; int sp = 0;
        stack[sp++] = root;
        while (sp > 0 && focus_count < 256) {
            LayoutNode* n = stack[--sp];
            if (n->styled && n->styled->node &&
                n->styled->node->type == GUMBO_NODE_ELEMENT) {
                GumboTag t = n->styled->node->v.element.tag;
                if (t == GUMBO_TAG_INPUT || t == GUMBO_TAG_BUTTON) {
                    focus_list[focus_count] = n;
                    /* Seed input buffer from DOM attribute on first run */
                    if (t == GUMBO_TAG_INPUT && g_input_buf[focus_count][0] == 0) {
                        GumboAttribute* attr = gumbo_get_attribute(
                            &n->styled->node->v.element.attributes, "value");
                        if (attr && attr->value)
                            strncpy(g_input_buf[focus_count], attr->value, sizeof(g_input_buf[0])-1);
                    }
                    focus_count++;
                }
            }
            for (size_t ci = 0; ci < n->num_children && sp < 256; ci++)
                stack[sp++] = n->children[ci];
        }
    }
    g_input_buf_count = focus_count;

    /* Patch INPUT text_content from g_input_buf */
    for (int fi = 0; fi < focus_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT &&
            n->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
            char buf[128];
            if (g_input_buf[fi][0]) snprintf(buf, sizeof(buf), "%s", g_input_buf[fi]);
            else snprintf(buf, sizeof(buf), " ");
            if (n->text_content) free(n->text_content);
            n->text_content = strdup(buf);
        }
    }

    /* Rebuild state: when pseudo-class changes, rebuild layout tree */
    LayoutNode* current_root = root;
    KatanaStylesheet* saved_css = css;
    StyledNode* saved_st = styled_root;

    int vw_cache = vw, vh_cache = vh;

    while (running) {
        /* Render frame */
        screen_clear(s);
        s->scroll_x = scroll_x;
        s->scroll_y = scroll_y;
        screen_render_tree(s, current_root);

        /* Focus indicator — use absolute position */
        if (focus_idx >= 0 && focus_idx < focus_count) {
            LayoutNode* f = focus_list[focus_idx];
            int bx, by, bw, bh;
            node_abs_box(f, scroll_x, scroll_y, &bx, &by, &bw, &bh);
            if (bw > 2 && bh > 1 && by >= 0 && by < s->rows) {
                /* Draw focus outline: left/right vertical bars */
                for (int ri = 0; ri < bh && by + ri < s->rows; ri++) {
                    if (bx >= 0 && bx < s->cols)
                        { scr_set(s, bx, by+ri, ' '); scr_bg(s, bx, by+ri, 80,80,120); }
                    if (bx+bw-1 >= 0 && bx+bw-1 < s->cols)
                        { scr_set(s, bx+bw-1, by+ri, ' '); scr_bg(s, bx+bw-1, by+ri, 80,80,120); }
                }
                /* Top and bottom bar */
                for (int ci = 0; ci < bw && bx + ci < s->cols; ci++) {
                    if (by >= 0 && bx+ci >= 0) { scr_set(s, bx+ci, by, ' '); scr_bg(s, bx+ci, by, 80,80,120); }
                    int btm = by+bh-1;
                    if (btm < s->rows && bx+ci >= 0) { scr_set(s, bx+ci, btm, ' '); scr_bg(s, bx+ci, btm, 80,80,120); }
                }
            }
            if (f->styled && f->styled->node && f->styled->node->type == GUMBO_NODE_ELEMENT &&
                f->styled->node->v.element.tag == GUMBO_TAG_INPUT &&
                f->text_content) {
                /* Cursor at end of input value (no brackets) */
                int cu = bx + f->border_left + f->padding_left;  /* content start */
                if (focus_idx >= 0 && focus_idx < g_input_buf_count)
                    cu += uc_str_width(g_input_buf[focus_idx]); /* end of value */
                int cursor_row = by + f->border_top + f->padding_top; /* text row */
                if (cu >= 0 && cu < s->cols && cursor_row >= 0 && cursor_row < s->rows) {
                    scr_set(s, cu, cursor_row, ' '); scr_bg(s, cu, cursor_row, 200, 200, 80);
                    scr_set(s, cu, cursor_row, '|'); scr_fg(s, cu, cursor_row, 0, 0, 0);
                }
            }
        }

        /* Draw status bar at bottom */
        if (g_status_msg[0]) {
            int sb_row = s->rows - 1;
            for (int ci = 0; ci < s->cols; ci++) {
                scr_set(s, ci, sb_row, ' ');
                scr_bg(s, ci, sb_row, 30, 30, 50);
            }
            const char* sp = g_status_msg;
            int ci = 0;
            while (*sp && ci < s->cols) {
                uint32_t cp = uc_dec(&sp);
                if (cp == 0) break;
                int w = uc_wid((int)cp);
                if (ci + w <= s->cols) {
                    scr_set(s, ci, sb_row, cp);
                    scr_fg(s, ci, sb_row, 200, 200, 100);
                    scr_bold(s, ci, sb_row, true);
                    if (w == 2 && ci + 1 < s->cols) scr_set(s, ci + 1, sb_row, 0);
                }
                ci += w;
            }
        }

        screen_flush(s);

        /* Poll input */
        if (tb_poll_event(&ev) != TB_OK) {
            if (tb_last_errno() == EINTR) continue;
            break;
        }

        /* Track state changes */
        bool restyle = false;

        if (ev.type == TB_EVENT_MOUSE) {
            if (ev.key != TB_KEY_MOUSE_WHEEL_UP && ev.key != TB_KEY_MOUSE_WHEEL_DOWN) {
                /* Find node under mouse for hover (deepest match) */
                GumboNode* hover_node = NULL;
                LayoutNode* hstack[256]; int hsp = 0;
                hstack[hsp++] = current_root;
                int best_depth = -1;
                while (hsp > 0) {
                    LayoutNode* n = hstack[--hsp];
                    /* Push children */
                    for (size_t ci = 0; ci < n->num_children && hsp < 256; ci++)
                        hstack[hsp++] = n->children[ci];
                    /* Compute absolute position */
                    int hx, hy, hw, hh;
                    node_abs_box(n, scroll_x, scroll_y, &hx, &hy, &hw, &hh);
                    int depth = 0; { LayoutNode* pp = n; while (pp) { depth++; pp = pp->parent; } }
                    if (ev.x >= hx && ev.x < hx + hw && ev.y >= hy && ev.y < hy + hh && depth > best_depth) {
                        if (n->styled && n->styled->node &&
                            n->styled->node->type == GUMBO_NODE_ELEMENT) {
                            hover_node = n->styled->node;
                            best_depth = depth;
                        }
                    }
                }

                if (hover_node != g_interact_hover) {
                    g_interact_hover = hover_node;
                    restyle = true;
                }

                /* Left click → focus + active */
                if (ev.key == TB_KEY_MOUSE_LEFT) {
                    focus_idx = -1;
                    for (int fi = 0; fi < focus_count; fi++) {
                        LayoutNode* n = focus_list[fi];
                        int nx, ny, nw, nh;
                        node_abs_box(n, scroll_x, scroll_y, &nx, &ny, &nw, &nh);
                        if (ev.x >= nx && ev.x < nx + nw && ev.y >= ny && ev.y < ny + nh) {
                            focus_idx = fi; break;
                        }
                    }
                    if (g_interact_focus != (focus_idx >= 0 ? focus_list[focus_idx]->styled->node : NULL)) {
                        g_interact_focus = (focus_idx >= 0 && focus_list[focus_idx]->styled) ? focus_list[focus_idx]->styled->node : NULL;
                        restyle = true;
                    }
                    g_interact_active = g_interact_focus;
                    restyle = true;
                }

                /* Left click on BUTTON: trigger click action */
                if (ev.key == TB_KEY_MOUSE_LEFT) {
                    LayoutNode* clicked_btn = NULL;
                    for (int fi = 0; fi < focus_count; fi++) {
                        LayoutNode* n = focus_list[fi];
                        int nx, ny, nw, nh;
                        node_abs_box(n, scroll_x, scroll_y, &nx, &ny, &nw, &nh);
                        if (ev.x >= nx && ev.x < nx + nw && ev.y >= ny && ev.y < ny + nh) {
                            if (n->styled && n->styled->node &&
                                n->styled->node->type == GUMBO_NODE_ELEMENT &&
                                n->styled->node->v.element.tag == GUMBO_TAG_BUTTON) {
                                clicked_btn = n;
                            }
                            break;
                        }
                    }
                    if (clicked_btn) {
                        handle_button_click(clicked_btn, focus_list, focus_count,
                                            g_input_buf, &g_input_buf_count,
                                            current_root, restyle);
                    }
                }
            }
        }

        if (ev.type == TB_EVENT_KEY) {
            if (ev.ch == 'q' || ev.ch == 'Q' || ev.key == TB_KEY_ESC ||
                ev.key == TB_KEY_CTRL_C || ev.key == TB_KEY_CTRL_D) {
                running = false; continue;
            }

            /* ── Input editing: when an INPUT is focused ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                char* buf = g_input_buf[focus_idx];
                size_t blen = strlen(buf);

                if (ev.key == TB_KEY_BACKSPACE2 || ev.key == TB_KEY_BACKSPACE) {
                    if (blen > 0) { buf[blen-1] = '\0'; }
                } else if (ev.key == TB_KEY_DELETE) {
                    /* do nothing for delete in this simple TUI */
                } else if (ev.ch >= 0x20 && ev.ch <= 0x7e) {
                    if (blen < (int)sizeof(g_input_buf[0]) - 2) {
                        char add[8];
                        int nch = uc_enc(ev.ch, add); add[nch] = '\0';
                        strcat(buf, add);
                    }
                } else if (ev.key == TB_KEY_SPACE) {
                    if (blen < (int)sizeof(g_input_buf[0]) - 2) {
                        strcat(buf, " ");
                    }
                } else if (ev.key == TB_KEY_ENTER) {
                    /* Enter on INPUT: just move to next focusable */
                    if (focus_count > 0) {
                        focus_idx = (focus_idx + 1) % focus_count;
                        if (focus_idx >= 0 && focus_list[focus_idx]->styled)
                            g_interact_focus = focus_list[focus_idx]->styled->node;
                        else g_interact_focus = NULL;
                        restyle = true;
                    }
                    continue;
                } else {
                    /* Fall through to scroll keys etc */
                    goto handle_scroll_keys;
                }

                /* Rebuild the input node's text (width stays as set by CSS/layout) */
                LayoutNode* inp = focus_list[focus_idx];
                char txt[128];
                snprintf(txt, sizeof(txt), "%s", buf);
                if (inp->text_content) free(inp->text_content);
                inp->text_content = strdup(txt);
                continue;
            }

            /* ── Button click: Enter/Space on focused BUTTON ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                (ev.key == TB_KEY_ENTER || ev.key == TB_KEY_SPACE) &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_BUTTON) {
                handle_button_click(focus_list[focus_idx], focus_list, focus_count,
                                    g_input_buf, &g_input_buf_count,
                                    current_root, restyle);
                continue;
            }

handle_scroll_keys:
            /* Scroll keys */
            if (ev.key == TB_KEY_ARROW_UP) { scroll_y--; }
            else if (ev.key == TB_KEY_ARROW_DOWN) { scroll_y++; }
            else if (ev.key == TB_KEY_ARROW_LEFT) { scroll_x--; }
            else if (ev.key == TB_KEY_ARROW_RIGHT) { scroll_x++; }
            else if (ev.key == TB_KEY_PGUP) { scroll_y -= vh; }
            else if (ev.key == TB_KEY_PGDN) { scroll_y += vh; }
            else if (ev.key == TB_KEY_HOME) { scroll_x = 0; scroll_y = 0; }
            else if (ev.key == TB_KEY_END) { scroll_y = 999999; }
            else if (ev.key == TB_KEY_TAB && focus_count > 0) {
                focus_idx = (focus_idx + 1) % focus_count;
                if (focus_idx >= 0 && focus_list[focus_idx]->styled) {
                    g_interact_focus = focus_list[focus_idx]->styled->node;
                } else {
                    g_interact_focus = NULL;
                }
                render_set_status("Tab: focus #%d (%s)", focus_idx,
                    focus_list[focus_idx]->styled &&
                    focus_list[focus_idx]->styled->node &&
                    focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT
                    ? gumbo_normalized_tagname(focus_list[focus_idx]->styled->node->v.element.tag)
                    : "?");
                restyle = true;
                continue;
            }

            /* Active: key press sets :active on focused element */
            if (g_interact_focus && g_interact_active != g_interact_focus) {
                g_interact_active = g_interact_focus;
                restyle = true;
            }
        }

        /* Mouse wheel — don't continue, fall through to clamp */
        if (ev.type == TB_EVENT_MOUSE) {
            if (ev.key == TB_KEY_MOUSE_WHEEL_UP) { scroll_y -= 3; }
            else if (ev.key == TB_KEY_MOUSE_WHEEL_DOWN) { scroll_y += 3; }
        }

        /* Key release → clear :active */
        if (ev.type == TB_EVENT_KEY && g_interact_active) {
            /* On key release (any key), clear active */
            /* termbox2 doesn't distinguish press/release clearly,
               so we clear active on any non-Tab keypress instead */
            if (ev.key != TB_KEY_TAB) {
                /* keep active briefly - clear on NEXT poll cycle */
            }
        }

        /* Resize */
        if (ev.type == TB_EVENT_RESIZE) {
            vw = ev.w; vh = ev.h;
            screen_free(s); s = screen_create(vw, vh);
            vw_cache = vw; vh_cache = vh;
            /* Restyle on resize too (rebuild layout) */
            restyle = true;
        }

        /* If hover/focus/active changed, rebuild styles and layout */
        if (restyle && saved_css && saved_st) {
            g_interact_hover = g_interact_hover;
            /* Recompute styles for affected nodes */
            if (g_interact_hover) {
                StyledNode* sn = find_styled_node(saved_st, g_interact_hover);
                if (sn) {
                    recompute_style_subtree(sn, saved_css, NULL);
                }
            }
            if (g_interact_focus && g_interact_focus != g_interact_hover) {
                StyledNode* sn = find_styled_node(saved_st, g_interact_focus);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            /* Rebuild layout tree */
            LayoutNode* new_root = build_layout_tree(saved_st, vw_cache, vh_cache);
            if (new_root) {
                LayoutNode* old_root = current_root;
                current_root = new_root;
                /* Don't free old root — it's being used in render path.
                   Actually it's not, since we clear and re-render. */
            }
            /* Re-collect focus list */
            focus_count = 0;
            {
                LayoutNode* stack[256]; int sp = 0;
                stack[sp++] = current_root;
                while (sp > 0 && focus_count < 256) {
                    LayoutNode* n = stack[--sp];
                    if (n->styled && n->styled->node &&
                        n->styled->node->type == GUMBO_NODE_ELEMENT) {
                        GumboTag t = n->styled->node->v.element.tag;
                        if (t == GUMBO_TAG_INPUT || t == GUMBO_TAG_BUTTON) {
                            focus_list[focus_count++] = n;
                        }
                    }
                    for (size_t ci = 0; ci < n->num_children && sp < 256; ci++)
                        stack[sp++] = n->children[ci];
                }
            }
            /* Re-patch INPUT text_content from g_input_buf */
            for (int fi = 0; fi < focus_count && fi < g_input_buf_count; fi++) {
                LayoutNode* n = focus_list[fi];
                if (n->styled && n->styled->node &&
                    n->styled->node->type == GUMBO_NODE_ELEMENT &&
                    n->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                    char buf[128];
                    const char* val = g_input_buf[fi];
                    if (val && val[0]) snprintf(buf, sizeof(buf), "%s", val);
                    else snprintf(buf, sizeof(buf), " ");
                    if (n->text_content) free(n->text_content);
                    n->text_content = strdup(buf);
                }
            }
        }

        /* Clamp scroll */
        s->scroll_x = scroll_x; s->scroll_y = scroll_y;
        screen_scroll_clamp(s, current_root->width, current_root->height);
        scroll_x = s->scroll_x; scroll_y = s->scroll_y;
    }

    screen_free(s);
}

#endif /* RENDER_RUN_IMPLEMENTATION */
#endif /* RENDER_H */

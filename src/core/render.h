#ifndef RENDER_H
#define RENDER_H

#include "layout.h"
#include <stdbool.h>
#include <stdint.h>

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

/** Run a full interactive loop using termbox2 + Screen buffer internally.
 *  Arrow/PgUp/Dn/Home/End scroll, mouse wheel scroll, q=quit.
 *  Must call render_init() before and render_shutdown() after. */
void render_run(LayoutNode* root);

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

static void draw_border(Screen* s, int x, int y, int w, int h, ResolvedColor color) {
    if (w < 2 || h < 2) return;
    int r = x + w - 1, b = y + h - 1;
    scr_set(s,x,y,'+'); scr_set(s,r,y,'+'); scr_set(s,x,b,'+'); scr_set(s,r,b,'+');
    for (int cx = x+1; cx < r; cx++) { scr_set(s,cx,y,'-'); scr_set(s,cx,b,'-'); }
    for (int cy = y+1; cy < b; cy++) { scr_set(s,x,cy,'|'); scr_set(s,r,cy,'|'); }
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
        ResolvedColor bc = n->border_color; if (!bc.valid && n->color.valid) bc = n->color; bc.valid = true;
        draw_border(s, scr_x, scr_y, tw, th, bc);
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
                if (cx - lnx + word_w > mw && cx > lnx) { cy++; cx = lnx; }
                if (cy >= s->rows) break;

                /* Render word characters */
                while (p < we && cy < s->rows) {
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
                    if (cx - lnx >= mw && p < we) { cy++; cx = lnx; }
                }

                /* Skip trailing space */
                if (*p == ' ') { cx++; p++; }
                /* Handle newlines */
                while (*p == '\n') { cy++; cx = lnx; p++; }
            }
        }
    }
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

void render_run(LayoutNode* root) {
    int vw, vh;
    render_size(&vw, &vh);

    Screen* s = screen_create(vw, vh);
    int scroll_x = 0, scroll_y = 0;
    bool running = true;
    struct tb_event ev;

    /* Input interaction state */
    LayoutNode* focused = NULL;       /* currently focused input/button */
    char input_buf[256];             /* for typing into <input> */
    int input_pos = 0;               /* cursor position */

    while (running) {
        /* Render frame */
        screen_clear(s);
        s->scroll_x = scroll_x;
        s->scroll_y = scroll_y;
        screen_render_tree(s, root);
        /* Draw focus indicator on focused input */
        if (focused && focused->text_content && input_pos >= 0) {
            /* Input field uses [value] format; cursor pos inside brackets */
            int bx = focused->x - focused->border_left - focused->padding_left - scroll_x;
            int by = focused->y - focused->border_top - focused->padding_top - scroll_y;
            int cu = bx + 1 + input_pos; /* +1 for '[' */
            if (cu >= 0 && cu < s->cols && by >= 0 && by < s->rows) {
                scr_set(s, cu, by, '|');
                scr_fg(s, cu, by, 255, 255, 0);
            }
        }
        screen_flush(s);

        /* Poll input */
        if (tb_poll_event(&ev) != TB_OK) {
            if (tb_last_errno() == EINTR) continue;
            break;
        }

        switch (ev.type) {
        case TB_EVENT_KEY:
            if (ev.ch == 'q' || ev.ch == 'Q') {
                running = false;
            } else if (ev.key == TB_KEY_TAB) {
                /* Cycle focus: collect all interactive nodes */
                static LayoutNode* prev_focused = NULL; /* hack: just toggle */
                focused = focused ? NULL : root; /* simple: focus root then... */
            } else if (focused && focused->text_content && focused->text_content[0] == '[') {
                /* Editing <input> value */
                if (ev.key == TB_KEY_BACKSPACE || ev.key == TB_KEY_BACKSPACE2) {
                    if (input_pos > 0) {
                        int cur_len = (int)strlen(focused->text_content);
                        if (cur_len > 3) {
                            memmove(&focused->text_content[1 + input_pos - 1],
                                    &focused->text_content[1 + input_pos],
                                    cur_len - 1 - input_pos);
                            focused->text_content[cur_len - 1] = '\0';
                        } else {
                            focused->text_content[1] = ' ';
                        }
                        input_pos--;
                    }
                } else if (ev.ch >= 32 && ev.ch < 127) {
                    int cur_len = (int)strlen(focused->text_content);
                    if (cur_len < (int)sizeof(input_buf) - 2) {
                        memmove(&focused->text_content[1 + input_pos + 1],
                                &focused->text_content[1 + input_pos],
                                cur_len - 1 - input_pos + 1); /* +1 for null */
                        focused->text_content[1 + input_pos] = (char)ev.ch;
                        input_pos++;
                    }
                }
            } else if (focused && focused->text_content && strcmp(focused->text_content, "[  ]") == 0) {
                /* Click on input with no value — start fresh */
                focused = focused; /* keep focus */
            } else if (ev.key == TB_KEY_ARROW_UP) {
                scroll_y--;
            } else if (ev.key == TB_KEY_ARROW_DOWN) {
                scroll_y++;
            } else if (ev.key == TB_KEY_ARROW_LEFT) {
                scroll_x--;
            } else if (ev.key == TB_KEY_ARROW_RIGHT) {
                scroll_x++;
            } else if (ev.key == TB_KEY_PGUP) {
                scroll_y -= vh;
            } else if (ev.key == TB_KEY_PGDN) {
                scroll_y += vh;
            } else if (ev.key == TB_KEY_HOME) {
                scroll_x = 0; scroll_y = 0;
            } else if (ev.key == TB_KEY_END) {
                scroll_y = 999999;
            }
            break;

        case TB_EVENT_MOUSE:
            if (ev.key == TB_KEY_MOUSE_WHEEL_UP) {
                scroll_y -= 3;
            } else if (ev.key == TB_KEY_MOUSE_WHEEL_DOWN) {
                scroll_y += 3;
            } else if (ev.key == TB_KEY_MOUSE_LEFT) {
                /* Click: find node at (ev.x, ev.y) */
                LayoutNode* hit = NULL;
                /* Find first matching input or button at click position */
                /* Walk the tree using a simple recursive search */
                {
                    LayoutNode* stack[256];
                    int sp = 0;
                    stack[sp++] = root;
                    while (sp > 0) {
                        LayoutNode* n = stack[--sp];
                        int nx = n->x - n->border_left - n->padding_left - scroll_x;
                        int ny = n->y - n->border_top - n->padding_top - scroll_y;
                        int nw = n->width + n->padding_left + n->padding_right +
                                  n->border_left + n->border_right;
                        int nh = n->height + n->padding_top + n->padding_bottom +
                                  n->border_top + n->border_bottom;
                        if (ev.x >= nx && ev.x < nx + nw && ev.y >= ny && ev.y < ny + nh) {
                            if (n->styled && n->styled->node &&
                                n->styled->node->type == GUMBO_NODE_ELEMENT) {
                                GumboTag t = n->styled->node->v.element.tag;
                                if (t == GUMBO_TAG_INPUT || t == GUMBO_TAG_BUTTON) {
                                    hit = n; break;
                                }
                            }
                        }
                        for (size_t ci = 0; ci < n->num_children; ci++)
                            stack[sp++] = n->children[ci];
                    }
                }
                if (hit) {
                    focused = hit;
                    /* Initialize input buffer from text content */
                    if (hit->text_content && hit->text_content[0] == '[') {
                        input_pos = 0;
                        /* Count chars between [ and ] */
                        const char* content = hit->text_content + 1;
                        input_pos = (int)strlen(content) - 1;
                        if (input_pos < 0) input_pos = 0;
                    }
                } else {
                    focused = NULL;
                }
            }
            break;

        case TB_EVENT_RESIZE:
            vw = ev.w; vh = ev.h;
            screen_free(s);
            s = screen_create(vw, vh);
            break;
        }

        /* Clamp scroll within content bounds */
        s->scroll_x = scroll_x;
        s->scroll_y = scroll_y;
        screen_scroll_clamp(s, root->width, root->height);
        scroll_x = s->scroll_x;
        scroll_y = s->scroll_y;
    }

    screen_free(s);
}

#endif /* RENDER_RUN_IMPLEMENTATION */
#endif /* RENDER_H */

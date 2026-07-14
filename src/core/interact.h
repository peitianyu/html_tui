#ifndef INTERACT_H
#define INTERACT_H

#include "render.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Callback Interface ======================== */

/** Forward declaration */
typedef struct InteractCallbacks InteractCallbacks;

/**
 * Button click callback.
 * @param btn_text   trimmed text content of the clicked button
 * @param focus_idx  index of the clicked button in focus_list
 *                   (focus_list[focus_idx] is the button's LayoutNode)
 * @param focus_list array of all focusable nodes (INPUT/BUTTON)
 * @param focus_count number of focusable nodes
 * @param input_buf  input value buffers, indexed same as focus_list
 * @param input_buf_count number of input buffers
 * @param cb         the callbacks struct (layout_root is available for queries)
 * @return true if a layout rebuild is needed
 */
typedef bool (*InteractButtonClickFn)(const char* btn_text, int focus_idx,
                                       LayoutNode** focus_list, int focus_count,
                                       char input_buf[][4096], int input_buf_count,
                                       InteractCallbacks* cb);

/**
 * Focus change callback (optional).
 * Called when Tab or mouse changes focus. Set status_msg as desired.
 */
typedef void (*InteractFocusChangeFn)(int focus_idx, const char* tag_name,
                                       InteractCallbacks* cb);

/**
 * Global key handler (optional).
 * Called for every key event before default processing.
 * Return true if key was handled (interact_run skips defaults).
 * Return false to let interact_run process it normally.
 *
 * Typical implementation: check ev->ch / ev->key, set cb->quit_flag
 * to exit the loop.
 */
typedef bool (*InteractKeyFn)(struct tb_event* ev, InteractCallbacks* cb);

/** Interact callbacks — user fills this in to customize behavior */
struct InteractCallbacks {
    /* Status message buffer. interact_run draws this on the status bar. */
    char status_msg[64];

    /* Button click handler. Required. */
    InteractButtonClickFn on_button_click;

    /* Focus change handler. Optional (NULL = auto status). */
    InteractFocusChangeFn on_focus_change;

    /* Global key handler. Optional (NULL = default keys handled internally). */
    InteractKeyFn on_key;

    /* Opaque user data pointer */
    void* user_data;

    /* ─── Runtime fields (set / read by interact_run each frame) ─── */

    /** Current layout tree root. Use with find_node_by_*() to locate elements. */
    LayoutNode* layout_root;

    /** Set to true from on_button_click or on_key to exit the interactive loop. */
    bool quit_flag;

    /**
     * Set to a file path to switch to a new page.
     * The main loop will read this file, parse HTML/CSS, and rebuild everything.
     * Cleared automatically after switching.
     */
    char switch_page[256];

    /* ─── Text overrides (persist across layout rebuilds) ───
     * Use node_override_text() to register a change; interact_run
     * re-applies it automatically after every rebuild.
     * This is how input_buf survives rebuilds — but for any element. */
    struct { char id[64]; char text[128]; } text_overrides[16];
    int text_override_count;

    /* ─── Scrollbar configuration ─── */
    /**
     * Set to true to show visual side scrollbars (vertical on right edge, horizontal above status bar).
     * Scrollbars only appear when content exceeds viewport size.
     * Supports click-to-jump and drag-to-scroll interactions.
     */
    bool show_scrollbars;
};

/* ======================== Element Query Helpers ======================== */

/**
 * Find a LayoutNode by its HTML id attribute (DFS).
 * Returns NULL if not found.
 */
LayoutNode* find_node_by_id(LayoutNode* root, const char* id);

/**
 * Find the nth (1-based) occurrence of a tag in the layout tree.
 * Example: find_nth_node_by_tag(root, GUMBO_TAG_DIV, 2) → 2nd <div>
 */
LayoutNode* find_nth_node_by_tag(LayoutNode* root, GumboTag tag, int nth);

/**
 * Find nodes by class name (DFS). Returns the first match.
 */
LayoutNode* find_node_by_class(LayoutNode* root, const char* class_name);

/**
 * Get the text content pointer of a node. Returns "" if NULL.
 */
const char* node_get_text(LayoutNode* n);

/**
 * Set the text content of a node (strdup's internally,
 * frees old content). Takes effect immediately on-screen.
 */
void node_set_text(LayoutNode* n, const char* txt);

/**
 * Set text by element id, and persist across layout rebuilds.
 * Use this instead of node_set_text() when the change should survive
 * hover/focus/active style recalculations.
 * 
 * @param cb  callbacks (must not be NULL)
 * @param id  HTML id attribute of the target element
 * @param txt new text content
 */
void node_override_text(InteractCallbacks* cb, const char* id, const char* txt);

/**
 * Toggle visibility (DISPLAY_NONE / DISPLAY_BLOCK).
 * NOTE: this only changes the LayoutNode display field.
 * To take effect, the callback must return true so the layout
 * tree is rebuilt.
 */
void node_set_visible(LayoutNode* n, bool visible);

/**
 * Run interactive TUI loop.
 * 
 * @param root        layout tree root
 * @param css         parsed CSS stylesheet (for :hover/:focus/:active restyling)
 * @param styled_root root of the style tree
 * @param cb          callbacks for user-defined behaviour
 */
void interact_run(LayoutNode* root, KatanaStylesheet* css,
                  StyledNode* styled_root,
                  InteractCallbacks* cb);

#ifdef __cplusplus
}
#endif

/* ==================================================================== */
/* IMPLEMENTATION                                                       */
/* ==================================================================== */
#ifdef INTERACT_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

/* Global details open/close state tracker (by element id, survives rebuilds) */
#define MAX_DETAILS_STATES 32
static struct { char id[64]; bool open; } g_details_states[MAX_DETAILS_STATES];
static int g_details_count = 0;

static void set_details_state(const char* id, bool open) {
    for (int i = 0; i < g_details_count; i++) {
        if (strcmp(g_details_states[i].id, id) == 0) {
            g_details_states[i].open = open;
            return;
        }
    }
    if (g_details_count < MAX_DETAILS_STATES) {
        strncpy(g_details_states[g_details_count].id, id, 63);
        g_details_states[g_details_count].open = open;
        g_details_count++;
    }
}

static bool get_details_state(const char* id) {
    for (int i = 0; i < g_details_count; i++) {
        if (strcmp(g_details_states[i].id, id) == 0)
            return g_details_states[i].open;
    }
    return true; /* default: open */
}

/* ─── Element query helpers ──────────────────────────────────────────── */

LayoutNode* find_node_by_id(LayoutNode* root, const char* id) {
    if (!root || !root->styled || !root->styled->node ||
        root->styled->node->type != GUMBO_NODE_ELEMENT)
        goto find_by_id_children;
    GumboAttribute* attr = gumbo_get_attribute(
        &root->styled->node->v.element.attributes, "id");
    if (attr && attr->value && strcmp(attr->value, id) == 0)
        return root;
find_by_id_children:
    for (size_t i = 0; i < root->num_children; i++) {
        LayoutNode* found = find_node_by_id(root->children[i], id);
        if (found) return found;
    }
    return NULL;
}

LayoutNode* find_nth_node_by_tag(LayoutNode* root, GumboTag tag, int nth) {
    if (!root || !root->styled || !root->styled->node ||
        root->styled->node->type != GUMBO_NODE_ELEMENT)
        goto find_tag_children;
    if (root->styled->node->v.element.tag == tag) {
        if (--nth == 0) return root;
    }
find_tag_children:
    for (size_t i = 0; i < root->num_children; i++) {
        LayoutNode* found = find_nth_node_by_tag(root->children[i], tag, nth);
        if (found) return found;
    }
    return NULL;
}

LayoutNode* find_node_by_class(LayoutNode* root, const char* class_name) {
    if (!root || !root->styled || !root->styled->node ||
        root->styled->node->type != GUMBO_NODE_ELEMENT)
        goto find_class_children;
    GumboAttribute* attr = gumbo_get_attribute(
        &root->styled->node->v.element.attributes, "class");
    if (attr && attr->value) {
        /* Simple word-boundary match */
        const char* p = attr->value;
        size_t nlen = strlen(class_name);
        while (*p) {
            while (*p == ' ') p++;
            if (strncmp(p, class_name, nlen) == 0) {
                char c = p[nlen];
                if (c == '\0' || c == ' ') return root;
            }
            while (*p && *p != ' ') p++;
        }
    }
find_class_children:
    for (size_t i = 0; i < root->num_children; i++) {
        LayoutNode* found = find_node_by_class(root->children[i], class_name);
        if (found) return found;
    }
    return NULL;
}

const char* node_get_text(LayoutNode* n) {
    return (n && n->text_content) ? n->text_content : "";
}

void node_set_text(LayoutNode* n, const char* txt) {
    if (!n) return;
    if (n->text_content) free(n->text_content);
    n->text_content = txt ? strdup(txt) : NULL;
}

void node_override_text(InteractCallbacks* cb, const char* id, const char* txt) {
    if (!cb || !id || !txt) return;
    /* Apply immediately */
    LayoutNode* n = find_node_by_id(cb->layout_root, id);
    node_set_text(n, txt);
    /* Register override so it survives rebuilds */
    for (int i = 0; i < cb->text_override_count; i++) {
        if (strcmp(cb->text_overrides[i].id, id) == 0) {
            strncpy(cb->text_overrides[i].text, txt, sizeof(cb->text_overrides[i].text)-1);
            return;
        }
    }
    if (cb->text_override_count < 16) {
        int i = cb->text_override_count++;
        strncpy(cb->text_overrides[i].id, id, sizeof(cb->text_overrides[i].id)-1);
        strncpy(cb->text_overrides[i].text, txt, sizeof(cb->text_overrides[i].text)-1);
    }
}

void node_set_visible(LayoutNode* n, bool visible) {
    if (!n) return;
    n->display = visible ? DISPLAY_BLOCK : DISPLAY_NONE;
}

/* ─── Focus list collection helper ──────────────────────────── */
/** Collect all focusable nodes (INPUT/BUTTON) from the layout tree.
 *  Returns the number collected, or 0 on empty. */
static int collect_focus_list(LayoutNode* root, LayoutNode** focus_list, int max_count) {
    int count = 0;
    LayoutNode* stack[256]; int sp = 0;
    stack[sp++] = root;
    while (sp > 0 && count < max_count) {
        LayoutNode* n = stack[--sp];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT) {
            GumboTag t = n->styled->node->v.element.tag;
            if (t == GUMBO_TAG_INPUT || t == GUMBO_TAG_BUTTON ||
                t == GUMBO_TAG_SUMMARY || t == GUMBO_TAG_TEXTAREA ||
                t == GUMBO_TAG_SELECT) {
                focus_list[count++] = n;
            }
        }
        for (size_t ci = 0; ci < n->num_children && sp < 256; ci++)
            stack[sp++] = n->children[ci];
    }
    return count;
}

/** Patch INPUT/TEXTAREA text_content from saved input buffers after a layout rebuild. */
static void patch_input_text(LayoutNode** focus_list, int focus_count,
                              char input_buf[][4096], int input_buf_count) {
    for (int fi = 0; fi < focus_count && fi < input_buf_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT &&
            (n->styled->node->v.element.tag == GUMBO_TAG_INPUT ||
             n->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA)) {
            char buf[4096];
            const char* val = input_buf[fi];
            if (val && val[0]) snprintf(buf, sizeof(buf), "%s", val);
            else snprintf(buf, sizeof(buf), " ");
            if (n->text_content) free(n->text_content);
            n->text_content = strdup(buf);
        }
    }
}

/* ─── Interactive loop ──────────────────────────────────────────── */
void interact_run(LayoutNode* root, KatanaStylesheet* css,
                  StyledNode* styled_root,
                  InteractCallbacks* cb)
{
    int vw, vh;
    render_size(&vw, &vh);

    Screen* s = screen_create(vw, vh);
    int scroll_x = 0, scroll_y = 0;
    bool running = true;
    struct tb_event ev;

    /* Input buffers: keyed by the same order as focus_list */
    char input_buf[256][4096] = {{{0}}};
    int  input_cursor[256] = {0}; /* cursor position within each input */
    int  textarea_target_col[256] = {0}; /* target visual column for textarea up/down nav */
    int  textarea_scroll_y[256] = {0}; /* vertical scroll offset for textarea content */
    int  textarea_scroll_x[256] = {0}; /* horizontal scroll offset for textarea content */
    int  input_buf_count = 0;

    /* ─── Select popup state ─── */
    int  select_popup_active = 0;       /* 0=closed, 1=open */
    int  select_popup_focus_idx = -1;   /* focus_list index of the <select> */
    int  select_popup_sel = 0;          /* currently highlighted option index */
    int  select_popup_count = 0;        /* total option count */
    char select_popup_vals[32][64];     /* option display labels */
    int  select_popup_scroll = 0;       /* vertical scroll offset in popup list */

    /* Collect interactive nodes (INPUT/BUTTON) for focus */
    LayoutNode* focus_list[256];
    int focus_count = 0;
    int focus_idx = -1;

    focus_count = collect_focus_list(root, focus_list, 256);
    /* Seed input buffer from DOM attribute on first run */
    for (int fi = 0; fi < focus_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT &&
            input_buf[fi][0] == 0) {
            GumboTag t = n->styled->node->v.element.tag;
            if (t == GUMBO_TAG_INPUT) {
                GumboAttribute* attr = gumbo_get_attribute(
                    &n->styled->node->v.element.attributes, "value");
                if (attr && attr->value)
                    strncpy(input_buf[fi], attr->value, sizeof(input_buf[0])-1);
            } else if (t == GUMBO_TAG_TEXTAREA) {
                /* Textarea: use text_content */
                if (n->text_content && n->text_content[0] && n->text_content[0] != ' ')
                    strncpy(input_buf[fi], n->text_content, sizeof(input_buf[0])-1);
            }
        }
    }
    input_buf_count = focus_count;

    /* Patch INPUT text_content from input_buf */
    patch_input_text(focus_list, focus_count, input_buf, input_buf_count);

    /* Rebuild state */
    LayoutNode* current_root = root;
    KatanaStylesheet* saved_css = css;
    StyledNode* saved_st = styled_root;

    int vw_cache = vw, vh_cache = vh;

    /* Track mouse position for :hover leave detection */
    int last_mouse_x = -1, last_mouse_y = -1;
    int mouse_idle_count = 0;

    /* Track scrollbar drag state: textarea index being dragged, -1 = not dragging */
    int textarea_scrollbar_drag = -1;
    /* Global scrollbar drag state: -1 = not dragging, 0/1 = dragging */
    int global_sb_drag_v = -1;
    int global_sb_drag_h = -1;

    /* Track previous hover/active for style recomputation */
    GumboNode* prev_hover = NULL;
    GumboNode* prev_active = NULL;

    while (running) {
        /* ── Check quit request from callback ── */
        if (cb && cb->quit_flag) { running = false; break; }

        /* ── Sync runtime info to callbacks ── */
        if (cb) cb->layout_root = current_root;

        /* ── Render frame ── */
        /* Sync textarea scroll offsets to layout node for render clipping */
        if (focus_idx >= 0 && focus_idx < focus_count) {
            LayoutNode* f = focus_list[focus_idx];
            if (f->styled && f->styled->node &&
                f->styled->node->type == GUMBO_NODE_ELEMENT &&
                f->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
                f->content_scroll_y = textarea_scroll_y[focus_idx];
                f->content_scroll_x = textarea_scroll_x[focus_idx];
            }
        }
        screen_clear(s);
        s->scroll_x = scroll_x;
        s->scroll_y = scroll_y;
        screen_render_tree(s, current_root);

        /* Focus indicator */
        if (focus_idx >= 0 && focus_idx < focus_count) {
            LayoutNode* f = focus_list[focus_idx];
            int bx, by, bw, bh;
            node_abs_box(f, scroll_x, scroll_y, &bx, &by, &bw, &bh);
            if (bw > 0 && bh >= 1 && by >= 0 && by < s->rows) {
                /*
                 * Highlight the entire element box area uniformly with a
                 * focus background colour.  We use screen_scr_bg() only
                 * (NOT screen_scr_set), so existing characters — text,
                 * border glyphs, spaces — remain fully visible while the
                 * background changes to indicate focus.
                 *
                 * This avoids two problems:
                 *  1) Edge-only bars leave the centre colour mismatched
                 *     when the CSS :active/:hover background differs.
                 *  2) screen_scr_set(…,' ') would erase button text or
                 *     border characters on elements without padding.
                 */
                int bg_r = 80, bg_g = 80, bg_b = 120;
                for (int ri = 0; ri < bh && by + ri < s->rows; ri++) {
                    for (int ci = 0; ci < bw && bx + ci < s->cols; ci++) {
                        if (by+ri >= 0 && bx+ci >= 0)
                            screen_scr_bg(s, bx+ci, by+ri, bg_r, bg_g, bg_b);
                    }
                }
            }
            if (f->styled && f->styled->node && f->styled->node->type == GUMBO_NODE_ELEMENT &&
                f->text_content &&
                (f->styled->node->v.element.tag == GUMBO_TAG_INPUT ||
                 f->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA)) {
                int cursor_pos = (focus_idx >= 0 && focus_idx < input_buf_count) ? input_cursor[focus_idx] : 0;
                int cu = bx + f->border_left + f->padding_left;
                int cursor_row = by + f->border_top + f->padding_top;

                if (f->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
                    /* For textarea, compute cursor row and column from \n-delimited text */
                    const char* txt = input_buf[focus_idx];
                    int bo = 0; /* byte offset in input_buf */
                    int line_idx = 0;
                    int line_start_bo = 0;
                    /* Find which line the cursor falls on */
                    while (bo < cursor_pos && txt[bo]) {
                        if (txt[bo] == '\n') {
                            line_idx++;
                            line_start_bo = bo + 1;
                        }
                        bo++;
                    }
                    int col_in_line = cursor_pos - line_start_bo;
                    int visual_col = uc_str_width_len(txt + line_start_bo, col_in_line);
                    cu += visual_col - textarea_scroll_x[focus_idx];
                    cursor_row += line_idx - textarea_scroll_y[focus_idx];
                } else {
                    /* INPUT: single-line, all on one row */
                    if (cursor_pos > 0) cu += uc_str_width_len(input_buf[focus_idx], cursor_pos);
                }

                {
                    /* Content area bounds — prevent cursor drawing outside textarea/input */
                    int ca_left = bx + f->border_left + f->padding_left;
                    int ca_top  = by + f->border_top  + f->padding_top;
                    int ca_right = ca_left + f->width;
                    int ca_bottom = ca_top + f->height;
                    if (cu >= 0 && cu < s->cols && cursor_row >= 0 && cursor_row < s->rows &&
                        cu >= ca_left && cu < ca_right &&
                        cursor_row >= ca_top && cursor_row < ca_bottom) {
                        /* Terminal-style block cursor: invert fg↔bg colors */
                        Cell* cur_cell = &s->cells[cursor_row * s->cols + cu];
                        int cf_r = cur_cell->fg_r, cf_g = cur_cell->fg_g, cf_b = cur_cell->fg_b;
                        int cb_r = cur_cell->bg_r, cb_g = cur_cell->bg_g, cb_b = cur_cell->bg_b;
                        screen_scr_fg(s, cu, cursor_row, cb_r, cb_g, cb_b);
                        screen_scr_bg(s, cu, cursor_row, cf_r, cf_g, cf_b);
                    }
                }
            }
        }

        /* Draw global scrollbars: hscroll first so vscroll draws on top at corner */
        if (cb && cb->show_scrollbars && current_root) {
            screen_draw_hscrollbar(s, current_root->width, scroll_x);
            screen_draw_vscrollbar(s, current_root->height, scroll_y);
        }

        /* Draw status bar at bottom */
        if (cb && cb->status_msg[0]) {
            int sb_row = s->rows - 1;
            for (int ci = 0; ci < s->cols; ci++) {
                screen_scr_set(s, ci, sb_row, ' ');
                screen_scr_bg(s, ci, sb_row, 30, 30, 50);
            }
            const char* sp = cb->status_msg;
            int ci = 0;
            while (*sp && ci < s->cols) {
                uint32_t cp = uc_dec(&sp);
                if (cp == 0) break;
                int w = uc_wid((int)cp);
                if (ci + w <= s->cols) {
                    screen_scr_set(s, ci, sb_row, cp);
                    screen_scr_fg(s, ci, sb_row, 200, 200, 100);
                    if (w == 2 && ci + 1 < s->cols) screen_scr_set(s, ci + 1, sb_row, 0);
                }
                ci += w;
            }
            /* Scroll indicators on the right side of status bar */
            if (scroll_y > 0) {
                int ri = s->cols - 3;
                if (ri > ci && ri < s->cols) { screen_scr_set(s, ri, sb_row, 0x25B2); screen_scr_fg(s, ri, sb_row, 100,180,255); }
            }
            if (current_root && scroll_y < current_root->height - s->rows) {
                int ri = s->cols - 2;
                if (ri > ci && ri < s->cols) { screen_scr_set(s, ri, sb_row, 0x25BC); screen_scr_fg(s, ri, sb_row, 100,180,255); }
            }
        }

        /* ── Draw select popup overlay ── */
        if (select_popup_active && select_popup_focus_idx >= 0 && select_popup_focus_idx < focus_count) {
            LayoutNode* sel_node = focus_list[select_popup_focus_idx];
            int sx, sy, sw, sh;
            node_abs_box(sel_node, scroll_x, scroll_y, &sx, &sy, &sw, &sh);
            /* Popup appears below the select element, aligned to left edge */
            int pop_x = sx;
            int pop_y = sy + sh; /* just below the select */
            int max_vis = select_popup_count;
            if (max_vis > s->rows - pop_y - 1) max_vis = s->rows - pop_y - 1;
            /* Compute max label width */
            int max_w = 0;
            for (int oi = 0; oi < select_popup_count; oi++) {
                int w = (int)strlen(select_popup_vals[oi]);
                if (w > max_w) max_w = w;
            }
            int pop_w = max_w + 3; /* 1 for indicator + 1 space padding each side */
            if (pop_w > s->cols - pop_x) pop_w = s->cols - pop_x;
            if (pop_w < 4) pop_w = 4;
            /* Clamp pop_y to screen */
            if (pop_y < 0) pop_y = 0;
            if (pop_y + max_vis > s->rows) max_vis = s->rows - pop_y;
            if (max_vis > select_popup_count) max_vis = select_popup_count;
            if (max_vis < 1 && select_popup_count > 0) max_vis = 1;
            /* Draw border */
            int border_color = 100;
            for (int ci = pop_x; ci < pop_x + pop_w && ci < s->cols; ci++) {
                for (int ri = pop_y; ri < pop_y + max_vis + 2 && ri < s->rows; ri++) {
                    screen_scr_set(s, ci, ri, ' ');
                    screen_scr_bg(s, ci, ri, 20, 20, 40);
                }
            }
            /* Top border */
            if (pop_y < s->rows) {
                for (int ci = pop_x; ci < pop_x + pop_w && ci < s->cols; ci++) {
                    screen_scr_set(s, ci, pop_y, 0x2500);
                    screen_scr_fg(s, ci, pop_y, border_color, border_color, border_color);
                    screen_scr_bg(s, ci, pop_y, 20, 20, 40);
                }
                if (pop_x < s->cols) { screen_scr_set(s, pop_x, pop_y, 0x250C); screen_scr_fg(s, pop_x, pop_y, border_color, border_color, border_color); }
                int rgt = pop_x + pop_w - 1;
                if (rgt < s->cols) { screen_scr_set(s, rgt, pop_y, 0x2510); screen_scr_fg(s, rgt, pop_y, border_color, border_color, border_color); }
            }
            /* Bottom border */
            int bottom_row = pop_y + max_vis + 1;
            if (bottom_row < s->rows) {
                for (int ci = pop_x; ci < pop_x + pop_w && ci < s->cols; ci++) {
                    screen_scr_set(s, ci, bottom_row, 0x2500);
                    screen_scr_fg(s, ci, bottom_row, border_color, border_color, border_color);
                    screen_scr_bg(s, ci, bottom_row, 20, 20, 40);
                }
                if (pop_x < s->cols) { screen_scr_set(s, pop_x, bottom_row, 0x2514); screen_scr_fg(s, pop_x, bottom_row, border_color, border_color, border_color); }
                int rgt = pop_x + pop_w - 1;
                if (rgt < s->cols) { screen_scr_set(s, rgt, bottom_row, 0x2518); screen_scr_fg(s, rgt, bottom_row, border_color, border_color, border_color); }
            }
            /* Left/right borders */
            for (int ri = pop_y + 1; ri < pop_y + max_vis + 1 && ri < s->rows; ri++) {
                if (pop_x < s->cols) { screen_scr_set(s, pop_x, ri, 0x2502); screen_scr_fg(s, pop_x, ri, border_color, border_color, border_color); }
                int rgt = pop_x + pop_w - 1;
                if (rgt < s->cols) { screen_scr_set(s, rgt, ri, 0x2502); screen_scr_fg(s, rgt, ri, border_color, border_color, border_color); }
            }
            /* Draw option items */
            for (int oi = 0; oi < max_vis; oi++) {
                int idx = select_popup_scroll + oi;
                if (idx >= select_popup_count) break;
                int row = pop_y + 1 + oi;
                if (row >= s->rows) break;
                bool is_sel = (idx == select_popup_sel);
                /* Background: highlighted vs normal */
                int bg_r = is_sel ? 80 : 20, bg_g = is_sel ? 80 : 20, bg_b = is_sel ? 140 : 40;
                /* Paint the option row */
                int col = pop_x + 1;
                const char* label = select_popup_vals[idx];
                /* Selection indicator inside the border (don't touch pop_x, that's the border) */
                screen_scr_bg(s, col, row, bg_r, bg_g, bg_b);
                if (is_sel) {
                    screen_scr_set(s, col, row, 0x25B6); /* ▶ */
                    screen_scr_fg(s, col, row, 200, 200, 255);
                } else {
                    screen_scr_set(s, col, row, ' ');
                }
                int ci = col + 1;
                const char* lp = label;
                while (*lp && ci < pop_x + pop_w - 1 && ci < s->cols) {
                    uint32_t cp = uc_dec(&lp);
                    if (cp == 0) break;
                    screen_scr_set(s, ci, row, cp);
                    screen_scr_fg(s, ci, row, 220, 220, 220);
                    screen_scr_bg(s, ci, row, bg_r, bg_g, bg_b);
                    int w = uc_wid((int)cp);
                    ci += w;
                    if (w == 2 && ci < s->cols) screen_scr_set(s, ci, row, 0);
                }
                /* Fill remaining */
                while (ci < pop_x + pop_w - 1 && ci < s->cols) {
                    screen_scr_set(s, ci, row, ' ');
                    screen_scr_bg(s, ci, row, bg_r, bg_g, bg_b);
                    ci++;
                }
                /* ci is now at right border column; keep its original background intact */
            }
            /* Scroll indicators if list is clipped */
            if (select_popup_scroll > 0 && pop_y + 1 < s->rows) {
                screen_scr_set(s, pop_x + pop_w - 2, pop_y + 1, 0x25B2);
                screen_scr_fg(s, pop_x + pop_w - 2, pop_y + 1, 100, 180, 255);
            }
            if (select_popup_scroll + max_vis < select_popup_count && pop_y + max_vis < s->rows) {
                screen_scr_set(s, pop_x + pop_w - 2, pop_y + max_vis, 0x25BC);
                screen_scr_fg(s, pop_x + pop_w - 2, pop_y + max_vis, 100, 180, 255);
            }
        }

        screen_flush(s);

        /* Poll input */
        if (tb_poll_event(&ev) != TB_OK) {
            if (tb_last_errno() == EINTR) continue;
            break;
        }

        bool restyle = false;

        if (ev.type == TB_EVENT_MOUSE) {
            /* If popup is active, skip hover/wheel; let left-click through to popup handler */
            if (select_popup_active) {
                if (ev.key == TB_KEY_MOUSE_RELEASE) {
                    textarea_scrollbar_drag = -1;
                    global_sb_drag_v = -1;
                    global_sb_drag_h = -1;
                }
                last_mouse_x = ev.x; last_mouse_y = ev.y;
                mouse_idle_count = 0;
                /* Allow plain left-click (no drag) to fall through → handled below */
                if (ev.key != TB_KEY_MOUSE_LEFT || (ev.mod & TB_MOD_MOTION)) {
                    goto skip_mouse_processing;
                }
            }
            if (ev.key != TB_KEY_MOUSE_WHEEL_UP && ev.key != TB_KEY_MOUSE_WHEEL_DOWN) {
                /* Find node under mouse for hover (deepest match) */
                GumboNode* hover_node = NULL;
                LayoutNode* hstack[256]; int hsp = 0;
                hstack[hsp++] = current_root;
                int best_depth = -1;
                while (hsp > 0) {
                    LayoutNode* n = hstack[--hsp];
                    for (size_t ci = 0; ci < n->num_children && hsp < 256; ci++)
                        hstack[hsp++] = n->children[ci];
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
                    prev_hover = g_interact_hover;
                    g_interact_hover = hover_node;
                    restyle = true;
                }

                /* Left click → focus + active (not pure motion) */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION)) {
                    /* ── Select popup mouse click ── */
                    if (select_popup_active) {
                        /* Compute popup dimensions (same as rendering section) */
                        LayoutNode* _psn = focus_list[select_popup_focus_idx];
                        int _psx, _psy, _psw, _psh;
                        node_abs_box(_psn, scroll_x, scroll_y, &_psx, &_psy, &_psw, &_psh);
                        int _pp_x = _psx;
                        int _pp_y = _psy + _psh;
                        int _max_w = 0;
                        for (int _oi = 0; _oi < select_popup_count; _oi++) {
                            int _w = (int)strlen(select_popup_vals[_oi]);
                            if (_w > _max_w) _max_w = _w;
                        }
                        int _pp_w = _max_w + 4; /* 2 padding + 2 borders */
                        if (_pp_w > s->cols - _pp_x) _pp_w = s->cols - _pp_x;
                        if (_pp_w < 4) _pp_w = 4;
                        int _pp_h = select_popup_count < (s->rows - _pp_y - 1) ? select_popup_count : (s->rows - _pp_y - 1);
                        if (_pp_h < 1 && select_popup_count > 0) _pp_h = 1;
                        /* Check if click is inside popup area */
                        if (ev.x >= _pp_x && ev.x < _pp_x + _pp_w &&
                            ev.y > _pp_y && ev.y <= _pp_y + _pp_h) {
                            int oi = (ev.y - _pp_y - 1) + select_popup_scroll;
                            if (oi >= 0 && oi < select_popup_count) {
                                select_popup_sel = oi;
                                /* Confirm selection (unless select has its own value=header) */
                                LayoutNode* _sel = focus_list[select_popup_focus_idx];
                                const char* _val = select_popup_vals[select_popup_sel];
                                GumboAttribute* _sel_va = gumbo_get_attribute(
                                    &_sel->styled->node->v.element.attributes, "value");
                                if (!_sel_va || !_sel_va->value || !_sel_va->value[0]) {
                                    node_set_text(_sel, _val);
                                    GumboAttribute* _id_a = gumbo_get_attribute(
                                        &_sel->styled->node->v.element.attributes, "id");
                                    if (_id_a && _id_a->value)
                                        node_override_text(cb, _id_a->value, _val);
                                }
                                snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ Selected: %s", _val);
                                select_popup_active = 0;
                            }
                        } else {
                            /* Click outside popup → cancel */
                            select_popup_active = 0;
                            snprintf(cb->status_msg, sizeof(cb->status_msg), "✗ Cancelled");
                        }
                        focus_idx = select_popup_focus_idx;
                        g_interact_focus = focus_list[focus_idx]->styled->node;
                        restyle = true;  /* Trigger style update for focus/active */
                        prev_active = g_interact_active;
                        g_interact_active = g_interact_focus;
                        goto end_mouse_left;
                    }
                    focus_idx = -1;
                    /* First check if clicked on a <label> — redirect to its for= target */
                    GumboNode* clicked_elem = NULL;
                    {
                        LayoutNode* hstack2[256]; int hsp2 = 0;
                        hstack2[hsp2++] = current_root;
                        int best_d2 = -1;
                        while (hsp2 > 0) {
                            LayoutNode* n = hstack2[--hsp2];
                            for (size_t ci = 0; ci < n->num_children && hsp2 < 256; ci++)
                                hstack2[hsp2++] = n->children[ci];
                            int hx, hy, hw, hh;
                            node_abs_box(n, scroll_x, scroll_y, &hx, &hy, &hw, &hh);
                            int depth = 0; { LayoutNode* pp = n; while (pp) { depth++; pp = pp->parent; } }
                            if (ev.x >= hx && ev.x < hx + hw && ev.y >= hy && ev.y < hy + hh && depth > best_d2) {
                                if (n->styled && n->styled->node &&
                                    n->styled->node->type == GUMBO_NODE_ELEMENT) {
                                    clicked_elem = n->styled->node;
                                    best_d2 = depth;
                                }
                            }
                        }
                    }
                    if (clicked_elem && clicked_elem->type == GUMBO_NODE_ELEMENT &&
                        clicked_elem->v.element.tag == GUMBO_TAG_LABEL) {
                        GumboAttribute* for_attr = gumbo_get_attribute(
                            &clicked_elem->v.element.attributes, "for");
                        if (for_attr && for_attr->value) {
                            for (int fi = 0; fi < focus_count; fi++) {
                                LayoutNode* n = focus_list[fi];
                                if (n->styled && n->styled->node &&
                                    n->styled->node->type == GUMBO_NODE_ELEMENT) {
                                    GumboAttribute* id_attr = gumbo_get_attribute(
                                        &n->styled->node->v.element.attributes, "id");
                                    if (id_attr && id_attr->value &&
                                        strcmp(id_attr->value, for_attr->value) == 0) {
                                        focus_idx = fi;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (focus_idx < 0) {
                        for (int fi = 0; fi < focus_count; fi++) {
                            LayoutNode* n = focus_list[fi];
                            int nx, ny, nw, nh;
                            node_abs_box(n, scroll_x, scroll_y, &nx, &ny, &nw, &nh);
                            if (ev.x >= nx && ev.x < nx + nw && ev.y >= ny && ev.y < ny + nh) {
                                focus_idx = fi;
                                /* If clicked on SELECT (or its child OPTION), open popup */
                                if (n->styled && n->styled->node &&
                                    n->styled->node->type == GUMBO_NODE_ELEMENT &&
                                    n->styled->node->v.element.tag == GUMBO_TAG_SELECT) {
                                    goto click_handle_select_popup;
                                }
                                /* If clicked on INPUT or TEXTAREA, set cursor to click position */
                                if (n->styled && n->styled->node &&
                                    n->styled->node->type == GUMBO_NODE_ELEMENT &&
                                    (n->styled->node->v.element.tag == GUMBO_TAG_INPUT ||
                                     n->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA)) {
                                    int ca_left = nx + n->border_left + n->padding_left;
                                    int ca_top = ny + n->border_top + n->padding_top;
                                    char* buf = input_buf[fi];
                                    int blen = (int)strlen(buf);
                                    int click_row = ev.y - ca_top;
                                    int click_col = ev.x - ca_left;
                                    if (n->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
                                        click_row += textarea_scroll_y[fi];
                                        click_col += textarea_scroll_x[fi];
                                        /* Find the target line in buffer */
                                        int bo = 0, line_idx = 0;
                                        int line_start = 0;
                                        while (bo < blen && line_idx < click_row) {
                                            if (buf[bo] == '\n') { line_idx++; line_start = bo + 1; }
                                            bo++;
                                        }
                                        /* Find byte offset closest to click_col */
                                        int new_pos = line_start;
                                        int cur_col = 0;
                                        while (new_pos < blen && buf[new_pos] != '\n') {
                                            int cw = uc_str_width_len(buf + new_pos,
                                                uc_utf8_length[(unsigned char)buf[new_pos]]);
                                            if (cur_col + cw > click_col) break;
                                            cur_col += cw;
                                            new_pos += uc_utf8_length[(unsigned char)buf[new_pos]];
                                        }
                                        input_cursor[fi] = new_pos;
                                    } else {
                                        /* INPUT: single-line */
                                        if (click_col < 0) click_col = 0;
                                        int new_pos = 0;
                                        int cur_col = 0;
                                        while (new_pos < blen) {
                                            int cw = uc_str_width_len(buf + new_pos,
                                                uc_utf8_length[(unsigned char)buf[new_pos]]);
                                            if (cur_col + cw > click_col) break;
                                            cur_col += cw;
                                            new_pos += uc_utf8_length[(unsigned char)buf[new_pos]];
                                        }
                                        input_cursor[fi] = new_pos;
                                    }
                                }
                                break;
                            }
                        }
                    }
click_handle_select_popup:
                    /* If the clicked (or focused) element is a <select>, open popup immediately */
                    if (focus_idx >= 0 && focus_idx < focus_count &&
                        !select_popup_active &&
                        focus_list[focus_idx]->styled &&
                        focus_list[focus_idx]->styled->node &&
                        focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                        focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_SELECT) {
                        select_popup_focus_idx = focus_idx;
                        select_popup_count = 0;
                        select_popup_sel = 0;
                        select_popup_scroll = 0;
                        const char* cur_val = focus_list[focus_idx]->text_content;
                        GumboVector* ch = &focus_list[focus_idx]->styled->node->v.element.children;
                        for (unsigned int oi = 0; oi < ch->length && select_popup_count < 32; oi++) {
                            GumboNode* oc = (GumboNode*)ch->data[oi];
                            if (oc->type == GUMBO_NODE_ELEMENT && oc->v.element.tag == GUMBO_TAG_OPTION) {
                                GumboAttribute* oa = gumbo_get_attribute(&oc->v.element.attributes, "value");
                                if (oa && oa->value) {
                                    snprintf(select_popup_vals[select_popup_count], 64, "%s", oa->value);
                                    if (cur_val && strcmp(oa->value, cur_val) == 0)
                                        select_popup_sel = select_popup_count;
                                    select_popup_count++;
                                }
                            }
                        }
                        if (select_popup_count > 0) {
                            select_popup_active = 1;
                            snprintf(cb->status_msg, sizeof(cb->status_msg),
                                     "▲ Select an option (↑↓ enter, Esc cancel)");
                        }
                    }
                    if (g_interact_focus != (focus_idx >= 0 ? focus_list[focus_idx]->styled->node : NULL)) {
                        g_interact_focus = (focus_idx >= 0 && focus_list[focus_idx]->styled) ? focus_list[focus_idx]->styled->node : NULL;
                        restyle = true;
                    }
                    prev_active = g_interact_active;
                    g_interact_active = g_interact_focus;
                    restyle = true;
                }

end_mouse_left:
                /* Mouse release → clear active and drag state */
                if (ev.key == TB_KEY_MOUSE_RELEASE) {
                    textarea_scrollbar_drag = -1;
                    global_sb_drag_v = -1;
                    global_sb_drag_h = -1;
                    if (g_interact_active) {
                        prev_active = g_interact_active;
                        g_interact_active = NULL;
                        restyle = true;
                    }
                }

                /* Scrollbar click: left-click on the scrollbar column of a textarea */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION)) {
                    for (int fi = 0; fi < focus_count; fi++) {
                        LayoutNode* n = focus_list[fi];
                        if (!n->styled || !n->styled->node ||
                            n->styled->node->type != GUMBO_NODE_ELEMENT ||
                            n->styled->node->v.element.tag != GUMBO_TAG_TEXTAREA)
                            continue;
                        int tx, ty, tw, th;
                        node_abs_box(n, scroll_x, scroll_y, &tx, &ty, &tw, &th);
                        int sb_col = tx + n->border_left + n->padding_left + n->width;
                        if (ev.x == sb_col && ev.y >= ty && ev.y < ty + th) {
                            int total = 1;
                            const char* sp = input_buf[fi];
                            while (sp && *sp) { if (*sp == '\n') total++; sp++; }
                            int ta_h = n->height > 0 ? n->height : 5;
                            int _scroll_range = total - ta_h;
                            if (_scroll_range > 0) {
                                int target = ((ev.y - ty) * _scroll_range) / ta_h;
                                if (target < 0) target = 0;
                                if (target > _scroll_range) target = _scroll_range;
                                textarea_scroll_y[fi] = target;
                                focus_idx = fi;
                                textarea_scrollbar_drag = fi;
                                restyle = true;
                            }
                            break;
                        }
                    }
                }

                /* Scrollbar drag: left-motion while dragging the scrollbar */
                if (ev.key == TB_KEY_MOUSE_LEFT && (ev.mod & TB_MOD_MOTION) && textarea_scrollbar_drag >= 0) {
                    int fi = textarea_scrollbar_drag;
                    if (fi >= 0 && fi < focus_count) {
                        LayoutNode* n = focus_list[fi];
                        int tx, ty, tw, th;
                        node_abs_box(n, scroll_x, scroll_y, &tx, &ty, &tw, &th);
                        if (ev.y >= ty && ev.y < ty + th) {
                            int total = 1;
                            const char* sp = input_buf[fi];
                            while (sp && *sp) { if (*sp == '\n') total++; sp++; }
                            int ta_h = n->height > 0 ? n->height : 5;
                            int _scroll_range = total - ta_h;
                            if (_scroll_range > 0) {
                                int target = ((ev.y - ty) * _scroll_range) / ta_h;
                                if (target < 0) target = 0;
                                if (target > _scroll_range) target = _scroll_range;
                                textarea_scroll_y[fi] = target;
                                restyle = true;
                            }
                        }
                    }
                }

                /* ── Global scrollbar click: vertical (rightmost column) ── */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION) &&
                    cb && cb->show_scrollbars && current_root &&
                    ev.x == s->cols - 1 && ev.y < s->rows - 1) {
                    int track_h = s->rows - 1;
                    int content_h = current_root->height;
                    if (content_h > track_h) {
                        int scroll_range = content_h - track_h;
                        scroll_y = (ev.y * scroll_range) / track_h;
                        if (scroll_y < 0) scroll_y = 0;
                        if (scroll_y > scroll_range) scroll_y = scroll_range;
                        global_sb_drag_v = 1;
                        restyle = true;
                    }
                }
                /* ── Global scrollbar click: horizontal (row above status bar) ── */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION) &&
                    cb && cb->show_scrollbars && current_root &&
                    ev.y == s->rows - 2 && ev.x < s->cols - 1) {
                    int track_w = s->cols - 1;
                    int content_w = current_root->width;
                    if (content_w > track_w) {
                        int scroll_range = content_w - track_w;
                        scroll_x = (ev.x * scroll_range) / track_w;
                        if (scroll_x < 0) scroll_x = 0;
                        if (scroll_x > scroll_range) scroll_x = scroll_range;
                        global_sb_drag_h = 1;
                        restyle = true;
                    }
                }

                /* ── Global scrollbar drag: vertical ── */
                if ((ev.mod & TB_MOD_MOTION) && global_sb_drag_v >= 0 &&
                    cb && cb->show_scrollbars && current_root) {
                    int track_h = s->rows - 1;
                    int content_h = current_root->height;
                    if (content_h > track_h) {
                        int scroll_range = content_h - track_h;
                        scroll_y = (ev.y * scroll_range) / track_h;
                        if (scroll_y < 0) scroll_y = 0;
                        if (scroll_y > scroll_range) scroll_y = scroll_range;
                        restyle = true;
                    }
                }
                /* ── Global scrollbar drag: horizontal ── */
                if ((ev.mod & TB_MOD_MOTION) && global_sb_drag_h >= 0 &&
                    cb && cb->show_scrollbars && current_root) {
                    int track_w = s->cols - 1;
                    int content_w = current_root->width;
                    if (content_w > track_w) {
                        int scroll_range = content_w - track_w;
                        scroll_x = (ev.x * scroll_range) / track_w;
                        if (scroll_x < 0) scroll_x = 0;
                        if (scroll_x > scroll_range) scroll_x = scroll_range;
                        restyle = true;
                    }
                }

                /* Left click on BUTTON: trigger click action via callback (not pure motion) */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION) && cb && cb->on_button_click) {
                    LayoutNode* clicked_btn = NULL;
                    int clicked_focus_idx = -1;
                    for (int fi = 0; fi < focus_count; fi++) {
                        LayoutNode* n = focus_list[fi];
                        int nx, ny, nw, nh;
                        node_abs_box(n, scroll_x, scroll_y, &nx, &ny, &nw, &nh);
                        if (ev.x >= nx && ev.x < nx + nw && ev.y >= ny && ev.y < ny + nh) {
                            if (n->styled && n->styled->node &&
                                n->styled->node->type == GUMBO_NODE_ELEMENT &&
                                n->styled->node->v.element.tag == GUMBO_TAG_BUTTON) {
                                clicked_btn = n;
                                clicked_focus_idx = fi;
                            }
                            break;
                        }
                    }
                    if (clicked_btn) {
                        const char* btxt = clicked_btn->text_content;
                        if (!btxt) btxt = "(unnamed)";
                        while (*btxt == ' ' || *btxt == '\n' || *btxt == '\r') btxt++;
                        if (cb->on_button_click(btxt, clicked_focus_idx,
                                                 focus_list, focus_count,
                                                 input_buf, input_buf_count, cb)) {
                            restyle = true;
                        }
                    }
                }
            }
        }

        if (ev.type == TB_EVENT_KEY) {
            /* ── Select popup key handling ── */
            if (select_popup_active) {
                if (ev.key == TB_KEY_ESC) {
                    select_popup_active = 0;
                    snprintf(cb->status_msg, sizeof(cb->status_msg), "✗ Cancelled");
                    continue;
                }
                if (ev.key == TB_KEY_ENTER || ev.key == TB_KEY_SPACE) {
                    /* Confirm selection */
                    if (select_popup_focus_idx >= 0 && select_popup_focus_idx < focus_count &&
                        select_popup_sel >= 0 && select_popup_sel < select_popup_count) {
                        LayoutNode* sel_node = focus_list[select_popup_focus_idx];
                        /* If select has its own value=header, keep display text unchanged */
                        GumboAttribute* sel_va = gumbo_get_attribute(
                            &sel_node->styled->node->v.element.attributes, "value");
                        bool is_header = (sel_va && sel_va->value && sel_va->value[0]);
                        const char* val = select_popup_vals[select_popup_sel];
                        if (!is_header) {
                            node_set_text(sel_node, val);
                            GumboAttribute* id_a = gumbo_get_attribute(
                                &sel_node->styled->node->v.element.attributes, "id");
                            if (id_a && id_a->value)
                                node_override_text(cb, id_a->value, val);
                        }
                        snprintf(cb->status_msg, sizeof(cb->status_msg),
                                 "✓ Selected: %s", val);
                    }
                    select_popup_active = 0;
                    continue;
                }
                if (ev.key == TB_KEY_ARROW_UP) {
                    if (select_popup_sel > 0) {
                        select_popup_sel--;
                        /* Auto-scroll: if selection goes above visible range */
                        if (select_popup_sel < select_popup_scroll)
                            select_popup_scroll = select_popup_sel;
                    }
                    continue;
                }
                if (ev.key == TB_KEY_ARROW_DOWN) {
                    if (select_popup_sel < select_popup_count - 1) {
                        select_popup_sel++;
                        /* Auto-scroll: if selection goes below visible range */
                        int popup_h = select_popup_count < (s->rows - 2) ? select_popup_count : (s->rows - 2);
                        if (select_popup_sel >= select_popup_scroll + popup_h)
                            select_popup_scroll = select_popup_sel - popup_h + 1;
                    }
                    continue;
                }
                continue; /* block other keys while popup is open */
            }

            /* ── User key handler (overrides defaults) ── */
            if (cb && cb->on_key && cb->on_key(&ev, cb)) {
                continue;
            }

            /* ── Input editing: when an INPUT or TEXTAREA is focused ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                (focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_INPUT ||
                 focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA)) {
                bool is_textarea = (focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA);
                char* buf = input_buf[focus_idx];
                size_t blen = strlen(buf);
                int* cpos = &input_cursor[focus_idx];
                LayoutNode* inp = focus_list[focus_idx];
                if (*cpos < 0) *cpos = 0;
                if ((size_t)*cpos > blen) *cpos = (int)blen;

                if (ev.key == TB_KEY_ARROW_UP && is_textarea) {
                    /* Move cursor up one line */
                    if (*cpos > 0) {
                        /* Find start of current line */
                        int line_start = *cpos;
                        while (line_start > 0 && buf[line_start - 1] != '\n')
                            line_start--;
                        /* Find target visual column = width of text from line_start to cursor */
                        int target_col = uc_str_width_len(buf + line_start, *cpos - line_start);
                        textarea_target_col[focus_idx] = target_col;
                        /* Find start of previous line */
                        if (line_start > 0) {
                            int prev_end = line_start - 1; /* the \n we just crossed */
                            int prev_start = prev_end;
                            while (prev_start > 0 && buf[prev_start - 1] != '\n')
                                prev_start--;
                            /* Find byte offset in prev line closest to target_col */
                            int new_pos = prev_start;
                            int cur_col = 0;
                            while (new_pos < prev_end) {
                                int next_w = uc_str_width_len(buf + new_pos,
                                    uc_utf8_length[(unsigned char)buf[new_pos]]);
                                if (cur_col + next_w > target_col) break;
                                cur_col += next_w;
                                new_pos += uc_utf8_length[(unsigned char)buf[new_pos]];
                            }
                            *cpos = new_pos;
                        }
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_ARROW_DOWN && is_textarea) {
                    /* Move cursor down one line */
                    /* Find end of current line */
                    int line_end = *cpos;
                    while (buf[line_end] && buf[line_end] != '\n')
                        line_end++;
                    if (buf[line_end] == '\n') {
                        /* Find start of current line for target column */
                        int line_start = *cpos;
                        while (line_start > 0 && buf[line_start - 1] != '\n')
                            line_start--;
                        int target_col = uc_str_width_len(buf + line_start, *cpos - line_start);
                        textarea_target_col[focus_idx] = target_col;
                        /* Move past \n to next line */
                        int next_start = line_end + 1;
                        if (next_start < (int)blen) {
                            /* Find end of next line */
                            int next_end = next_start;
                            while (buf[next_end] && buf[next_end] != '\n')
                                next_end++;
                            /* Find byte offset in next line closest to target_col */
                            int new_pos = next_start;
                            int cur_col = 0;
                            while (new_pos < next_end) {
                                int next_w = uc_str_width_len(buf + new_pos,
                                    uc_utf8_length[(unsigned char)buf[new_pos]]);
                                if (cur_col + next_w > target_col) break;
                                cur_col += next_w;
                                new_pos += uc_utf8_length[(unsigned char)buf[new_pos]];
                            }
                            *cpos = new_pos;
                        } else {
                            /* Move past final \n to end of buffer */
                            *cpos = (int)blen;
                        }
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_ARROW_LEFT) {
                    if (*cpos > 0) {
                        (*cpos)--;
                        while (*cpos > 0 && (buf[*cpos] & 0xC0) == 0x80)
                            (*cpos)--;
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_ARROW_RIGHT) {
                    if (*cpos < (int)blen) {
                        (*cpos)++;
                        while (*cpos < (int)blen && (buf[*cpos] & 0xC0) == 0x80)
                            (*cpos)++;
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_HOME) {
                    *cpos = 0;
                    goto input_update_text;
                } else if (ev.key == TB_KEY_END) {
                    *cpos = (int)blen;
                    goto input_update_text;
                } else if (ev.key == TB_KEY_BACKSPACE2 || ev.key == TB_KEY_BACKSPACE) {
                    if (*cpos > 0) {
                        /* Walk back to the start of the UTF-8 character */
                        int start = *cpos - 1;
                        while (start > 0 && (buf[start] & 0xC0) == 0x80)
                            start--;
                        int char_bytes = *cpos - start;
                        memmove(buf + start, buf + *cpos, blen - *cpos + 1);
                        *cpos = start;
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_DELETE) {
                    if (*cpos < (int)blen) {
                        /* Find the full UTF-8 character at cursor */
                        int end = *cpos + 1;
                        while (end < (int)blen && (buf[end] & 0xC0) == 0x80)
                            end++;
                        int char_bytes = end - *cpos;
                        memmove(buf + *cpos, buf + end, blen - end + 1);
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_ENTER && is_textarea) {
                    /* Textarea: Enter inserts newline */
                    if (blen < (int)sizeof(input_buf[0]) - 2) {
                        memmove(buf + *cpos + 1, buf + *cpos, blen - *cpos + 1);
                        buf[*cpos] = '\n';
                        (*cpos)++;
                    }
                    goto input_update_text;
                } else if (ev.key == TB_KEY_ENTER && !is_textarea) {
                    /* Input: Enter = go to next focus */
                    if (focus_count > 0) {
                        focus_idx = (focus_idx + 1) % focus_count;
                        if (focus_idx >= 0 && focus_list[focus_idx]->styled)
                            g_interact_focus = focus_list[focus_idx]->styled->node;
                        else g_interact_focus = NULL;
                        restyle = true;
                    }
                    continue;
                } else if (ev.ch >= 0x20) {
                    /* Accept any printable Unicode codepoint (ASCII + CJK + other scripts).
                       termbox2 decodes UTF-8 input into ev.ch as a proper Unicode codepoint,
                       and uc_enc() encodes it back to UTF-8 for the input buffer. */
                    if (blen < (int)sizeof(input_buf[0]) - 2) {
                        char add[8];
                        int nch = uc_enc(ev.ch, add); add[nch] = '\0';
                        size_t addlen = strlen(add);
                        /* Ensure buffer doesn't overflow */
                        if (blen + addlen < sizeof(input_buf[0]) - 1) {
                            memmove(buf + *cpos + addlen, buf + *cpos, blen - *cpos + 1);
                            memcpy(buf + *cpos, add, addlen);
                            *cpos += (int)addlen;
                        }
                    }
                } else if (ev.key == TB_KEY_SPACE) {
                    if (blen < (int)sizeof(input_buf[0]) - 2) {
                        memmove(buf + *cpos + 1, buf + *cpos, blen - *cpos + 1);
                        buf[*cpos] = ' ';
                        (*cpos)++;
                    }
                } else {
                    goto handle_scroll_keys;
                }

input_update_text:
                /* Auto-scroll textarea to keep cursor in visible area */
                if (is_textarea) {
                    const char* _ta = input_buf[focus_idx];
                    int _ta_len = (int)strlen(_ta);
                    /* Count total lines and max line width */
                    int _total_lines = 1, _max_line_w = 0, _cur_lw = 0;
                    for (int _i = 0; _i < _ta_len; _i++) {
                        if (_ta[_i] == '\n') { _total_lines++;
                            if (_cur_lw > _max_line_w) _max_line_w = _cur_lw;
                            _cur_lw = 0; continue; }
                        if ((_ta[_i] & 0xC0) == 0x80) continue;
                        _cur_lw += 1;
                    }
                    if (_cur_lw > _max_line_w) _max_line_w = _cur_lw;
                    /* Find cursor line index */
                    int _ci = 0, _last_ls = 0;
                    for (int _i = 0; _i < *cpos && _i < _ta_len; _i++) {
                        if (_ta[_i] == '\n') { _ci++; _last_ls = _i + 1; }
                    }
                    int _col = uc_str_width_len(_ta + _last_ls, *cpos - _last_ls);
                    int _vh = inp->height > 0 ? inp->height : 5;
                    int _vw = inp->width > 0 ? inp->width : 40;
                    int _max_v = _total_lines - _vh; if (_max_v < 0) _max_v = 0;
                    int _max_h = _max_line_w - _vw; if (_max_h < 0) _max_h = 0;
                    /* Only adjust scroll if cursor is OUTSIDE visible area */
                    if (_ci < textarea_scroll_y[focus_idx])
                        textarea_scroll_y[focus_idx] = _ci;
                    if (_ci >= textarea_scroll_y[focus_idx] + _vh)
                        textarea_scroll_y[focus_idx] = _ci - _vh + 1;
                    if (_col < textarea_scroll_x[focus_idx])
                        textarea_scroll_x[focus_idx] = _col;
                    if (_col >= textarea_scroll_x[focus_idx] + _vw)
                        textarea_scroll_x[focus_idx] = _col - _vw + 1;
                    if (textarea_scroll_y[focus_idx] < 0) textarea_scroll_y[focus_idx] = 0;
                    if (textarea_scroll_x[focus_idx] < 0) textarea_scroll_x[focus_idx] = 0;
                    if (textarea_scroll_y[focus_idx] > _max_v) textarea_scroll_y[focus_idx] = _max_v;
                    if (textarea_scroll_x[focus_idx] > _max_h) textarea_scroll_x[focus_idx] = _max_h;
                }
                /* Update text_content */
                { char txt[4096];
                snprintf(txt, sizeof(txt), "%s", buf);
                if (inp->text_content) free(inp->text_content);
                inp->text_content = strdup(txt); }
                continue;
            }

            /* ── Button click: Enter/Space on focused BUTTON / SUMMARY / SELECT ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                (ev.key == TB_KEY_ENTER || ev.key == TB_KEY_SPACE) &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_BUTTON &&
                cb && cb->on_button_click) {
                const char* btxt = focus_list[focus_idx]->text_content;
                if (!btxt) btxt = "(unnamed)";
                while (*btxt == ' ' || *btxt == '\n' || *btxt == '\r') btxt++;
                if (cb->on_button_click(btxt, focus_idx,
                                         focus_list, focus_count,
                                         input_buf, input_buf_count, cb)) {
                    restyle = true;
                }
                continue;
            }

            /* ── <summary> click: toggle parent <details> ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                (ev.key == TB_KEY_ENTER || ev.key == TB_KEY_SPACE) &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_SUMMARY) {
                /* Find parent <details> by walking up the layout tree */
                for (LayoutNode* det = focus_list[focus_idx]->parent; det; det = det->parent) {
                    /* Check if this is a details node */
                    if (det->styled && det->styled->node &&
                        det->styled->node->type == GUMBO_NODE_ELEMENT &&
                        det->styled->node->v.element.tag == GUMBO_TAG_DETAILS) {
                        bool new_state = !det->details_open;
                        /* Save to global state (survives layout rebuild) */
                        GumboAttribute* id_attr = gumbo_get_attribute(
                            &det->styled->node->v.element.attributes, "id");
                        if (id_attr && id_attr->value)
                            details_set_state(id_attr->value, new_state);
                        det->details_open = new_state;
                        snprintf(cb->status_msg, sizeof(cb->status_msg),
                                 new_state ? "▾ Expanded" : "▸ Collapsed");
                        restyle = true;
                        break;
                    }
                }
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
                /* Clear hover state on keyboard navigation */
                if (g_interact_hover) { g_interact_hover = NULL; restyle = true; }
                focus_idx = (focus_idx + 1) % focus_count;
                GumboNode* gn = (focus_idx >= 0 && focus_list[focus_idx]->styled)
                                ? focus_list[focus_idx]->styled->node : NULL;
                if (gn) {
                    g_interact_focus = gn;
                    if (cb && cb->on_focus_change) {
                        cb->on_focus_change(focus_idx,
                            gumbo_normalized_tagname(gn->v.element.tag), cb);
                    } else if (cb) {
                        snprintf(cb->status_msg, sizeof(cb->status_msg),
                                 "Tab: focus #%d (%s)", focus_idx,
                                 gumbo_normalized_tagname(gn->v.element.tag));
                    }
                } else {
                    g_interact_focus = NULL;
                }
                restyle = true;
                continue;
            }
            else if (ev.key == TB_KEY_BACK_TAB && focus_count > 0) {
                /* Clear hover state on keyboard navigation */
                if (g_interact_hover) { g_interact_hover = NULL; restyle = true; }
                focus_idx = (focus_idx - 1 + focus_count) % focus_count;
                GumboNode* gn = (focus_idx >= 0 && focus_list[focus_idx]->styled)
                                ? focus_list[focus_idx]->styled->node : NULL;
                if (gn) {
                    g_interact_focus = gn;
                    if (cb && cb->on_focus_change) {
                        cb->on_focus_change(focus_idx,
                            gumbo_normalized_tagname(gn->v.element.tag), cb);
                    } else if (cb) {
                        snprintf(cb->status_msg, sizeof(cb->status_msg),
                                 "Shift+Tab: focus #%d (%s)", focus_idx,
                                 gumbo_normalized_tagname(gn->v.element.tag));
                    }
                } else {
                    g_interact_focus = NULL;
                }
                restyle = true;
                continue;
            }

            if (g_interact_focus && g_interact_active != g_interact_focus) {
                g_interact_active = g_interact_focus;
                restyle = true;
            }

            /* ── <select> popup: Enter on focused SELECT opens popup ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                (ev.key == TB_KEY_ENTER || ev.key == TB_KEY_SPACE) &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_SELECT &&
                !select_popup_active) {
                /* Collect option values from the <select>'s <option> children */
                select_popup_focus_idx = focus_idx;
                select_popup_count = 0;
                select_popup_sel = 0;
                select_popup_scroll = 0;
                const char* cur_val = focus_list[focus_idx]->text_content;
                GumboVector* ch = &focus_list[focus_idx]->styled->node->v.element.children;
                for (unsigned int oi = 0; oi < ch->length && select_popup_count < 32; oi++) {
                    GumboNode* oc = (GumboNode*)ch->data[oi];
                    if (oc->type == GUMBO_NODE_ELEMENT && oc->v.element.tag == GUMBO_TAG_OPTION) {
                        GumboAttribute* oa = gumbo_get_attribute(&oc->v.element.attributes, "value");
                        if (oa && oa->value) {
                            snprintf(select_popup_vals[select_popup_count], 64, "%s", oa->value);
                            if (cur_val && strcmp(oa->value, cur_val) == 0)
                                select_popup_sel = select_popup_count;
                            select_popup_count++;
                        }
                    }
                }
                if (select_popup_count > 0) {
                    select_popup_active = 1;
                    snprintf(cb->status_msg, sizeof(cb->status_msg),
                             "▲ Select an option (↑↓ enter, Esc cancel)");
                }
                continue;
            }
        }

        /* Mouse wheel */
        if (ev.type == TB_EVENT_MOUSE) {
            if (ev.key == TB_KEY_MOUSE_WHEEL_UP || ev.key == TB_KEY_MOUSE_WHEEL_DOWN) {
                /* Check if mouse is over a focused textarea — scroll its content instead of the page */
                int ta_idx = -1;
                if (focus_idx >= 0 && focus_idx < focus_count) {
                    LayoutNode* _tf = focus_list[focus_idx];
                    if (_tf->styled && _tf->styled->node &&
                        _tf->styled->node->type == GUMBO_NODE_ELEMENT &&
                        _tf->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
                        int _tx, _ty, _tw, _th;
                        node_abs_box(_tf, scroll_x, scroll_y, &_tx, &_ty, &_tw, &_th);
                        if (ev.x >= _tx && ev.x < _tx + _tw && ev.y >= _ty && ev.y < _ty + _th)
                            ta_idx = focus_idx;
                    }
                }
                if (ta_idx >= 0) {
                    int total_lines = 1;
                    const char* _lp = input_buf[ta_idx];
                    while (*_lp) { if (*_lp == '\n') total_lines++; _lp++; }
                    int ta_h = focus_list[ta_idx]->height > 0 ? focus_list[ta_idx]->height : 5;
                    int max_scroll = total_lines - ta_h;
                    if (max_scroll < 0) max_scroll = 0;
                    if (ev.key == TB_KEY_MOUSE_WHEEL_UP) {
                        if (textarea_scroll_y[ta_idx] > 0) textarea_scroll_y[ta_idx] -= 1;
                    } else {
                        textarea_scroll_y[ta_idx] += 1;
                        if (textarea_scroll_y[ta_idx] > max_scroll) textarea_scroll_y[ta_idx] = max_scroll;
                    }
                    /* Clear hover on wheel for textarea — element under cursor changed */
                    if (g_interact_hover) { prev_hover = g_interact_hover; g_interact_hover = NULL; restyle = true; }
                } else {
                    if (ev.key == TB_KEY_MOUSE_WHEEL_UP) { scroll_y -= 3; }
                    else if (ev.key == TB_KEY_MOUSE_WHEEL_DOWN) { scroll_y += 3; }
                }
            }
            /* Track mouse position for :hover leave detection */
            last_mouse_x = ev.x; last_mouse_y = ev.y;
            mouse_idle_count = 0;
        }

skip_mouse_processing:
        /* :hover mouse leave: clear hover if keyboard input without mouse */
        if (ev.type == TB_EVENT_KEY && g_interact_hover) {
            mouse_idle_count++;
            if (mouse_idle_count > 3) {
                g_interact_hover = NULL;
                restyle = true;
                mouse_idle_count = 0;
            }
        }

        /* Resize via event — termbox2 handles SIGWINCH internally */
        if (ev.type == TB_EVENT_RESIZE) {
            vw = ev.w; vh = ev.h;
            screen_free(s); s = screen_create(vw, vh);
            vw_cache = vw; vh_cache = vh;
            restyle = true;
        }

        /* If hover/focus/active changed, rebuild styles and layout */
        if (restyle && saved_css && saved_st) {
            /* Recompute previous hover node (remove old :hover styles) */
            if (prev_hover && prev_hover != g_interact_hover && prev_hover != g_interact_focus) {
                StyledNode* sn = find_styled_node(saved_st, prev_hover);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            if (g_interact_hover) {
                StyledNode* sn = find_styled_node(saved_st, g_interact_hover);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            if (g_interact_focus && g_interact_focus != g_interact_hover) {
                StyledNode* sn = find_styled_node(saved_st, g_interact_focus);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            /* Recompute previous active node (remove old :active styles) */
            if (prev_active && prev_active != g_interact_hover && prev_active != g_interact_focus && prev_active != g_interact_active) {
                StyledNode* sn = find_styled_node(saved_st, prev_active);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            prev_hover = NULL;
            prev_active = NULL;
            LayoutNode* new_root = build_layout_tree(saved_st, vw_cache, vh_cache);
            if (new_root) {
                /* Free old root */
                if (current_root != root) free_layout_tree(current_root);
                current_root = new_root;
            }
            /* Re-collect focus list */
            focus_count = collect_focus_list(current_root, focus_list, 256);
            /* Re-patch INPUT text_content from input_buf */
            patch_input_text(focus_list, focus_count, input_buf, input_buf_count);
            /* Re-apply user text overrides (survive layout rebuilds) */
            if (cb) {
                for (int i = 0; i < cb->text_override_count; i++) {
                    LayoutNode* ov = find_node_by_id(current_root, cb->text_overrides[i].id);
                    node_set_text(ov, cb->text_overrides[i].text);
                }
            }
        }

        /* Clamp scroll */
        s->scroll_x = scroll_x; s->scroll_y = scroll_y;
        screen_scroll_clamp(s, current_root->width, current_root->height);
        scroll_x = s->scroll_x; scroll_y = s->scroll_y;
    }

    /* Free the rebuilt root if it differs from original */
    if (current_root != root) free_layout_tree(current_root);
    screen_free(s);
}

#endif /* INTERACT_IMPLEMENTATION */
#endif /* INTERACT_H */

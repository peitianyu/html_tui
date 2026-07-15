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
    char status_msg[256];

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

    /* ─── Status bar configuration ─── */
    /**
     * Set to true to show the status bar at the bottom of the screen.
     * When false, the status bar row is used for content rendering instead.
     * Default: true.
     */
    bool show_statusbar;
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

/* ─── Select popup rect helper ──────────────────────────── */
typedef struct { int x, y, w, h; int max_vis; int max_w; } SelectPopupRect;

static SelectPopupRect calc_select_popup_rect(LayoutNode* sel_node, int scroll_x, int scroll_y,
                                                Screen* s, int select_popup_count,
                                                char select_popup_vals[][64], int sel_row) {
    SelectPopupRect r = {0,0,0,0,0,0};
    int sx, sy, sw, sh;
    node_abs_box(sel_node, scroll_x, scroll_y, &sx, &sy, &sw, &sh);
    r.x = sx;
    r.y = sy + sh;
    /* Compute max label width */
    r.max_w = 0;
    for (int oi = 0; oi < select_popup_count; oi++) {
        int w = (int)strlen(select_popup_vals[oi]);
        if (w > r.max_w) r.max_w = w;
    }
    r.w = r.max_w + 4;
    if (r.w > s->cols - r.x) r.w = s->cols - r.x;
    if (r.w < 4) r.w = 4;
    /* Clamp y */
    if (r.y < 0) r.y = 0;
    r.max_vis = select_popup_count;
    if (r.max_vis > s->rows - r.y - 1) r.max_vis = s->rows - r.y - 1;
    if (r.y + r.max_vis > s->rows) r.max_vis = s->rows - r.y;
    if (r.max_vis > select_popup_count) r.max_vis = select_popup_count;
    if (r.max_vis < 1 && select_popup_count > 0) r.max_vis = 1;
    r.h = r.max_vis + 2; /* +2 for top+bottom border rows */
    return r;
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

/** Patch INPUT/TEXTAREA text_content from saved input buffers after a layout rebuild.
 *  Skips checkbox/radio/color; range regenerated from buffer; password masked with •. */
static void patch_input_text(LayoutNode** focus_list, int focus_count,
                              char input_buf[][4096], int input_buf_count) {
    for (int fi = 0; fi < focus_count && fi < input_buf_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (!n->styled || !n->styled->node ||
            n->styled->node->type != GUMBO_NODE_ELEMENT) continue;
        GumboTag t = n->styled->node->v.element.tag;
        if (t != GUMBO_TAG_INPUT && t != GUMBO_TAG_TEXTAREA) continue;
        const char* type_val = NULL;
        if (t == GUMBO_TAG_INPUT) {
            GumboAttribute* ta = gumbo_get_attribute(&n->styled->node->v.element.attributes, "type");
            if (ta && ta->value) type_val = ta->value;
            if (!type_val) type_val = "text";
        }
        if (t == GUMBO_TAG_INPUT && type_val &&
            (strcmp(type_val, "checkbox") == 0 || strcmp(type_val, "radio") == 0 ||
             strcmp(type_val, "color") == 0)) continue;
        if (t == GUMBO_TAG_INPUT && type_val && strcmp(type_val, "range") == 0) {
            int pct = 50;
            if (input_buf[fi][0]) pct = (int)strtol(input_buf[fi], NULL, 10);
            if (pct < 0) pct = 0; if (pct > 100) pct = 100;
            int bar_w = 20, thumb_pos = (pct * (bar_w - 1)) / 100;
            char _rb[256]; int pos = 0;
            _rb[pos++] = '[';
            for (int k = 1; k < bar_w; k++) {
                if (k <= thumb_pos) {
                    _rb[pos++] = 0xE2; _rb[pos++] = 0x95; _rb[pos++] = 0x90;
                } else if (k == thumb_pos + 1) {
                    _rb[pos++] = 0xE2; _rb[pos++] = 0x97; _rb[pos++] = 0x8F;
                } else {
                    _rb[pos++] = ' ';
                }
            }
            _rb[pos++] = ']';
            snprintf(_rb + pos, sizeof(_rb) - pos, " %d%%", pct);
            if (n->text_content) free(n->text_content);
            n->text_content = strdup(_rb);
            n->width = uc_str_width(n->text_content);
            n->color.r = 180; n->color.g = 220; n->color.b = 255;
            continue;
        }
        char buf[4096]; const char* val = input_buf[fi];
        if (t == GUMBO_TAG_INPUT && type_val && strcmp(type_val, "password") == 0) {
            if (val && val[0]) {
                int vlen = (int)strlen(val); int lim = ((int)sizeof(buf)-2)/3;
                if (vlen > lim) vlen = lim;
                int pos = 0;
                for (int k = 0; k < vlen; k++) {
                    buf[pos++] = 0xE2; buf[pos++] = 0x80; buf[pos++] = 0xA2;
                }
                buf[pos] = '\0';
            } else strcpy(buf, " ");
        } else {
            if (val && val[0]) snprintf(buf, sizeof(buf), "%s", val);
            else snprintf(buf, sizeof(buf), " ");
        }
        if (n->text_content) free(n->text_content);
        n->text_content = strdup(buf);
    }
}

/* ─── Find deepest element node at screen coordinates ──────── */
static GumboNode* find_deepest_node_at(LayoutNode* root, int x, int y, int scroll_x, int scroll_y) {
    GumboNode* result = NULL;
    LayoutNode* stack[256]; int sp = 0;
    stack[sp++] = root;
    int best_depth = -1;
    while (sp > 0) {
        LayoutNode* n = stack[--sp];
        for (size_t ci = 0; ci < n->num_children && sp < 256; ci++)
            stack[sp++] = n->children[ci];
        int hx, hy, hw, hh;
        node_abs_box(n, scroll_x, scroll_y, &hx, &hy, &hw, &hh);
        int depth = 0;
        { LayoutNode* pp = n; while (pp) { depth++; pp = pp->parent; } }
        if (x >= hx && x < hx + hw && y >= hy && y < hy + hh && depth > best_depth) {
            if (n->styled && n->styled->node &&
                n->styled->node->type == GUMBO_NODE_ELEMENT) {
                result = n->styled->node;
                best_depth = depth;
            }
        }
    }
    return result;
}

/* ─── Draw focus indicator around a focused element ─────────── */
static void draw_focus_indicator(Screen* s, LayoutNode* f, int scroll_x, int scroll_y,
                                  char input_buf[][4096], int* input_cursor,
                                  int* textarea_scroll_y, int* textarea_scroll_x,
                                  int focus_idx) {
    int bx, by, bw, bh;
    node_abs_box(f, scroll_x, scroll_y, &bx, &by, &bw, &bh);
    if (bw <= 0 || bh < 1 || by < 0 || by >= s->rows) return;
    int bg_r = 80, bg_g = 80, bg_b = 120;
    for (int ri = 0; ri < bh && by + ri < s->rows; ri++) {
        for (int ci = 0; ci < bw && bx + ci < s->cols; ci++) {
            if (by+ri >= 0 && bx+ci >= 0)
                screen_scr_bg(s, bx+ci, by+ri, bg_r, bg_g, bg_b);
        }
    }
    if (!f->styled || !f->styled->node || f->styled->node->type != GUMBO_NODE_ELEMENT ||
        !f->text_content) return;
    GumboTag tag = f->styled->node->v.element.tag;
    if (tag != GUMBO_TAG_INPUT && tag != GUMBO_TAG_TEXTAREA) return;

    /* Range & color: no text cursor */
    if (tag == GUMBO_TAG_INPUT) {
        GumboAttribute* ft = gumbo_get_attribute(&f->styled->node->v.element.attributes, "type");
        if (ft && ft->value &&
            (strcmp(ft->value, "range") == 0 || strcmp(ft->value, "color") == 0 ||
             strcmp(ft->value, "checkbox") == 0 || strcmp(ft->value, "radio") == 0)) return;
    }

    int cursor_pos = input_cursor[focus_idx];
    int cu = bx + f->border_left + f->padding_left;
    int cursor_row = by + f->border_top + f->padding_top;

    if (tag == GUMBO_TAG_TEXTAREA) {
        const char* txt = input_buf[focus_idx];
        int bo = 0, line_idx = 0, line_start_bo = 0;
        while (bo < cursor_pos && txt[bo]) {
            if (txt[bo] == '\n') { line_idx++; line_start_bo = bo + 1; }
            bo++;
        }
        int visual_col = uc_str_width_len(txt + line_start_bo, cursor_pos - line_start_bo);
        cu += visual_col - textarea_scroll_x[focus_idx];
        cursor_row += line_idx - textarea_scroll_y[focus_idx];
    } else {
        if (cursor_pos > 0) cu += uc_str_width_len(input_buf[focus_idx], cursor_pos);
    }

    int ca_left = bx + f->border_left + f->padding_left;
    int ca_top  = by + f->border_top  + f->padding_top;
    int ca_right = ca_left + f->width;
    int ca_bottom = ca_top + f->height;
    if (cu >= 0 && cu < s->cols && cursor_row >= 0 && cursor_row < s->rows &&
        cu >= ca_left && cu < ca_right &&
        cursor_row >= ca_top && cursor_row < ca_bottom) {
        Cell* cur_cell = &s->cells[cursor_row * s->cols + cu];
        int cf_r = cur_cell->fg_r, cf_g = cur_cell->fg_g, cf_b = cur_cell->fg_b;
        int cb_r = cur_cell->bg_r, cb_g = cur_cell->bg_g, cb_b = cur_cell->bg_b;
        screen_scr_fg(s, cu, cursor_row, cb_r, cb_g, cb_b);
        screen_scr_bg(s, cu, cursor_row, cf_r, cf_g, cf_b);
    }
}

/* ─── Draw status bar at bottom ─────────────────────────────── */
static void draw_status_bar(Screen* s, InteractCallbacks* cb, int scroll_x, int scroll_y,
                             LayoutNode* current_root) {
    if (!cb || !cb->show_statusbar || !cb->status_msg[0]) return;
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
    if (scroll_y > 0) {
        int ri = s->cols - 3;
        if (ri > ci && ri < s->cols) { screen_scr_set(s, ri, sb_row, 0x25B2); screen_scr_fg(s, ri, sb_row, 100,180,255); }
    }
    if (current_root && scroll_y < current_root->height - s->rows) {
        int ri = s->cols - 2;
        if (ri > ci && ri < s->cols) { screen_scr_set(s, ri, sb_row, 0x25BC); screen_scr_fg(s, ri, sb_row, 100,180,255); }
    }
}

/* ─── Helper: toggle checkbox/radio text ───────────── */
static void toggle_cb_radio_text(LayoutNode* n, InteractCallbacks* cb) {
    if (!n->text_content) return;
    char _new[128];
    const char* _old = n->text_content;
    const char* _label = strchr(_old, ']'); if (!_label) _label = strchr(_old, ')'); _label += 2;
    if (_old[0] == '[') {
        bool _was = (_old[1] == 'x');
        snprintf(_new, sizeof(_new), _was ? "[ ] %s" : "[x] %s", _label);
        n->color.r = _was ? 200 : 120; n->color.g = _was ? 200 : 220; n->color.b = _was ? 200 : 120;
    } else {
        bool _was = ((unsigned char)_old[1] == 0xE2);
        snprintf(_new, sizeof(_new), _was ? "( ) %s" : "(•) %s", _label);
        n->color.r = _was ? 200 : 120; n->color.g = _was ? 200 : 220; n->color.b = _was ? 200 : 120;
    }
    node_set_text(n, _new);
    GumboAttribute* _id_a = gumbo_get_attribute(&n->styled->node->v.element.attributes, "id");
    if (_id_a && _id_a->value) node_override_text(cb, _id_a->value, _new);
    snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", _new);
}

static bool is_cb_radio_node(LayoutNode* n) {
    if (!n->styled || !n->styled->node || n->styled->node->type != GUMBO_NODE_ELEMENT) return false;
    if (n->styled->node->v.element.tag != GUMBO_TAG_INPUT) return false;
    GumboAttribute* ta = gumbo_get_attribute(&n->styled->node->v.element.attributes, "type");
    return ta && ta->value && (strcmp(ta->value, "checkbox") == 0 || strcmp(ta->value, "radio") == 0);
}

/* ─── Helper: uncheck sibling radios in same group ── */
static void radio_group_uncheck(LayoutNode* n, LayoutNode** focus_list, int focus_count, InteractCallbacks* cb) {
    GumboAttribute* name_a = gumbo_get_attribute(&n->styled->node->v.element.attributes, "name");
    if (!name_a || !name_a->value) return;
    for (int i = 0; i < focus_count; i++) {
        if (focus_list[i] == n) continue;
        if (!focus_list[i]->styled || !focus_list[i]->styled->node) continue;
        if (focus_list[i]->styled->node->type != GUMBO_NODE_ELEMENT) continue;
        if (focus_list[i]->styled->node->v.element.tag != GUMBO_TAG_INPUT) continue;
        GumboAttribute* ta = gumbo_get_attribute(&focus_list[i]->styled->node->v.element.attributes, "type");
        if (!ta || !ta->value || strcmp(ta->value, "radio") != 0) continue;
        GumboAttribute* rn = gumbo_get_attribute(&focus_list[i]->styled->node->v.element.attributes, "name");
        if (!rn || !rn->value || strcmp(rn->value, name_a->value) != 0) continue;
        /* Already unchecked? skip */
        if (!focus_list[i]->text_content || focus_list[i]->text_content[0] != '(') continue;
        const char* _ro = focus_list[i]->text_content;
        if ((unsigned char)_ro[1] != 0xE2) continue; /* not checked */
        const char* _rl = strchr(_ro, ')');
        if (_rl) {
            char _rnb[128];
            snprintf(_rnb, sizeof(_rnb), "( ) %s", _rl + 2);
            node_set_text(focus_list[i], _rnb);
            focus_list[i]->color.r = 200; focus_list[i]->color.g = 200; focus_list[i]->color.b = 200;
            GumboAttribute* _ria = gumbo_get_attribute(&focus_list[i]->styled->node->v.element.attributes, "id");
            if (_ria && _ria->value) node_override_text(cb, _ria->value, _rnb);
        }
    }
}

/* ─── Helper: update range slider text from percentage ─ */
static void range_update_text(LayoutNode* n, int pct, InteractCallbacks* cb) {
    if (!n) return;
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    int bar_w = 20, thumb_pos = (pct * (bar_w - 1)) / 100;
    char buf[256]; int pos = 0;
    buf[pos++] = '[';
    for (int k = 1; k < bar_w; k++) {
        if (k <= thumb_pos) {
            buf[pos++] = 0xE2; buf[pos++] = 0x95; buf[pos++] = 0x90; /* ═ */
        } else if (k == thumb_pos + 1) {
            buf[pos++] = 0xE2; buf[pos++] = 0x97; buf[pos++] = 0x8F; /* ● */
        } else {
            buf[pos++] = ' ';
        }
    }
    buf[pos++] = ']';
    snprintf(buf + pos, sizeof(buf) - pos, " %d%%", pct);
    node_set_text(n, buf);
    n->width = uc_str_width(buf);
    n->color.r = 180; n->color.g = 220; n->color.b = 255;
    GumboAttribute* _ia = gumbo_get_attribute(&n->styled->node->v.element.attributes, "id");
    if (_ia && _ia->value) node_override_text(cb, _ia->value, buf);
}

/* ─── Helper: sync textarea scroll for rendering ──── */
static void sync_textarea_scroll(LayoutNode** focus_list, int focus_count, int focus_idx,
                                  int* textarea_scroll_y, int* textarea_scroll_x) {
    if (focus_idx >= 0 && focus_idx < focus_count) {
        LayoutNode* f = focus_list[focus_idx];
        if (f->styled && f->styled->node &&
            f->styled->node->type == GUMBO_NODE_ELEMENT &&
            f->styled->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
            f->content_scroll_y = textarea_scroll_y[focus_idx];
            f->content_scroll_x = textarea_scroll_x[focus_idx];
        }
    }
}

/* ─── Helper: restyle on hover/focus/active change ── */
static void do_restyle(KatanaStylesheet* css, StyledNode* st_root,
                       LayoutNode** current_root, LayoutNode* original_root,
                       int w, int h,
                       LayoutNode** focus_list, int* focus_count,
                       char input_buf[][4096], int input_buf_count,
                       GumboNode* g_hover, GumboNode* g_focus,
                       GumboNode* g_active, 
                       GumboNode** p_hover, GumboNode** p_active,
                       InteractCallbacks* cb)
{
    StyledNode* nodes[8]; int cnt = 0;
    StyledNode* sn;
    #define ADD_SN(n) do { if (n) { sn = find_styled_node(st_root, n); if (sn) { bool dup=false; for(int _i=0;_i<cnt;_i++) if(nodes[_i]==sn){dup=true;break;} if(!dup)nodes[cnt++]=sn; } } } while(0)
    if (p_hover && *p_hover && *p_hover != g_hover && *p_hover != g_focus) ADD_SN(*p_hover);
    ADD_SN(g_hover);
    if (g_focus != g_hover) ADD_SN(g_focus);
    if (p_active && *p_active && *p_active != g_hover && *p_active != g_focus && *p_active != g_active) ADD_SN(*p_active);
    #undef ADD_SN
    if (p_hover) *p_hover = NULL; if (p_active) *p_active = NULL;
    for (int i = 0; i < cnt; i++) recompute_style_subtree(nodes[i], css, NULL);
    LayoutNode* nr = build_layout_tree(st_root, w, h);
    if (nr) { if (*current_root != original_root) free_layout_tree(*current_root); *current_root = nr; }
    *focus_count = collect_focus_list(*current_root, focus_list, 256);
    patch_input_text(focus_list, *focus_count, input_buf, input_buf_count);
    if (cb) for (int i = 0; i < cb->text_override_count; i++) {
        LayoutNode* ov = find_node_by_id(*current_root, cb->text_overrides[i].id);
        node_set_text(ov, cb->text_overrides[i].text);
    }
}

/* ─── Helper: render one frame ─────────────────────── */
static void render_frame(Screen* s, LayoutNode* current_root, int scroll_x, int scroll_y,
                          LayoutNode** focus_list, int focus_count, int focus_idx,
                          char input_buf[][4096], int* input_cursor,
                          int* textarea_scroll_y, int* textarea_scroll_x,
                          InteractCallbacks* cb,
                          int select_popup_active, int select_popup_focus_idx,
                          int select_popup_count, char select_popup_vals[][64],
                          int select_popup_types[32], int select_popup_sel,
                          int select_popup_scroll)
{
    sync_textarea_scroll(focus_list, focus_count, focus_idx, textarea_scroll_y, textarea_scroll_x);
    screen_clear(s);
    s->scroll_x = scroll_x; s->scroll_y = scroll_y;
    screen_render_tree(s, current_root);
    if (focus_idx >= 0 && focus_idx < focus_count)
        draw_focus_indicator(s, focus_list[focus_idx], scroll_x, scroll_y,
                             input_buf, input_cursor, textarea_scroll_y, textarea_scroll_x, focus_idx);
    if (cb && cb->show_scrollbars && current_root) {
        screen_draw_hscrollbar(s, current_root->width, scroll_x);
        screen_draw_vscrollbar(s, current_root->height, scroll_y);
    }
    draw_status_bar(s, cb, scroll_x, scroll_y, current_root);
    /* Select popup overlay */
    if (select_popup_active && select_popup_focus_idx >= 0 && select_popup_focus_idx < focus_count) {
        LayoutNode* sn = focus_list[select_popup_focus_idx];
        SelectPopupRect pr = calc_select_popup_rect(sn, scroll_x, scroll_y, s,
                                                     select_popup_count, select_popup_vals, select_popup_sel);
        int px=pr.x, py=pr.y, pw=pr.w, mv=pr.max_vis, bc=100;
        for (int ci=px; ci<px+pw&&ci<s->cols; ci++) for (int ri=py; ri<py+mv+2&&ri<s->rows; ri++)
            { screen_scr_set(s,ci,ri,' '); screen_scr_bg(s,ci,ri,20,20,40); }
        if (py < s->rows) {
            for (int ci=px; ci<px+pw&&ci<s->cols; ci++) { screen_scr_set(s,ci,py,0x2500); screen_scr_fg(s,ci,py,bc,bc,bc); screen_scr_bg(s,ci,py,20,20,40); }
            if (px<s->cols) { screen_scr_set(s,px,py,0x250C); screen_scr_fg(s,px,py,bc,bc,bc); }
            int r=px+pw-1; if (r<s->cols) { screen_scr_set(s,r,py,0x2510); screen_scr_fg(s,r,py,bc,bc,bc); }
        }
        int br=py+mv+1;
        if (br < s->rows) {
            for (int ci=px; ci<px+pw&&ci<s->cols; ci++) { screen_scr_set(s,ci,br,0x2500); screen_scr_fg(s,ci,br,bc,bc,bc); screen_scr_bg(s,ci,br,20,20,40); }
            if (px<s->cols) { screen_scr_set(s,px,br,0x2514); screen_scr_fg(s,px,br,bc,bc,bc); }
            int r=px+pw-1; if (r<s->cols) { screen_scr_set(s,r,br,0x2518); screen_scr_fg(s,r,br,bc,bc,bc); }
        }
        for (int ri=py+1; ri<py+mv+1&&ri<s->rows; ri++) {
            if (px<s->cols) { screen_scr_set(s,px,ri,0x2502); screen_scr_fg(s,px,ri,bc,bc,bc); }
            int r=px+pw-1; if (r<s->cols) { screen_scr_set(s,r,ri,0x2502); screen_scr_fg(s,r,ri,bc,bc,bc); }
        }
        for (int oi=0; oi<mv; oi++) {
            int idx=select_popup_scroll+oi; if (idx>=select_popup_count) break;
            int row=py+1+oi; if (row>=s->rows) break;
            int bg_r=(idx==select_popup_sel)?80:(select_popup_types[idx]==1?30:20);
            int bg_g=(idx==select_popup_sel)?80:(select_popup_types[idx]==1?40:20);
            int bg_b=(idx==select_popup_sel)?140:(select_popup_types[idx]==1?60:40);
            int col=px+1; const char* lb=select_popup_vals[idx];
            screen_scr_bg(s,col,row,bg_r,bg_g,bg_b);
            if (select_popup_types[idx]==1) { screen_scr_set(s,col,row,0x2503); screen_scr_fg(s,col,row,180,200,120); screen_scr_bold(s,col,row,true); }
            else if (idx==select_popup_sel) { screen_scr_set(s,col,row,0x25B6); screen_scr_fg(s,col,row,200,200,255); }
            else screen_scr_set(s,col,row,' ');
            int ci=col+1; while (*lb&&ci<px+pw-1&&ci<s->cols) {
                uint32_t cp=uc_dec(&lb); if (cp==0) break;
                screen_scr_set(s,ci,row,cp); screen_scr_fg(s,ci,row,220,220,220); screen_scr_bg(s,ci,row,bg_r,bg_g,bg_b);
                int w=uc_wid((int)cp); ci+=w; if (w==2&&ci<s->cols) screen_scr_set(s,ci,row,0);
            }
            while (ci<px+pw-1&&ci<s->cols) { screen_scr_set(s,ci,row,' '); screen_scr_bg(s,ci,row,bg_r,bg_g,bg_b); ci++; }
        }
        if (select_popup_scroll>0&&py+1<s->rows) { screen_scr_set(s,px+pw-2,py+1,0x25B2); screen_scr_fg(s,px+pw-2,py+1,100,180,255); }
        if (select_popup_scroll+mv<select_popup_count&&py+mv<s->rows) { screen_scr_set(s,px+pw-2,py+mv,0x25BC); screen_scr_fg(s,px+pw-2,py+mv,100,180,255); }
    }
    screen_flush(s);
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
    char input_buf[256][4096] = {{0}};
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
    int  select_popup_types[32];        /* 0=option, 1=group header */
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
                const char* type_val = NULL;
                GumboAttribute* ta = gumbo_get_attribute(
                    &n->styled->node->v.element.attributes, "type");
                if (ta && ta->value) type_val = ta->value;
                if (type_val && (strcmp(type_val, "checkbox") == 0 ||
                                 strcmp(type_val, "radio") == 0 ||
                                 strcmp(type_val, "color") == 0)) continue;
                /* Range: store numeric value in input_buf */
                if (type_val && strcmp(type_val, "range") == 0) {
                    GumboAttribute* attr = gumbo_get_attribute(
                        &n->styled->node->v.element.attributes, "value");
                    if (attr && attr->value)
                        strncpy(input_buf[fi], attr->value, sizeof(input_buf[0])-1);
                    else
                        strncpy(input_buf[fi], "50", sizeof(input_buf[0])-1);
                    continue;
                }
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

    /* Range slider drag state */
    int range_drag_idx = -1;

    /* Track previous hover/active for style recomputation */
    GumboNode* prev_hover = NULL;
    GumboNode* prev_active = NULL;

    while (running) {
        /* ── Check quit request from callback ── */
        if (cb && cb->quit_flag) { running = false; break; }

        /* ── Sync runtime info to callbacks ── */
        if (cb) cb->layout_root = current_root;

        /* ── Render frame ── */
        render_frame(s, current_root, scroll_x, scroll_y,
                     focus_list, focus_count, focus_idx,
                     input_buf, input_cursor,
                     textarea_scroll_y, textarea_scroll_x,
                     cb, select_popup_active, select_popup_focus_idx,
                     select_popup_count, select_popup_vals,
                     select_popup_types, select_popup_sel, select_popup_scroll);

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
                GumboNode* hover_node = find_deepest_node_at(current_root, ev.x, ev.y, scroll_x, scroll_y);
                if (hover_node != g_interact_hover) {
                    prev_hover = g_interact_hover;
                    g_interact_hover = hover_node;
                    restyle = true;
                }

                /* Left click → focus + active (not pure motion) */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION)) {
                    /* ── Select popup mouse click ── */
                    if (select_popup_active) {
                        LayoutNode* _psn = focus_list[select_popup_focus_idx];
                        SelectPopupRect pr = calc_select_popup_rect(_psn, scroll_x, scroll_y, s,
                                                                     select_popup_count, select_popup_vals, select_popup_sel);
                        /* Check if click is inside popup area */
                        if (ev.x >= pr.x && ev.x < pr.x + pr.w &&
                            ev.y > pr.y && ev.y <= pr.y + pr.max_vis) {
                            int oi = (ev.y - pr.y - 1) + select_popup_scroll;
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
                    GumboNode* clicked_elem = find_deepest_node_at(current_root, ev.x, ev.y, scroll_x, scroll_y);
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
                /* Click on checkbox/radio -- toggle */
                if (focus_idx >= 0 && focus_idx < focus_count) {
                    LayoutNode* _n = focus_list[focus_idx];
                    if (_n->styled && _n->styled->node &&
                        _n->styled->node->type == GUMBO_NODE_ELEMENT &&
                        _n->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                        GumboAttribute* _ta = gumbo_get_attribute(
                            &_n->styled->node->v.element.attributes, "type");
                        if (_ta && _ta->value && (strcmp(_ta->value, "checkbox") == 0 ||
                                                   strcmp(_ta->value, "radio") == 0)) {
                            if (_n->text_content) {
                                char _nx[128]; const char* _ox = _n->text_content;
                                const char* _lx = strchr(_ox, ']'); if (!_lx) _lx = strchr(_ox, ')'); _lx += 2;
                                if (_ox[0] == '[') {
                                    bool _w = (_ox[1] == 'x');
                                    snprintf(_nx, sizeof(_nx), _w ? "[ ] %s" : "[x] %s", _lx);
                                    _n->color.r = _w ? 200 : 120; _n->color.g = _w ? 200 : 220; _n->color.b = _w ? 200 : 120;
                                    node_set_text(_n, _nx);
                                    GumboAttribute* _ia = gumbo_get_attribute(
                                        &_n->styled->node->v.element.attributes, "id");
                                    if (_ia && _ia->value) node_override_text(cb, _ia->value, _nx);
                                    snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", _nx);
                                } else {
                                    bool _w = ((unsigned char)_ox[1] == 0xE2);
                                    if (!_w) {
                                        radio_group_uncheck(_n, focus_list, focus_count, cb);
                                        snprintf(_nx, sizeof(_nx), "(•) %s", _lx);
                                        _n->color.r = 120; _n->color.g = 220; _n->color.b = 120;
                                        node_set_text(_n, _nx);
                                        GumboAttribute* _ia = gumbo_get_attribute(
                                            &_n->styled->node->v.element.attributes, "id");
                                        if (_ia && _ia->value) node_override_text(cb, _ia->value, _nx);
                                        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", _nx);
                                    }
                                }
                            }
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
                    range_drag_idx = -1;
                    if (g_interact_active) {
                        prev_active = g_interact_active;
                        g_interact_active = NULL;
                        restyle = true;
                    }
                }

                /* Right click → context menu (show element info in status bar) */
                if (ev.key == TB_KEY_MOUSE_RIGHT && !(ev.mod & TB_MOD_MOTION) && cb) {
                    GumboNode* clicked = find_deepest_node_at(current_root, ev.x, ev.y, scroll_x, scroll_y);
                    if (clicked && clicked->type == GUMBO_NODE_ELEMENT) {
                        const char* tn = gumbo_normalized_tagname(clicked->v.element.tag);
                        GumboAttribute* id_a = gumbo_get_attribute(&clicked->v.element.attributes, "id");
                        GumboAttribute* cl_a = gumbo_get_attribute(&clicked->v.element.attributes, "class");
                        char ctx[128] = "";
                        snprintf(ctx, sizeof(ctx), "📍 <%s>", tn);
                        if (id_a && id_a->value) {
                            int cl = (int)strlen(ctx);
                            snprintf(ctx + cl, sizeof(ctx) - cl, " #%s", id_a->value);
                        }
                        if (cl_a && cl_a->value) {
                            int cl = (int)strlen(ctx);
                            snprintf(ctx + cl, sizeof(ctx) - cl, " .%s", cl_a->value);
                        }
                        snprintf(cb->status_msg, sizeof(cb->status_msg), "%s", ctx);
                    } else {
                        snprintf(cb->status_msg, sizeof(cb->status_msg), "📍 (no element)");
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

                /* ── Range slider click ── */
                if (ev.key == TB_KEY_MOUSE_LEFT && !(ev.mod & TB_MOD_MOTION)) {
                    for (int fi = 0; fi < focus_count; fi++) {
                        LayoutNode* n = focus_list[fi];
                        if (!n->styled || !n->styled->node ||
                            n->styled->node->type != GUMBO_NODE_ELEMENT ||
                            n->styled->node->v.element.tag != GUMBO_TAG_INPUT)
                            continue;
                        GumboAttribute* rt = gumbo_get_attribute(
                            &n->styled->node->v.element.attributes, "type");
                        if (!rt || !rt->value || strcmp(rt->value, "range") != 0) continue;
                        int nx, ny, nw, nh;
                        node_abs_box(n, scroll_x, scroll_y, &nx, &ny, &nw, &nh);
                        if (ev.x >= nx && ev.x < nx + nw && ev.y >= ny && ev.y < ny + nh) {
                            /* Calculate percentage from X within bar */
                            int bar_left = nx + n->border_left + n->padding_left + 1; /* after '[' */
                            int bar_w = 20;
                            int rel_x = ev.x - bar_left;
                            if (rel_x < 0) rel_x = 0;
                            if (rel_x >= bar_w) rel_x = bar_w - 1;
                            int pct = (rel_x * 100) / (bar_w - 1);
                            if (pct < 0) pct = 0; if (pct > 100) pct = 100;
                            snprintf(input_buf[fi], sizeof(input_buf[0]), "%d", pct);
                            range_update_text(n, pct, cb);
                            range_drag_idx = fi;
                            snprintf(cb->status_msg, sizeof(cb->status_msg),
                                     "↔ Range: %d%%", pct);
                            break;
                        }
                    }
                }

                /* ── Range slider drag ── */
                if (ev.key == TB_KEY_MOUSE_LEFT && (ev.mod & TB_MOD_MOTION) && range_drag_idx >= 0) {
                    int fi = range_drag_idx;
                    if (fi >= 0 && fi < focus_count) {
                        LayoutNode* n = focus_list[fi];
                        int nx, ny, nw, nh;
                        node_abs_box(n, scroll_x, scroll_y, &nx, &ny, &nw, &nh);
                        int bar_left = nx + n->border_left + n->padding_left + 1;
                        int bar_w = 20;
                        int rel_x = ev.x - bar_left;
                        if (rel_x < 0) rel_x = 0;
                        if (rel_x >= bar_w) rel_x = bar_w - 1;
                        int pct = (rel_x * 100) / (bar_w - 1);
                        if (pct < 0) pct = 0; if (pct > 100) pct = 100;
                        snprintf(input_buf[fi], sizeof(input_buf[0]), "%d", pct);
                        range_update_text(n, pct, cb);
                        snprintf(cb->status_msg, sizeof(cb->status_msg),
                                 "↔ Range: %d%%", pct);
                        restyle = true;
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
                if (!is_textarea) {
                    GumboAttribute* ct = gumbo_get_attribute(
                        &focus_list[focus_idx]->styled->node->v.element.attributes, "type");
                    if (ct && ct->value && (strcmp(ct->value, "checkbox") == 0 ||
                                             strcmp(ct->value, "radio") == 0)) {
                        if (ev.key == TB_KEY_SPACE || ev.key == TB_KEY_ENTER) {
                            LayoutNode* cn = focus_list[focus_idx];
                            if (cn->text_content) {
                                char _nx[128]; const char* _ox = cn->text_content;
                                const char* _lx = strchr(_ox, ']'); if (!_lx) _lx = strchr(_ox, ')'); _lx += 2;
                                if (_ox[0] == '[') {
                                    bool _w = (_ox[1] == 'x');
                                    snprintf(_nx, sizeof(_nx), _w ? "[ ] %s" : "[x] %s", _lx);
                                    cn->color.r = _w ? 200 : 120; cn->color.g = _w ? 200 : 220; cn->color.b = _w ? 200 : 120;
                                    node_set_text(cn, _nx);
                                    GumboAttribute* _ia = gumbo_get_attribute(&cn->styled->node->v.element.attributes, "id");
                                    if (_ia && _ia->value) node_override_text(cb, _ia->value, _nx);
                                    snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", _nx);
                                } else {
                                    bool _w = ((unsigned char)_ox[1] == 0xE2);
                                    if (!_w) {
                                        radio_group_uncheck(cn, focus_list, focus_count, cb);
                                        snprintf(_nx, sizeof(_nx), "(•) %s", _lx);
                                        cn->color.r = 120; cn->color.g = 220; cn->color.b = 120;
                                        node_set_text(cn, _nx);
                                        GumboAttribute* _ia = gumbo_get_attribute(&cn->styled->node->v.element.attributes, "id");
                                        if (_ia && _ia->value) node_override_text(cb, _ia->value, _nx);
                                        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", _nx);
                                    }
                                }
                            }
                        }
                        continue;
                    }
                    /* Range & color: only drag/click, no keyboard input */
                    if (ct && ct->value &&
                        (strcmp(ct->value, "range") == 0 || strcmp(ct->value, "color") == 0)) {
                        continue;
                    }
                }
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
                /* Collect option values from the <select>'s children (<option> and <optgroup>) */
                select_popup_focus_idx = focus_idx;
                select_popup_count = 0;
                select_popup_sel = 0;
                select_popup_scroll = 0;
                const char* cur_val = focus_list[focus_idx]->text_content;
                GumboVector* ch = &focus_list[focus_idx]->styled->node->v.element.children;
                for (unsigned int oi = 0; oi < ch->length && select_popup_count < 32; oi++) {
                    GumboNode* oc = (GumboNode*)ch->data[oi];
                    if (oc->type != GUMBO_NODE_ELEMENT) continue;
                    if (oc->v.element.tag == GUMBO_TAG_OPTION) {
                        GumboAttribute* oa = gumbo_get_attribute(&oc->v.element.attributes, "value");
                        if (oa && oa->value) {
                            snprintf(select_popup_vals[select_popup_count], 64, "%s", oa->value);
                            select_popup_types[select_popup_count] = 0;
                            if (cur_val && strcmp(oa->value, cur_val) == 0)
                                select_popup_sel = select_popup_count;
                            select_popup_count++;
                        }
                    } else if (oc->v.element.tag == GUMBO_TAG_OPTGROUP) {
                        GumboAttribute* gl = gumbo_get_attribute(&oc->v.element.attributes, "label");
                        if (gl && gl->value && gl->value[0] && select_popup_count < 32) {
                            snprintf(select_popup_vals[select_popup_count], 64, "%s", gl->value);
                            select_popup_types[select_popup_count] = 1;
                            select_popup_count++;
                        }
                        /* Add optgroup's <option> children */
                        GumboVector* og_ch = &oc->v.element.children;
                        for (unsigned int gi = 0; gi < og_ch->length && select_popup_count < 32; gi++) {
                            GumboNode* ogc = (GumboNode*)og_ch->data[gi];
                            if (ogc->type == GUMBO_NODE_ELEMENT && ogc->v.element.tag == GUMBO_TAG_OPTION) {
                                GumboAttribute* oa = gumbo_get_attribute(&ogc->v.element.attributes, "value");
                                if (oa && oa->value) {
                                    snprintf(select_popup_vals[select_popup_count], 64, "%s", oa->value);
                                    select_popup_types[select_popup_count] = 0;
                                    if (cur_val && strcmp(oa->value, cur_val) == 0)
                                        select_popup_sel = select_popup_count;
                                    select_popup_count++;
                                }
                            }
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
            do_restyle(saved_css, saved_st, &current_root, root,
                       vw_cache, vh_cache,
                       focus_list, &focus_count, input_buf, input_buf_count,
                       g_interact_hover, g_interact_focus, g_interact_active,
                       &prev_hover, &prev_active, cb);
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

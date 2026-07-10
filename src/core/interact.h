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
                                       char input_buf[][64], int input_buf_count,
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
    char input_buf[256][64] = {{{0}}};
    int  input_buf_count = 0;

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
                    if (t == GUMBO_TAG_INPUT && input_buf[focus_count][0] == 0) {
                        GumboAttribute* attr = gumbo_get_attribute(
                            &n->styled->node->v.element.attributes, "value");
                        if (attr && attr->value)
                            strncpy(input_buf[focus_count], attr->value, sizeof(input_buf[0])-1);
                    }
                    focus_count++;
                }
            }
            for (size_t ci = 0; ci < n->num_children && sp < 256; ci++)
                stack[sp++] = n->children[ci];
        }
    }
    input_buf_count = focus_count;

    /* Patch INPUT text_content from input_buf */
    for (int fi = 0; fi < focus_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT &&
            n->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
            char buf[128];
            if (input_buf[fi][0]) snprintf(buf, sizeof(buf), "%s", input_buf[fi]);
            else snprintf(buf, sizeof(buf), " ");
            if (n->text_content) free(n->text_content);
            n->text_content = strdup(buf);
        }
    }

    /* Rebuild state */
    LayoutNode* current_root = root;
    KatanaStylesheet* saved_css = css;
    StyledNode* saved_st = styled_root;

    int vw_cache = vw, vh_cache = vh;

    while (running) {
        /* ── Check quit request from callback ── */
        if (cb && cb->quit_flag) { running = false; break; }

        /* ── Sync runtime info to callbacks ── */
        if (cb) cb->layout_root = current_root;

        /* ── Render frame ── */
        screen_clear(s);
        s->scroll_x = scroll_x;
        s->scroll_y = scroll_y;
        screen_render_tree(s, current_root);

        /* Focus indicator */
        if (focus_idx >= 0 && focus_idx < focus_count) {
            LayoutNode* f = focus_list[focus_idx];
            int bx, by, bw, bh;
            node_abs_box(f, scroll_x, scroll_y, &bx, &by, &bw, &bh);
            if (bw > 2 && bh > 1 && by >= 0 && by < s->rows) {
                for (int ri = 0; ri < bh && by + ri < s->rows; ri++) {
                    if (bx >= 0 && bx < s->cols)
                        { screen_scr_set(s, bx, by+ri, ' '); screen_scr_bg(s, bx, by+ri, 80,80,120); }
                    if (bx+bw-1 >= 0 && bx+bw-1 < s->cols)
                        { screen_scr_set(s, bx+bw-1, by+ri, ' '); screen_scr_bg(s, bx+bw-1, by+ri, 80,80,120); }
                }
                for (int ci = 0; ci < bw && bx + ci < s->cols; ci++) {
                    if (by >= 0 && bx+ci >= 0) { screen_scr_set(s, bx+ci, by, ' '); screen_scr_bg(s, bx+ci, by, 80,80,120); }
                    int btm = by+bh-1;
                    if (btm < s->rows && bx+ci >= 0) { screen_scr_set(s, bx+ci, btm, ' '); screen_scr_bg(s, bx+ci, btm, 80,80,120); }
                }
            }
            if (f->styled && f->styled->node && f->styled->node->type == GUMBO_NODE_ELEMENT &&
                f->styled->node->v.element.tag == GUMBO_TAG_INPUT &&
                f->text_content) {
                int cu = bx + f->border_left + f->padding_left;
                if (focus_idx >= 0 && focus_idx < input_buf_count)
                    cu += uc_str_width(input_buf[focus_idx]);
                int cursor_row = by + f->border_top + f->padding_top;
                if (cu >= 0 && cu < s->cols && cursor_row >= 0 && cursor_row < s->rows) {
                    screen_scr_set(s, cu, cursor_row, ' '); screen_scr_bg(s, cu, cursor_row, 200, 200, 80);
                    screen_scr_set(s, cu, cursor_row, '|'); screen_scr_fg(s, cu, cursor_row, 0, 0, 0);
                }
            }
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
        }

        screen_flush(s);

        /* Poll input */
        if (tb_poll_event(&ev) != TB_OK) {
            if (tb_last_errno() == EINTR) continue;
            break;
        }

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

                /* Left click on BUTTON: trigger click action via callback */
                if (ev.key == TB_KEY_MOUSE_LEFT && cb && cb->on_button_click) {
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
            /* ── User key handler (overrides defaults) ── */
            if (cb && cb->on_key && cb->on_key(&ev, cb)) {
                continue;
            }

            /* ── Input editing: when an INPUT is focused ── */
            if (focus_idx >= 0 && focus_idx < focus_count &&
                focus_list[focus_idx]->styled &&
                focus_list[focus_idx]->styled->node &&
                focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                char* buf = input_buf[focus_idx];
                size_t blen = strlen(buf);

                if (ev.key == TB_KEY_BACKSPACE2 || ev.key == TB_KEY_BACKSPACE) {
                    if (blen > 0) { buf[blen-1] = '\0'; }
                } else if (ev.key == TB_KEY_DELETE) {
                    /* no-op */
                } else if (ev.ch >= 0x20 && ev.ch <= 0x7e) {
                    if (blen < (int)sizeof(input_buf[0]) - 2) {
                        char add[8];
                        int nch = uc_enc(ev.ch, add); add[nch] = '\0';
                        strcat(buf, add);
                    }
                } else if (ev.key == TB_KEY_SPACE) {
                    if (blen < (int)sizeof(input_buf[0]) - 2) {
                        strcat(buf, " ");
                    }
                } else if (ev.key == TB_KEY_ENTER) {
                    if (focus_count > 0) {
                        focus_idx = (focus_idx + 1) % focus_count;
                        if (focus_idx >= 0 && focus_list[focus_idx]->styled)
                            g_interact_focus = focus_list[focus_idx]->styled->node;
                        else g_interact_focus = NULL;
                        restyle = true;
                    }
                    continue;
                } else {
                    goto handle_scroll_keys;
                }

                /* Update text_content */
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

            if (g_interact_focus && g_interact_active != g_interact_focus) {
                g_interact_active = g_interact_focus;
                restyle = true;
            }
        }

        /* Mouse wheel */
        if (ev.type == TB_EVENT_MOUSE) {
            if (ev.key == TB_KEY_MOUSE_WHEEL_UP) { scroll_y -= 3; }
            else if (ev.key == TB_KEY_MOUSE_WHEEL_DOWN) { scroll_y += 3; }
        }

        /* Resize */
        if (ev.type == TB_EVENT_RESIZE) {
            vw = ev.w; vh = ev.h;
            screen_free(s); s = screen_create(vw, vh);
            vw_cache = vw; vh_cache = vh;
            restyle = true;
        }

        /* If hover/focus/active changed, rebuild styles and layout */
        if (restyle && saved_css && saved_st) {
            if (g_interact_hover) {
                StyledNode* sn = find_styled_node(saved_st, g_interact_hover);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            if (g_interact_focus && g_interact_focus != g_interact_hover) {
                StyledNode* sn = find_styled_node(saved_st, g_interact_focus);
                if (sn) recompute_style_subtree(sn, saved_css, NULL);
            }
            LayoutNode* new_root = build_layout_tree(saved_st, vw_cache, vh_cache);
            if (new_root) {
                /* Free old root */
                if (current_root != root) free_layout_tree(current_root);
                current_root = new_root;
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
            /* Re-patch INPUT text_content from input_buf */
            for (int fi = 0; fi < focus_count && fi < input_buf_count; fi++) {
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

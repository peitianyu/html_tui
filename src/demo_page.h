/* ======================== demo_page.h ========================
 * Page configuration for the TUI browser demo.
 *
 * Loads HTML files from disk; each file contains both HTML structure
 * and inline CSS in <style> tags within <head>.
 *
 * Usage: tcc src/demo_main.c -I src -run [path/to/page.html]
 *   Default: doc/test_pages/00-menu.html
 * ============================================================== */

#ifndef DEMO_PAGE_H
#define DEMO_PAGE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/interact.h"

/* ==================================================================== */
/*  File I/O Helpers                                                     */
/* ==================================================================== */

static char* read_file_content(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len <= 0) { fclose(f); return NULL; }
    rewind(f);
    char* buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t nread = fread(buf, 1, (size_t)len, f);
    fclose(f);
    buf[nread] = '\0';
    if (out_len) *out_len = nread;
    return buf;
}

/* ==================================================================== */
/*  CSS Extraction from HTML String                                      */
/*                                                                       */
/*  Scans raw HTML for <style>...</style> blocks, extracts their content */
/*  as CSS, and produces a "clean" HTML with style blocks removed.       */
/* ==================================================================== */

#define CSS_BUF_SIZE (65536 * 4)

static void extract_css_from_html(const char* raw_html,
                                   char* css_out, size_t css_size,
                                   char* html_out, size_t html_size)
{
    css_out[0] = '\0';
    size_t css_pos = 0, html_pos = 0;
    const char* p = raw_html;

    while (*p && html_pos < html_size - 1) {
        /* Look for <style or <STYLE */
        const char* style_start = NULL;
        const char* scan = p;
        while (*scan) {
            if ((scan[0] == '<' && (scan[1] == 's' || scan[1] == 'S') &&
                 (scan[2] == 't' || scan[2] == 'T') &&
                 (scan[3] == 'y' || scan[3] == 'Y') &&
                 (scan[4] == 'l' || scan[4] == 'L') &&
                 (scan[5] == 'e' || scan[5] == 'E')) &&
                 (scan[6] == '>' || scan[6] == ' ' || scan[6] == '\t' ||
                  scan[6] == '\n' || scan[6] == '\r')) {
                style_start = scan;
                break;
            }
            scan++;
        }

        if (!style_start) {
            /* No more style blocks — copy rest */
            while (*p && html_pos < html_size - 1)
                html_out[html_pos++] = *p++;
            break;
        }

        /* Copy content before <style> to html_out */
        while (p < style_start && html_pos < html_size - 1)
            html_out[html_pos++] = *p++;

        /* Find closing > of <style ... > */
        const char* gt = strchr(style_start, '>');
        if (!gt) {
            while (*p && html_pos < html_size - 1)
                html_out[html_pos++] = *p++;
            break;
        }

        /* Find </style> (case-insensitive) */
        const char* style_end = NULL;
        {
            const char* sp = gt + 1;
            while (*sp) {
                if (sp[0] == '<' && sp[1] == '/' &&
                    (sp[2] == 's' || sp[2] == 'S') &&
                    (sp[3] == 't' || sp[3] == 'T') &&
                    (sp[4] == 'y' || sp[4] == 'Y') &&
                    (sp[5] == 'l' || sp[5] == 'L') &&
                    (sp[6] == 'e' || sp[6] == 'E') &&
                    sp[7] == '>') {
                    style_end = sp;
                    break;
                }
                sp++;
            }
        }

        if (!style_end) {
            /* No closing tag — treat rest as CSS */
            const char* csp = gt + 1;
            while (*csp && css_pos < css_size - 1)
                css_out[css_pos++] = *csp++;
            p = gt + 1 + strlen(gt + 1);
            break;
        }

        /* Extract CSS content (between > and </style>) trimmed */
        const char* css_start = gt + 1;
        while (css_start < style_end &&
               (*css_start == ' ' || *css_start == '\n' ||
                *css_start == '\r' || *css_start == '\t'))
            css_start++;
        const char* css_end = style_end;
        while (css_end > css_start &&
               (*(css_end-1) == ' ' || *(css_end-1) == '\n' ||
                *(css_end-1) == '\r' || *(css_end-1) == '\t'))
            css_end--;

        size_t css_len = (size_t)(css_end - css_start);
        if (css_pos + css_len < css_size - 1) {
            memcpy(css_out + css_pos, css_start, css_len);
            css_pos += css_len;
            css_out[css_pos] = '\0';
        }

        /* Add newline between multiple style blocks */
        if (css_pos < css_size - 2) {
            css_out[css_pos++] = '\n';
            css_out[css_pos] = '\0';
        }

        /* Skip the style block in html output */
        p = style_end + 8; /* +8 for "</style>" */
    }

    html_out[html_pos] = '\0';
    css_out[css_pos] = '\0';
}

/* ==================================================================== */
/*  Page Map — button ID → HTML file path                               */
/* ==================================================================== */

#define PAGES_DIR "doc/test_pages/"

typedef struct {
    const char* btn_id;
    const char* file_path;
} PageEntry;

static PageEntry page_map[] = {
    {"btn-page-01", PAGES_DIR "01-complex-form.html"},
    {"btn-page-02", PAGES_DIR "02-flex-layout.html"},
    {"btn-page-03", PAGES_DIR "03-selectors-elements.html"},
    {"btn-back",    PAGES_DIR "00-menu.html"},
    {"btn-back-2",  PAGES_DIR "00-menu.html"},
};

#define PAGE_MAP_COUNT ((int)(sizeof(page_map) / sizeof(page_map[0])))

/* ==================================================================== */
/*  Key & Button Callbacks                                               */
/* ==================================================================== */

/* ── Default key handler: q/Q/ESC/Ctrl+C/D → quit ── */
static bool demo_default_on_key(struct tb_event* ev, InteractCallbacks* cb) {
    if (ev->ch == 'q' || ev->ch == 'Q' || ev->key == TB_KEY_ESC ||
        ev->key == TB_KEY_CTRL_C || ev->key == TB_KEY_CTRL_D) {
        cb->quit_flag = true;
        return true;
    }
    return false;
}

/* ── Button click handler ── */
static bool demo_on_button_click(const char* btn_text, int focus_idx,
                                  LayoutNode** focus_list, int focus_count,
                                  char input_buf[][64], int input_buf_count,
                                  InteractCallbacks* cb)
{
    (void)btn_text;
    bool need_restyle = false;

    /* Get button ID from DOM */
    const char* btn_id = NULL;
    if (focus_idx >= 0 && focus_idx < focus_count &&
        focus_list[focus_idx]->styled && focus_list[focus_idx]->styled->node &&
        focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* attr = gumbo_get_attribute(
            &focus_list[focus_idx]->styled->node->v.element.attributes, "id");
        if (attr) btn_id = attr->value;
    }

    /* ── Page switching buttons ── */
    if (btn_id) {
        for (int i = 0; i < PAGE_MAP_COUNT; i++) {
            if (strcmp(btn_id, page_map[i].btn_id) == 0) {
                strncpy(cb->switch_page, page_map[i].file_path,
                        sizeof(cb->switch_page) - 1);
                cb->switch_page[sizeof(cb->switch_page) - 1] = '\0';
                cb->quit_flag = true;
                snprintf(cb->status_msg, sizeof(cb->status_msg),
                         "→ Switching page...");
                return false;
            }
        }
    }

    /* ── Form buttons (01-complex-form.html) ── */
    if (btn_id && strcmp(btn_id, "btn-save") == 0) {
        char vals[512] = "";
        int n = 0;
        for (int i = 0; i < focus_count && i < input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                char tmp[80];
                snprintf(tmp, sizeof(tmp), "%s%s", n > 0 ? ", " : "", input_buf[i]);
                strncat(vals, tmp, sizeof(vals) - strlen(vals) - 1);
                n++;
            }
        }
        snprintf(cb->status_msg, sizeof(cb->status_msg),
                 "✓ Saved: %s", n > 0 ? vals : "(no inputs)");

    } else if (btn_id && strcmp(btn_id, "btn-clear") == 0) {
        for (int i = 0; i < focus_count && i < input_buf_count; i++)
            input_buf[i][0] = '\0';
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ All inputs cleared");

    } else if (btn_id && strcmp(btn_id, "btn-reset-f") == 0) {
        const char* defaults[] = {"John", "Doe", "john@example.com",
                                   "+86 138-0000-0000", "johndoe", "********"};
        for (int i = 0; i < focus_count && i < input_buf_count && i < 6; i++)
            strncpy(input_buf[i], defaults[i], sizeof(input_buf[0]) - 1);
        snprintf(cb->status_msg, sizeof(cb->status_msg), "↺ Form reset to defaults");

    } else if (btn_id && strcmp(btn_id, "btn-test-1") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "⚡ Test 1 triggered");
    } else if (btn_id && strcmp(btn_id, "btn-test-2") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "⚡ Test 2 triggered");
    } else if (btn_id && strcmp(btn_id, "btn-test-3") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "⚠ Danger button pressed");

    /* ── Flex layout buttons (02-flex-layout.html) ── */
    } else if (btn_id && strcmp(btn_id, "btn-nav-1") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "🏠 Home");
    } else if (btn_id && strcmp(btn_id, "btn-nav-2") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "📊 Dashboard");
    } else if (btn_id && strcmp(btn_id, "btn-nav-3") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "⚙ Settings");
    } else if (btn_id && strcmp(btn_id, "btn-nav-4") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "👤 Profile");
    } else if (btn_id && strcmp(btn_id, "btn-nav-5") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "🚪 Logout");
    } else if (btn_id && strcmp(btn_id, "btn-center-act") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "🎯 Centered button clicked");
    } else if (btn_id && strcmp(btn_id, "btn-end-1") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✏ Edit action");
    } else if (btn_id && strcmp(btn_id, "btn-end-2") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "🗑 Delete action");
    } else if (btn_id && strcmp(btn_id, "btn-end-3") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "📤 Export action");
    } else if (btn_id && strcmp(btn_id, "btn-ev-1") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "One");
    } else if (btn_id && strcmp(btn_id, "btn-ev-2") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "Two");
    } else if (btn_id && strcmp(btn_id, "btn-ev-3") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "Three");
    } else if (btn_id && strcmp(btn_id, "btn-ev-4") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "Four");
    } else if (btn_id && strcmp(btn_id, "btn-vc-1") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "⬆ Up");
    } else if (btn_id && strcmp(btn_id, "btn-vc-2") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "⬇ Down");

    /* ── Selectors page buttons (03-selectors-elements.html) ── */
    } else if (btn_id && strcmp(btn_id, "btn-table-act") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "📊 Table refreshed");
    } else if (btn_id && strcmp(btn_id, "btn-list-act") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "📝 List updated");
    } else if (btn_id && strcmp(btn_id, "btn-info") == 0) {
        snprintf(cb->status_msg, sizeof(cb->status_msg),
                 "ℹ Pseudo-classes: :first-child :last-child :nth-child :hover");

    } else {
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", btn_text);
    }

    /* Refresh input text in layout after any action */
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

    return need_restyle;
}

/* ==================================================================== */
/*  Entry Point                                                          */
/* ==================================================================== */

static void demo_run(const char* filepath) {
    char current_path[512];
    strncpy(current_path, filepath, sizeof(current_path) - 1);
    current_path[sizeof(current_path) - 1] = '\0';

    /* Track whether terminal was initialized */
    bool term_inited = false;

    while (1) {
        /* ── Read HTML file ── */
        char* raw_html = read_file_content(current_path, NULL);
        if (!raw_html) {
            fprintf(stderr, "Failed to load '%s'\n", current_path);
            if (term_inited) render_shutdown();
            return;
        }

        /* ── Extract CSS and clean HTML (remove <style> blocks) ── */
        char* css = (char*)calloc(CSS_BUF_SIZE, 1);
        char* clean_html = (char*)calloc(CSS_BUF_SIZE, 1);
        extract_css_from_html(raw_html, css, CSS_BUF_SIZE, clean_html, CSS_BUF_SIZE);
        free(raw_html);

        if (strlen(clean_html) == 0) {
            strncpy(clean_html, "<html><body><p>No content</p></body></html>",
                    CSS_BUF_SIZE - 1);
        }

        /* ── Parse HTML + CSS ── */
        GumboOutput* dom = gumbo_parse(clean_html);
        KatanaOutput* css_out = NULL;
        if (strlen(css) > 0) {
            css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
        } else {
            css_out = katana_parse("", 0, KatanaParserModeStylesheet);
        }
        StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);

        /* ── Init terminal (once) ── */
        if (!term_inited) {
            if (render_init()) {
                fprintf(stderr, "render_init failed\n");
                free(css); free(clean_html);
                katana_destroy_output(css_out);
                free_style_tree(st);
                gumbo_destroy_output(&kGumboDefaultOptions, dom);
                return;
            }
            term_inited = true;
        }

        /* ── Build layout ── */
        int vw, vh;
        render_size(&vw, &vh);
        LayoutNode* lt = build_layout_tree(st, vw, vh);

        /* ── Setup callbacks ── */
        InteractCallbacks cb;
        memset(&cb, 0, sizeof(cb));
        cb.on_button_click = demo_on_button_click;
        cb.on_focus_change = NULL;
        cb.on_key = demo_default_on_key;
        cb.quit_flag = false;
        cb.status_msg[0] = '\0';
        cb.switch_page[0] = '\0';
        cb.layout_root = lt;

        /* Show page name in status bar */
        {
            const char* basename = strrchr(current_path, '/');
            basename = basename ? basename + 1 : current_path;
            snprintf(cb.status_msg, sizeof(cb.status_msg),
                     "📄 %s — Tab: focus | Enter: click | q: quit", basename);
        }

        /* ── Run interaction loop ── */
        interact_run(lt, css_out->stylesheet, st, &cb);

        /* ── Cleanup this page ── */
        free_layout_tree(lt);
        free_style_tree(st);
        katana_destroy_output(css_out);
        gumbo_destroy_output(&kGumboDefaultOptions, dom);
        free(css);
        free(clean_html);

        /* ── Check for page switch ── */
        if (cb.switch_page[0]) {
            strncpy(current_path, cb.switch_page, sizeof(current_path) - 1);
            current_path[sizeof(current_path) - 1] = '\0';
        } else {
            break;
        }
    }

    if (term_inited) render_shutdown();
    printf("Goodbye.\n");
}

#endif /* DEMO_PAGE_H */

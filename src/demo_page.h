/* ======================== demo_page.h ========================
 * Page configuration for the TUI browser demo.
 *
 * 使用方式:
 *   1. 在 doc/test_pages/ 下放 HTML 文件 (含 <style> 内联 CSS)
 *   2. 在 btn_actions[] 中注册按钮行为 (可选)
 *   3. 运行: tcc -I src -run src/demo_main.c [page.html]
 *
 * 页面自动编号规则:
 *   文件名以 NN- 开头, 菜单中自动生成 "NN. 标题" 条目
 *   按钮 id 为 btn-page-NN 自动跳转到对应页面
 *   btn-back / btn-back-N 自动回菜单
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
    if (!f) { fprintf(stderr, "Error: cannot open '%s'\n", path); return NULL; }
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
                style_start = scan; break;
            }
            scan++;
        }
        if (!style_start) {
            while (*p && html_pos < html_size - 1) html_out[html_pos++] = *p++;
            break;
        }
        while (p < style_start && html_pos < html_size - 1) html_out[html_pos++] = *p++;
        const char* gt = strchr(style_start, '>');
        if (!gt) { while (*p && html_pos < html_size - 1) html_out[html_pos++] = *p++; break; }
        const char* style_end = NULL;
        { const char* sp = gt + 1; while (*sp) {
            if (sp[0] == '<' && sp[1] == '/' &&
                (sp[2] == 's'||sp[2]=='S') && (sp[3] == 't'||sp[3]=='T') &&
                (sp[4] == 'y'||sp[4]=='Y') && (sp[5] == 'l'||sp[5]=='L') &&
                (sp[6] == 'e'||sp[6]=='E') && sp[7] == '>') { style_end = sp; break; }
            sp++; } }
        if (!style_end) {
            const char* csp = gt + 1; while (*csp && css_pos < css_size - 1) css_out[css_pos++] = *csp++;
            p = gt + 1 + strlen(gt + 1); break;
        }
        const char* css_start = gt + 1;
        while (css_start < style_end && (*css_start == ' '||*css_start=='\n'||*css_start=='\r'||*css_start=='\t')) css_start++;
        const char* css_end = style_end;
        while (css_end > css_start && (*(css_end-1)==' '||*(css_end-1)=='\n'||*(css_end-1)=='\r'||*(css_end-1)=='\t')) css_end--;
        size_t css_len = (size_t)(css_end - css_start);
        if (css_pos + css_len < css_size - 1) { memcpy(css_out + css_pos, css_start, css_len); css_pos += css_len; css_out[css_pos] = '\0'; }
        if (css_pos < css_size - 2) { css_out[css_pos++] = '\n'; css_out[css_pos] = '\0'; }
        p = style_end + 8;
    }
    html_out[html_pos] = '\0'; css_out[css_pos] = '\0';
}

/* ==================================================================== */
/*  SIMPLE PAGE MAPPING                                                  */
/*                                                                       */
/*  新增页面只需:                                                         */
/*    1. 在 doc/test_pages/ 下放 NN-xxx.html                             */
/*    2. 如果按钮 id 是 btn-page-NN, 自动跳转到 NN-*.html               */
/*    3. btn-back / btn-back-N 自动回菜单                                */
/* ==================================================================== */

#define PAGES_DIR "pages/"

static void resolve_page_path(int page_num, char* out, size_t out_size) {
    /* Try common patterns: NN-xxx.html in PAGES_DIR */
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "%s%02d-", PAGES_DIR, page_num);
    /* We can't do file listing in tcc, so use a known pattern */
    snprintf(out, out_size, "%s%02d-complex-form.html", PAGES_DIR, page_num);
}

/* ==================================================================== */
/*  BUTTON ACTION TABLE — 在这里注册按钮行为                              */
/*                                                                       */
/*  格式: { "按钮id", "状态栏显示文字" }                                   */
/*  = 开头表示从 input 收集值的特殊消息                                    */
/* ==================================================================== */

typedef struct { const char* id; const char* msg; } BtnAction;

static const BtnAction btn_actions[] = {
    /* ── Form 页面 ── */
    {"btn-clear",   "✓ All inputs cleared"},
    {"btn-reset-f",  "↺ Form reset to defaults"},
    {"btn-test-1",  "⚡ Test 1 triggered"},
    {"btn-test-2",  "⚡ Test 2 triggered"},
    {"btn-test-3",  "⚠ Danger button pressed"},

    /* ── Flex 页面 ── */
    {"btn-nav-1",   "🏠 Home"},
    {"btn-nav-2",   "📊 Dashboard"},
    {"btn-nav-3",   "⚙ Settings"},
    {"btn-nav-4",   "👤 Profile"},
    {"btn-nav-5",   "🚪 Logout"},
    {"btn-center-act", "🎯 Centered button clicked"},
    {"btn-end-1",   "✏ Edit action"},
    {"btn-end-2",   "🗑 Delete action"},
    {"btn-end-3",   "📤 Export action"},
    {"btn-ev-1",    "One"},
    {"btn-ev-2",    "Two"},
    {"btn-ev-3",    "Three"},
    {"btn-ev-4",    "Four"},
    {"btn-vc-1",    "⬆ Up"},
    {"btn-vc-2",    "⬇ Down"},
    {"btn-card-a",  "✓ Detail clicked"},
    {"btn-card-b",  "✓ Detail clicked"},
    {"btn-card-c",  "✓ Detail clicked"},

    /* ── Selectors 页面 ── */
    {"btn-table-act","📊 Table refreshed"},
    {"btn-list-act", "📝 List updated"},
    {"btn-info",     "ℹ Pseudo-classes: :first-child :last-child :nth-child :hover"},
};

#define BTN_ACTIONS_COUNT ((int)(sizeof(btn_actions) / sizeof(btn_actions[0])))

/* ==================================================================== */
/*  Key & Button Callbacks                                               */
/* ==================================================================== */

static bool demo_default_on_key(struct tb_event* ev, InteractCallbacks* cb) {
    if (ev->ch == 'q' || ev->ch == 'Q' || ev->key == TB_KEY_ESC ||
        ev->key == TB_KEY_CTRL_C || ev->key == TB_KEY_CTRL_D) {
        cb->quit_flag = true;
        return true;
    }
    return false;
}

static bool demo_on_button_click(const char* btn_text, int focus_idx,
                                  LayoutNode** focus_list, int focus_count,
                                  char input_buf[][64], int input_buf_count,
                                  InteractCallbacks* cb)
{
    (void)btn_text;
    bool need_restyle = false;

    /* Get button ID */
    const char* btn_id = NULL;
    if (focus_idx >= 0 && focus_idx < focus_count &&
        focus_list[focus_idx]->styled && focus_list[focus_idx]->styled->node &&
        focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* attr = gumbo_get_attribute(
            &focus_list[focus_idx]->styled->node->v.element.attributes, "id");
        if (attr) btn_id = attr->value;
    }

    if (!btn_id) { snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", btn_text); return false; }

    /* ── Page switching: btn-page-NN → PAGES_DIR/NN-*.html ── */
    if (strncmp(btn_id, "btn-page-", 9) == 0) {
        int page_num = atoi(btn_id + 9);
        if (page_num >= 1 && page_num <= 99) {
            /* Build filename from known pages list */
            const char* known_pages[] = {
                "01-complex-form.html",
                "02-flex-layout.html",
                "03-selectors-elements.html",
            };
            if (page_num >= 1 && page_num <= 3) {
                snprintf(cb->switch_page, sizeof(cb->switch_page),
                         PAGES_DIR "%s", known_pages[page_num - 1]);
                cb->quit_flag = true;
                snprintf(cb->status_msg, sizeof(cb->status_msg), "→ Switching page...");
                return false;
            }
        }
    }

    /* ── btn-back / btn-back-N → menu ── */
    if (btn_id && (strcmp(btn_id, "btn-back") == 0 ||
                   strncmp(btn_id, "btn-back-", 9) == 0)) {
        snprintf(cb->switch_page, sizeof(cb->switch_page),
                 PAGES_DIR "00-menu.html");
        cb->quit_flag = true;
        snprintf(cb->status_msg, sizeof(cb->status_msg), "→ Back to menu...");
        return false;
    }

    /* ── btn-save: collect input values ── */
    if (strcmp(btn_id, "btn-save") == 0) {
        char vals[512] = ""; int n = 0;
        for (int i = 0; i < focus_count && i < input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                char tmp[80]; snprintf(tmp, sizeof(tmp), "%s%s", n > 0 ? ", " : "", input_buf[i]);
                strncat(vals, tmp, sizeof(vals) - strlen(vals) - 1); n++;
            }
        }
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ Saved: %s", n > 0 ? vals : "(no inputs)");
        goto refresh_inputs;
    }

    /* ── Look up in btn_actions[] table ── */
    for (int i = 0; i < BTN_ACTIONS_COUNT; i++) {
        if (strcmp(btn_id, btn_actions[i].id) == 0) {
            snprintf(cb->status_msg, sizeof(cb->status_msg), "%s", btn_actions[i].msg);
            goto refresh_inputs;
        }
    }

    /* ── Default: show button text ── */
    snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", btn_text);

refresh_inputs:
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
    bool term_inited = false;

    while (1) {
        /* ── Read HTML file ── */
        char* raw_html = read_file_content(current_path, NULL);
        if (!raw_html) {
            fprintf(stderr, "Failed to load '%s'\n", current_path);
            if (term_inited) render_shutdown();
            return;
        }

        /* ── Extract CSS and clean HTML ── */
        char* css = (char*)calloc(CSS_BUF_SIZE, 1);
        char* clean_html = (char*)calloc(CSS_BUF_SIZE, 1);
        extract_css_from_html(raw_html, css, CSS_BUF_SIZE, clean_html, CSS_BUF_SIZE);
        free(raw_html);

        if (strlen(clean_html) == 0)
            strncpy(clean_html, "<html><body><p>No content</p></body></html>", CSS_BUF_SIZE - 1);

        /* ── Parse HTML + CSS ── */
        GumboOutput* dom = gumbo_parse(clean_html);
        KatanaOutput* css_out = NULL;
        if (strlen(css) > 0)
            css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
        else
            css_out = katana_parse("", 0, KatanaParserModeStylesheet);
        StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);

        /* ── Init terminal ── */
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
        int vw, vh; render_size(&vw, &vh);
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

        /* Status: show page name */
        const char* basename = strrchr(current_path, '/');
        basename = basename ? basename + 1 : current_path;
        snprintf(cb.status_msg, sizeof(cb.status_msg),
                 "📄 %s — Tab: focus | Enter: click | q: quit", basename);

        /* ── Run interaction ── */
        interact_run(lt, css_out->stylesheet, st, &cb);

        /* ── Cleanup ── */
        free_layout_tree(lt);
        free_style_tree(st);
        katana_destroy_output(css_out);
        gumbo_destroy_output(&kGumboDefaultOptions, dom);
        free(css); free(clean_html);

        /* ── Page switch ── */
        if (cb.switch_page[0]) {
            strncpy(current_path, cb.switch_page, sizeof(current_path) - 1);
            current_path[sizeof(current_path) - 1] = '\0';
        } else { break; }
    }

    if (term_inited) render_shutdown();
    printf("Goodbye.\n");
}

#endif /* DEMO_PAGE_H */

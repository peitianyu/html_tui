/* tools/headless_test.c — 无头渲染测试工具 v2
 * 加载 HTML → 样式树 → 布局 → 渲染, 输出两种 dump:
 *   1. 屏幕文本 dump  (screen_dump_text, ASCII 化)
 *   2. 布局树 dump    (每元素: tag/id/class + 绝对坐标 + 盒模型 + 计算样式)
 * 断言: 每个 --expect <子串> 在任一 dump 中命中即 OK。
 *
 * 编译: gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
 * 用法: headless_test <page.html> [--layout] [--expect SUBSTR ...]
 *   输出: $PREFIX/tmp/mb_headless_dump.txt (文本), mb_headless_layout.txt (布局)
 *   返回: 0=全部断言命中, 1=有未命中
 */
#define THEME_IMPLEMENTATION
#include "core/theme.h"
#define UC_IMPLEMENTATION
#include "core/uc.h"
#define STYLETREE_IMPLEMENTATION
#include "core/styletree.h"
#undef ECHO
#define LAYOUT_IMPLEMENTATION
#include "core/layout.h"
#define RENDER_IMPLEMENTATION
#include "core/render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 简化版 <style> 提取（与 mini_browser.h 中 mb_extract_css 同逻辑） */
static void extract_css(const char* raw, char* css, size_t cs, char* html, size_t hs) {
    css[0] = '\0'; html[0] = '\0';
    size_t cp = 0, hp = 0;
    int in_style = 0;
    const char* p = raw;
    while (*p && hp < hs - 1) {
        if (!in_style) {
            if (p[0]=='<' && (p[1]=='s'||p[1]=='S') && (p[2]=='t'||p[2]=='T') &&
                (p[3]=='y'||p[3]=='Y') && (p[4]=='l'||p[4]=='L') && (p[5]=='e'||p[5]=='E') &&
                (p[6]=='>'||p[6]==' '||p[6]=='\t'||p[6]=='\n'||p[6]=='\r')) {
                const char* gt = (p[6]=='>') ? p+6 : strchr(p+6, '>');
                if (!gt) break;
                p = gt + 1; in_style = 1; continue;
            }
            html[hp++] = *p++;
        } else {
            if (p[0]=='<' && p[1]=='/' && (p[2]=='s'||p[2]=='S') && (p[3]=='t'||p[3]=='T') &&
                (p[4]=='y'||p[4]=='Y') && (p[5]=='l'||p[5]=='L') && (p[6]=='e'||p[6]=='E') && p[7]=='>') {
                p += 8; in_style = 0;
                if (cp > 0 && cp < cs-2) css[cp++] = '\n';
                continue;
            }
            if (cp < cs-1) css[cp++] = *p++;
        }
    }
    html[hp] = '\0'; css[cp] = '\0';
}

/* ── 布局树 dump: tag/id/class + 几何 + 盒模型 + 计算样式 ── */
static void dump_layout(LayoutNode* n, FILE* out, int depth) {
    if (!n) return;
    for (int i = 0; i < depth; i++) fputs("  ", out);
    const char* tag = "?";
    const char* id = NULL;
    const char* cls = NULL;
    if (n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboElement* el = &n->styled->node->v.element;
        tag = gumbo_normalized_tagname(el->tag);
        GumboAttribute* a = gumbo_get_attribute(&el->attributes, "id");
        if (a) id = a->value;
        a = gumbo_get_attribute(&el->attributes, "class");
        if (a) cls = a->value;
    }
    fprintf(out, "<%s", tag);
    if (id) fprintf(out, " id=\"%s\"", id);
    if (cls) fprintf(out, " class=\"%s\"", cls);
    if (n->styled && n->styled->node && n->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* ta = gumbo_get_attribute(
            &n->styled->node->v.element.attributes, "type");
        if (ta && ta->value) fprintf(out, " type=\"%s\"", ta->value);
    }
    fprintf(out, "> abs=(%d,%d) content=%dx%d pad=%d,%d,%d,%d border=%d,%d,%d,%d margin=%d,%d,%d,%d",
            n->abs_x, n->abs_y, n->width, n->height,
            n->padding_top, n->padding_right, n->padding_bottom, n->padding_left,
            n->border_top, n->border_right, n->border_bottom, n->border_left,
            n->margin_top, n->margin_right, n->margin_bottom, n->margin_left);
    if (n->color.valid) fprintf(out, " color=%d,%d,%d", n->color.r, n->color.g, n->color.b);
    if (n->bg_color.valid) fprintf(out, " bg=%d,%d,%d", n->bg_color.r, n->bg_color.g, n->bg_color.b);
    if (n->font_bold) fputs(" bold", out);
    if (n->font_italic) fputs(" italic", out);
    if (n->font_underline) fprintf(out, " ul=%d", n->font_underline);
    if (n->text_align) fprintf(out, " align=%d", n->text_align);
    if (n->list_marker) fprintf(out, " list=%d", n->list_marker);
    if (n->z_index) fprintf(out, " z=%d", n->z_index);
    if (n->clickable) fprintf(out, " clickable action=\"%s\"", n->click_action ? n->click_action : "");
    if (n->disabled) fputs(" disabled", out);
    if (n->display == DISPLAY_FLEX) fprintf(out, " flex-dir=%d justify=%d align=%d gap=%d", n->flex_direction, n->justify_content, n->align_items, n->gap);
    if (n->display == DISPLAY_TABLE) fputs(" table", out);
    fputc('\n', out);
    for (size_t i = 0; i < n->num_children; i++)
        dump_layout(n->children[i], out, depth + 1);
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <page.html> [--layout] [--expect SUBSTR ...]\n", argv[0]); return 2; }
    bool want_layout = false;
    for (int i = 2; i < argc; i++)
        if (strcmp(argv[i], "--layout") == 0) want_layout = true;

    FILE* f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }
    fseek(f, 0, SEEK_END); long len = ftell(f); rewind(f);
    char* raw = (char*)malloc((size_t)len + 1);
    size_t n = fread(raw, 1, (size_t)len, f); fclose(f);
    raw[n] = '\0';

    size_t buf = n + 4096; if (buf < 8192) buf = 8192;
    char* css = (char*)calloc(buf, 1);
    char* html = (char*)calloc(buf, 1);
    extract_css(raw, css, buf, html, buf);

    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* ko = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, ko->stylesheet);
    LayoutNode* lt = build_layout_tree(st, 80, 60);
    Screen* s = screen_create(80, 60);
    screen_render_tree(s, lt);

    const char* dump_path = "/data/data/com.termux/files/usr/tmp/mb_headless_dump.txt";
    const char* layout_path = "/data/data/com.termux/files/usr/tmp/mb_headless_layout.txt";
    FILE* out = fopen(dump_path, "w");
    screen_dump_text(s, out);
    fclose(out);
    if (want_layout) {
        out = fopen(layout_path, "w");
        dump_layout(lt, out, 0);
        fclose(out);
    }

    /* 读取两份 dump */
    char text_dump[200000] = {0};
    char lay_dump[400000] = {0};
    out = fopen(dump_path, "r");
    size_t dn = fread(text_dump, 1, sizeof(text_dump)-1, out); fclose(out);
    text_dump[dn] = '\0';
    if (want_layout) {
        out = fopen(layout_path, "r");
        size_t ln = fread(lay_dump, 1, sizeof(lay_dump)-1, out); fclose(out);
        lay_dump[ln] = '\0';
    }

    /* 断言: 任一 dump 命中即 OK (支持 --expect=SUBSTR 或 --expect SUBSTR) */
    int fail = 0;
    for (int i = 2; i < argc; i++) {
        const char* exp = NULL;
        if (strncmp(argv[i], "--expect=", 9) == 0) exp = argv[i] + 9;
        else if (strcmp(argv[i], "--expect") == 0 && i + 1 < argc) exp = argv[++i];
        if (!exp) continue;
        bool hit = (strstr(text_dump, exp) != NULL) ||
                   (want_layout && strstr(lay_dump, exp) != NULL);
        if (hit) printf("OK   : %s\n", exp);
        else { printf("FAIL : %s\n", exp); fail = 1; }
    }

    free_layout_tree(lt);
    free_style_tree(st);
    katana_destroy_output(ko);
    gumbo_destroy_output(&kGumboDefaultOptions, dom);
    free(css); free(html); free(raw); screen_free(s);
    return fail;
}

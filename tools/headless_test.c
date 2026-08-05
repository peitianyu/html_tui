/* tools/headless_test.c
 * 无头渲染测试工具：加载 HTML → 样式树 → 布局 → 渲染 → dump 文本 + 断言。
 * 编译: gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
 * 用法: headless_test <page.html> [expect-substring ...]
 *   输出 dump 到 $PREFIX/tmp/mb_headless_dump.txt，返回 0=全部断言命中, 1=有断言未命中
 */
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

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s <page.html> [expect...]\n", argv[0]); return 2; }
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
    FILE* out = fopen(dump_path, "w");
    if (!out) { fprintf(stderr, "cannot write dump\n"); return 2; }
    screen_dump_text(s, out);
    fclose(out);

    /* 断言 */
    char dump[200000] = {0};
    out = fopen(dump_path, "r");
    size_t dn = fread(dump, 1, sizeof(dump)-1, out); fclose(out);
    dump[dn] = '\0';

    int fail = 0;
    for (int i = 2; i < argc; i++) {
        if (strstr(dump, argv[i])) printf("OK   : %s\n", argv[i]);
        else { printf("FAIL : %s (not found in dump)\n", argv[i]); fail = 1; }
    }
    free_layout_tree(lt);
    free_style_tree(st);
    katana_destroy_output(ko);
    gumbo_destroy_output(&kGumboDefaultOptions, dom);
    free(css); free(html); free(raw); screen_free(s);
    return fail;
}

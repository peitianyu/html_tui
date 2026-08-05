/* tools/headless_test.c — 无头渲染测试工具 v3
 * 加载 HTML → 样式树 → 布局 → 渲染, 输出两种 dump:
 *   1. 屏幕文本 dump  (screen_dump_text, ASCII 化)
 *   2. 布局树 dump    (每元素: tag/id/class + 绝对坐标 + 盒模型 + 计算样式)
 * 断言语法:
 *   --expect=SUBSTR        任一 dump 中包含 (跨行)
 *   --expect-line=A|B|C    同一行同时包含 A、B、C (精确行级)
 *   --expect-count=S=N     SUBSTR 出现次数恰好为 N (含跨行)
 *   --viewport=WxH         自定义视口 (默认 80x60, 测 @media)
 *
 * 编译: gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
 * 用法: headless_test <page.html> [--layout] [--viewport=WxH] [断言...]
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

/* 从 <tag ...> 内提取属性值 (与 mini_browser.h mb_attr_get 同逻辑) */
static char* attr_get(const char* tag, size_t tag_len, const char* attr) {
    size_t alen = strlen(attr);
    for (size_t i = 0; i + alen < tag_len; i++) {
        char prev = (i == 0) ? '<' : tag[i - 1];
        if (prev != ' ' && prev != '\t' && prev != '\n' && prev != '\r' &&
            prev != '<' && prev != '>') continue;
        if (strncasecmp(tag + i, attr, alen) != 0) continue;
        char next = tag[i + alen];
        if (next != ' ' && next != '\t' && next != '=' && next != '>') continue;
        size_t j = i + alen;
        while (j < tag_len && (tag[j] == ' ' || tag[j] == '\t')) j++;
        if (j >= tag_len || tag[j] != '=') continue;
        j++;
        while (j < tag_len && (tag[j] == ' ' || tag[j] == '\t')) j++;
        if (j >= tag_len) continue;
        char q = (tag[j] == '\'' || tag[j] == '"') ? tag[j] : 0;
        if (q) j++;
        size_t start = j;
        while (j < tag_len && (q ? tag[j] != q : (tag[j] != ' ' && tag[j] != '>' && tag[j] != '\t'))) j++;
        size_t vlen = j - start;
        if (vlen == 0) continue;
        char* val = (char*)malloc(vlen + 1);
        memcpy(val, tag + start, vlen);
        val[vlen] = '\0';
        return val;
    }
    return NULL;
}

/* 简化版 <style>/<link> 提取（与 mini_browser.h 中 mb_extract_css 同逻辑） */
static void extract_css(const char* raw, char* css, size_t cs, char* html, size_t hs,
                        const char* base_dir) {
    css[0] = '\0'; html[0] = '\0';
    size_t cp = 0, hp = 0;
    int in_style = 0;
    const char* p = raw;
    while (*p && hp < hs - 1) {
        if (!in_style) {
            if (p[0]=='<' && (p[1]=='l'||p[1]=='L') && (p[2]=='i'||p[2]=='I') &&
                (p[3]=='n'||p[3]=='N') && (p[4]=='k'||p[4]=='K') &&
                (p[5]=='>'||p[5]==' '||p[5]=='\t'||p[5]=='\n'||p[5]=='\r')) {
                const char* gt = strchr(p + 5, '>');
                if (gt) {
                    size_t tag_len = (size_t)(gt - p) + 1;
                    char* rel = attr_get(p, tag_len, "rel");
                    char* href = attr_get(p, tag_len, "href");
                    if (rel && strcasestr(rel, "stylesheet") && href && href[0]) {
                        char path[1024];
                        if (base_dir && base_dir[0])
                            snprintf(path, sizeof(path), "%s/%s", base_dir, href);
                        else
                            snprintf(path, sizeof(path), "%s", href);
                        FILE* lf = fopen(path, "rb");
                        if (lf) {
                            fseek(lf, 0, SEEK_END); long llen = ftell(lf); rewind(lf);
                            if (llen > 0) {
                                char* lbuf = (char*)malloc((size_t)llen + 1);
                                size_t lr = fread(lbuf, 1, (size_t)llen, lf);
                                lbuf[lr] = '\0';
                                if (cp + lr + 2 < cs) {
                                    memcpy(css + cp, lbuf, lr);
                                    cp += lr;
                                    css[cp++] = '\n';
                                }
                                free(lbuf);
                            }
                            fclose(lf);
                        }
                    }
                    free(rel); free(href);
                    p = gt + 1;  /* 跳过整个标签 */
                    continue;
                }
            }
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
    if (argc < 2) { fprintf(stderr, "usage: %s <page.html> [--layout] [--viewport=WxH] [断言...]\n", argv[0]); return 2; }
    bool want_layout = false;
    int vw = 80, vh = 60;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--layout") == 0) want_layout = true;
        else if (strncmp(argv[i], "--viewport=", 11) == 0) {
            vw = atoi(argv[i] + 11);
            const char* c = strchr(argv[i] + 11, 'x');
            vh = c ? atoi(c + 1) : vh;
        }
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }
    fseek(f, 0, SEEK_END); long len = ftell(f); rewind(f);
    char* raw = (char*)malloc((size_t)len + 1);
    size_t n = fread(raw, 1, (size_t)len, f); fclose(f);
    raw[n] = '\0';

    size_t buf = n + 4096; if (buf < 8192) buf = 8192;
    char* css = (char*)calloc(buf, 1);
    char* html = (char*)calloc(buf, 1);
    char base_dir[512];
    {
        const char* slash = strrchr(argv[1], '/');
        if (slash && slash != argv[1]) {
            size_t bn = (size_t)(slash - argv[1]);
            if (bn >= sizeof(base_dir)) bn = sizeof(base_dir) - 1;
            memcpy(base_dir, argv[1], bn);
            base_dir[bn] = '\0';
        } else base_dir[0] = '\0';
    }
    extract_css(raw, css, buf, html, buf, base_dir);

    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* ko = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, ko->stylesheet, vw);
    LayoutNode* lt = build_layout_tree(st, vw, vh);
    Screen* s = screen_create(vw, vh);
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

    /* 断言: 支持 --expect=SUBSTR / --expect-line=A|B|C / --expect-count=S=N */
    /* flush diff 统计: --flush-stats 输出 首帧/静止帧/单cell变化帧 的字节数到 dump */
    FILE* fs = fopen("/data/data/com.termux/files/usr/tmp/mb_flush_stats.txt", "w");
    if (fs) {
        char fbuf[131072];
        /* 首帧 (prev 全空) */
        size_t n1 = screen_flush_to_buffer(s, fbuf, sizeof(fbuf));
        /* 静止帧: 再次 flush (无变化) */
        size_t n2 = screen_flush_to_buffer(s, fbuf, sizeof(fbuf));
        /* 单 cell 变化帧 */
        screen_scr_set(s, 3, 2, 'X');
        size_t n3 = screen_flush_to_buffer(s, fbuf, sizeof(fbuf));
        fprintf(fs, "first=%zu idle=%zu onecell=%zu\n", n1, n2, n3);
        fclose(fs);
    }

    int fail = 0;
    for (int i = 2; i < argc; i++) {
        const char* arg = argv[i];
        if (strncmp(arg, "--expect=", 9) == 0) {
            const char* exp = arg + 9;
            bool hit = (strstr(text_dump, exp) != NULL) ||
                       (want_layout && strstr(lay_dump, exp) != NULL);
            if (hit) printf("OK   : %s\n", exp);
            else { printf("FAIL : %s\n", exp); fail = 1; }
        } else if (strncmp(arg, "--expect-line=", 14) == 0) {
            /* 同一行必须同时包含所有 | 分隔子串 (不破坏输入缓冲) */
            char buf[1024]; snprintf(buf, sizeof(buf), "%s", arg + 14);
            char* save = NULL;
            char* toks[8]; int nt = 0;
            for (char* tk = strtok_r(buf, "|", &save); tk && nt < 8; tk = strtok_r(NULL, "|", &save))
                toks[nt++] = tk;
            /* 按行扫描 helper (内联): 返回是否某行含全部子串 */
            bool found = false;
            const char* row = text_dump;
            while (*row && !found) {
                const char* nl = strchr(row, '\n');
                size_t rlen = nl ? (size_t)(nl - row) : strlen(row);
                if (rlen < sizeof(buf)) {
                    char line[1024];
                    memcpy(line, row, rlen); line[rlen] = '\0';
                    bool all = true;
                    for (int t = 0; t < nt; t++)
                        if (!strstr(line, toks[t])) { all = false; break; }
                    if (all) found = true;
                }
                if (!nl) break;
                row = nl + 1;
            }
            if (want_layout && !found) {
                row = lay_dump;
                while (*row && !found) {
                    const char* nl = strchr(row, '\n');
                    size_t rlen = nl ? (size_t)(nl - row) : strlen(row);
                    if (rlen < sizeof(buf)) {
                        char line[1024];
                        memcpy(line, row, rlen); line[rlen] = '\0';
                        bool all = true;
                        for (int t = 0; t < nt; t++)
                            if (!strstr(line, toks[t])) { all = false; break; }
                        if (all) found = true;
                    }
                    if (!nl) break;
                    row = nl + 1;
                }
            }
            if (found) printf("OK   : line[%s]\n", arg + 14);
            else { printf("FAIL : line[%s]\n", arg + 14); fail = 1; }
        } else if (strcmp(arg, "--flush-ok") == 0) {
            /* flush diff 有效: 静止帧 0 字节, 单 cell 帧 < 首帧 */
            FILE* fsf = fopen("/data/data/com.termux/files/usr/tmp/mb_flush_stats.txt", "r");
            if (!fsf) { printf("FAIL : --flush-ok (no stats)\n"); fail = 1; continue; }
            char line[128];
            if (!fgets(line, sizeof(line), fsf)) { fclose(fsf); printf("FAIL : --flush-ok (no stats)\n"); fail = 1; continue; }
            fclose(fsf);
            size_t n1 = 0, n2 = 0, n3 = 0;
            if (sscanf(line, "first=%zu idle=%zu onecell=%zu", &n1, &n2, &n3) != 3) {
                printf("FAIL : --flush-ok (bad stats '%s')\n", line); fail = 1; continue;
            }
            bool ok = (n2 == 0) && (n3 > 0 && n3 < n1);
            if (ok) printf("OK   : flush diff (first=%zuB idle=%zuB onecell=%zuB)\n", n1, n2, n3);
            else { printf("FAIL : flush diff (first=%zuB idle=%zuB onecell=%zuB, want idle==0 && 0<onecell<first)\n", n1, n2, n3); fail = 1; }
        } else if (strncmp(arg, "--expect-count=", 15) == 0) {
            /* --expect-count=N:SUBSTR: 出现次数恰好为 N (两 dump 合计) */
            const char* rest = arg + 15;
            char num[16]; int ni = 0;
            while (rest[ni] && rest[ni] != ':' && ni < 15) { num[ni] = rest[ni]; ni++; }
            num[ni] = '\0';
            if (rest[ni] != ':') { printf("FAIL : bad --expect-count (need N:SUBSTR)\n"); fail = 1; continue; }
            const char* sub = rest + ni + 1;
            int want = atoi(num);
            int cnt = 0;
            const char* p = text_dump;
            while ((p = strstr(p, sub))) { cnt++; p += strlen(sub); }
            if (want_layout) {
                p = lay_dump;
                while ((p = strstr(p, sub))) { cnt++; p += strlen(sub); }
            }
            if (cnt == want) printf("OK   : count[%s]=%d\n", sub, cnt);
            else { printf("FAIL : count[%s] got %d want %d\n", sub, cnt, want); fail = 1; }
        }
    }

    free_layout_tree(lt);
    free_style_tree(st);
    katana_destroy_output(ko);
    gumbo_destroy_output(&kGumboDefaultOptions, dom);
    free(css); free(html); free(raw); screen_free(s);
    return fail;
}

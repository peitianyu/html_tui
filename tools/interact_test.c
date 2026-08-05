/* tools/interact_test.c — 无头交互测试
 * 通过事件注入队列驱动 interact_run:
 *   - Tab/Shift+Tab 焦点遍历, Enter 触发
 *   - 鼠标点击 (坐标) / 输入框编辑
 * 断言: 回调日志 (累积) + 最终状态栏消息。
 *
 * 编译: gcc -DRENDER_HEADLESS -I src -o /tmp/interact_test tools/interact_test.c -lm
 * 用法: interact_test <page.html> --events 'tab tab enter click:5,10' \
 *                        [--expect=SUBSTR ...]
 *   EVENTS 空格分隔: tab btab enter esc key:X keycode:N
 *                    click:X,Y release motion:X,Y
 *   --expect=SUBSTR  在回调日志或最终 status_msg 中搜索
 *   --log            打印回调日志 (调试)
 */
#define MINI_BROWSER_IMPLEMENTATION
#include "core/mini_browser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char g_log[16384];
static int g_log_len = 0;

static void tlogf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(g_log + g_log_len, sizeof(g_log) - g_log_len, fmt, ap);
    va_end(ap);
    if (n > 0) g_log_len += n;
    if (g_log_len > (int)sizeof(g_log) - 8) g_log_len = sizeof(g_log) - 8;
}

static bool test_on_button_click(const char* btn_id, const char* btn_text,
                                 MiniBrowser* mb, void* userdata)
{
    (void)userdata;
    tlogf("btn[id=%s text=%s] ", btn_id ? btn_id : "-", btn_text ? btn_text : "-");
    /* 模拟 demo.c 的 btn-save: 收集输入值设置状态消息 (验证不被默认覆盖) */
    if (btn_id && strcmp(btn_id, "btn-save") == 0) {
        const char* v = mb_get_text(mb, "inp-first");
        char tmp[256];
        snprintf(tmp, sizeof(tmp), "saved: %s", v ? v : "");
        mb_set_status(mb, tmp);
    }
    return false;
}

static bool test_on_element_click(const char* action, const char* element_id,
                                  const char* tag, MiniBrowser* mb, void* userdata)
{
    (void)userdata;
    tlogf("el[action=%s id=%s tag=%s] ", action ? action : "-",
         element_id ? element_id : "-", tag ? tag : "-");
    /* <a> 链接演示跳页 */
    if (tag && strcmp(tag, "a") == 0 && action && strstr(action, ".html"))
        mb_switch_page(mb, action);
    return false;
}

/* 遍历布局树找第一个 class 匹配的元素, 返回其背景色 */
static bool find_bg_by_class(MiniBrowser* mb, const char* cls, int* r, int* g, int* b)
{
    LayoutNode* root = mb->cb.layout_root;
    if (!root) return false;
    LayoutNode* stack[4096]; int sp = 0;
    stack[sp++] = root;
    while (sp > 0) {
        LayoutNode* n = stack[--sp];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT && n->bg_color.valid) {
            GumboAttribute* ca = gumbo_get_attribute(
                &n->styled->node->v.element.attributes, "class");
            if (ca && ca->value && strstr(ca->value, cls)) {
                *r = n->bg_color.r; *g = n->bg_color.g; *b = n->bg_color.b;
                return true;
            }
        }
        for (size_t ci = n->num_children; ci > 0 && sp < 4096; ci--)
            stack[sp++] = n->children[ci - 1];
    }
    return false;
}

/* 遍历布局树找第一个 tag+id 匹配的元素, 返回其前景色 */
static bool find_color(MiniBrowser* mb, const char* tag, const char* href,
                       int* r, int* g, int* b)
{
    LayoutNode* root = mb->cb.layout_root;
    if (!root) return false;
    LayoutNode* stack[4096]; int sp = 0;
    stack[sp++] = root;
    while (sp > 0) {
        LayoutNode* n = stack[--sp];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT) {
            GumboElement* el = &n->styled->node->v.element;
            if (el->tag == GUMBO_TAG_A && n->color.valid) {
                if (!href) { *r = n->color.r; *g = n->color.g; *b = n->color.b; return true; }
                GumboAttribute* h = gumbo_get_attribute(&el->attributes, "href");
                if (h && h->value && strcmp(h->value, href) == 0) {
                    *r = n->color.r; *g = n->color.g; *b = n->color.b; return true;
                }
            }
        }
        for (size_t ci = n->num_children; ci > 0 && sp < 4096; ci--)
            stack[sp++] = n->children[ci - 1];
    }
    return false;
}

static void push_event(const char* tok) {
    if (strcmp(tok, "tab") == 0)          headless_push_key(0, TB_KEY_TAB);
    else if (strcmp(tok, "btab") == 0)    headless_push_key(0, TB_KEY_BACK_TAB);
    else if (strcmp(tok, "enter") == 0)   headless_push_key(0, TB_KEY_ENTER);
    else if (strcmp(tok, "esc") == 0)     headless_push_key(0, TB_KEY_ESC);
    else if (strcmp(tok, "release") == 0) headless_push_mouse(TB_KEY_MOUSE_RELEASE, 0, 0, 0);
    else if (strncmp(tok, "tabx:", 5) == 0) {
        int n = atoi(tok + 5);
        if (n > 0 && n <= 4096) for (int i = 0; i < n; i++) headless_push_key(0, TB_KEY_TAB);
    }
    else if (strncmp(tok, "key:", 4) == 0)     headless_push_key((unsigned char)tok[4], 0);
    else if (strncmp(tok, "keycode:", 8) == 0) headless_push_key(0, atoi(tok + 8));
    else if (strncmp(tok, "click:", 6) == 0) {
        int x = atoi(tok + 6);
        const char* c = strchr(tok + 6, ',');
        headless_push_mouse(TB_KEY_MOUSE_LEFT, x, c ? atoi(c + 1) : 0, 0);
    } else if (strncmp(tok, "motion:", 7) == 0) {
        int x = atoi(tok + 7);
        const char* c = strchr(tok + 7, ',');
        headless_push_mouse(TB_KEY_MOUSE_LEFT, x, c ? atoi(c + 1) : 0, TB_MOD_MOTION);
    } else {
        fprintf(stderr, "WARN: unknown event '%s'\n", tok);
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <page.html> --events '...' [--expect=SUBSTR ...] [--log]\n", argv[0]);
        return 2;
    }
    const char* page = argv[1];
    const char* events = NULL;
    bool show_log = false;

    MB_Config cfg = MB_CONFIG_DEFAULT;
    cfg.on_button_click = test_on_button_click;
    cfg.on_element_click = test_on_element_click;
    cfg.show_statusbar = true;
    cfg.show_scrollbars = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--events") == 0 && i + 1 < argc) events = argv[++i];
        else if (strcmp(argv[i], "--log") == 0) show_log = true;
    }

    MiniBrowser* mb = mini_browser_open(page, &cfg);
    if (!mb) { fprintf(stderr, "open failed: %s\n", page); return 2; }

    /* 入队所有事件 + 末尾 esc 兜底退出 */
    headless_clear_events();
    if (events) {
        char buf[2048];
        snprintf(buf, sizeof(buf), "%s", events);
        char* save = NULL;
        for (char* tk = strtok_r(buf, " ", &save); tk; tk = strtok_r(NULL, " ", &save))
            push_event(tk);
    }
    headless_push_key(0, TB_KEY_ESC);

    mini_browser_run_loop(mb);

    if (show_log) printf("LOG: %s\n", g_log);

    /* 断言: 支持 --expect=SUBSTR 或 --expect SUBSTR; 命中回调日志或最终 status_msg */
    int fail = 0;
    for (int i = 2; i < argc; i++) {
        const char* exp = NULL;
        if (strncmp(argv[i], "--expect=", 9) == 0) exp = argv[i] + 9;
        else if (strcmp(argv[i], "--expect") == 0 && i + 1 < argc) exp = argv[++i];
        if (!exp) continue;
        bool hit = (strstr(g_log, exp) != NULL) ||
                   (strstr(mb->cb.status_msg, exp) != NULL);
        if (hit) printf("OK   : %s\n", exp);
        else { printf("FAIL : %s (log=\"%s\" status=\"%s\")\n", exp, g_log, mb->cb.status_msg); fail = 1; }
    }
    /* 元素文本断言: --expect-text=id|SUBSTR (查 mb_get_text) */
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--expect-text=", 14) != 0) continue;
        char buf[256]; snprintf(buf, sizeof(buf), "%s", argv[i] + 14);
        char* sep = strchr(buf, '|');
        if (!sep) { printf("FAIL : bad --expect-text (need id|SUBSTR)\n"); fail = 1; continue; }
        *sep = '\0';
        const char* got = mb_get_text(mb, buf);
        if (got && strstr(got, sep + 1)) printf("OK   : text %s contains %s\n", buf, sep + 1);
        else { printf("FAIL : text %s (got \"%s\", want contains \"%s\")\n", buf, got ? got : "", sep + 1); fail = 1; }
    }
    /* 元素背景色断言: --expect-bg=class|r,g,b (hover 快速路径验证) */
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--expect-bg=", 12) != 0) continue;
        char buf[256]; snprintf(buf, sizeof(buf), "%s", argv[i] + 12);
        char* sep = strchr(buf, '|');
        if (!sep) { printf("FAIL : bad --expect-bg (need class|r,g,b)\n"); fail = 1; continue; }
        *sep = '\0';
        int r = 0, g = 0, b = 0;
        if (sscanf(sep + 1, "%d,%d,%d", &r, &g, &b) != 3) { printf("FAIL : bad color\n"); fail = 1; continue; }
        int fr = 0, fg = 0, fb = 0;
        if (find_bg_by_class(mb, buf, &fr, &fg, &fb) && fr == r && fg == g && fb == b)
            printf("OK   : bg %s -> %d,%d,%d\n", buf, fr, fg, fb);
        else {
            printf("FAIL : bg %s (got %d,%d,%d, want %d,%d,%d)\n", buf, fr, fg, fb, r, g, b);
            fail = 1;
        }
    }
    /* 链接颜色断言: --expect-color=href|r,g,b (href 空 = 任意链接) */
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--expect-color=", 15) != 0) continue;
        char buf[256]; snprintf(buf, sizeof(buf), "%s", argv[i] + 15);
        char* sep = strchr(buf, '|');
        if (!sep) { printf("FAIL : bad --expect-color (need href|r,g,b)\n"); fail = 1; continue; }
        *sep = '\0';
        const char* href = buf[0] ? buf : NULL;
        int r = 0, g = 0, b = 0;
        if (sscanf(sep + 1, "%d,%d,%d", &r, &g, &b) != 3) { printf("FAIL : bad color\n"); fail = 1; continue; }
        int fr = 0, fg = 0, fb = 0;
        if (find_color(mb, "a", href, &fr, &fg, &fb) && fr == r && fg == g && fb == b)
            printf("OK   : color %s -> %d,%d,%d\n", href ? href : "*", fr, fg, fb);
        else {
            printf("FAIL : color %s (got %d,%d,%d, want %d,%d,%d)\n",
                   href ? href : "*", fr, fg, fb, r, g, b);
            fail = 1;
        }
    }

    mini_browser_close(mb);
    return fail;
}

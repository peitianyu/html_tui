#ifndef MINI_BROWSER_H
#define MINI_BROWSER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 不透明句柄 — 内部持有所有状态 */
typedef struct MiniBrowser MiniBrowser;

/**
 * 按钮点击回调。
 *
 * @param btn_id   按钮的 HTML id 属性 (如 `btn-save`), 无 id 时为 NULL
 * @param btn_text 按钮的显示文本
 * @param mb       MiniBrowser 句柄, 用于调用 mb_set_* 等运行时 API
 * @param userdata 用户在 MB_Config 中传入的指针
 * @return true 表示需要重建布局树 (显示/隐藏元素后)
 */
typedef bool (*MB_ButtonClickFn)(const char* btn_id, const char* btn_text,
                                 MiniBrowser* mb, void* userdata);

/**
 * 全局按键回调 (在默认处理之前调用)。
 *
 * @param ch       输入的 Unicode 字符 (0 = 非字符键)
 * @param key      特殊键码 (TB_KEY_* 系列), 常用值:
 *                 TB_KEY_ESC=27  TB_KEY_TAB=9
 *                 TB_KEY_ENTER=13  TB_KEY_BACKSPACE=8
 * @param mb       MiniBrowser 句柄
 * @param userdata 用户数据指针
 * @return true 表示按键已处理, 不再执行默认行为
 */
typedef bool (*MB_KeyFn)(uint32_t ch, int key,
                         MiniBrowser* mb, void* userdata);

/* ═══════════════════════════════════════════════════════════════════ */
/*  配置                                                              */
/* ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    MB_ButtonClickFn on_button_click;   /* 按钮点击 (NULL=默认) */
    MB_KeyFn         on_key;            /* 全局按键 (NULL=默认) */
    void*            userdata;          /* 透传给回调 */
    bool             show_scrollbars;   /* 显示滚动条 (默认 true) */
} MB_Config;

#define MB_CONFIG_DEFAULT \
    { .on_button_click = NULL, .on_key = NULL, \
      .userdata = NULL, .show_scrollbars = true }

/* ═══════════════════════════════════════════════════════════════════ */
/*  核心 API                                                         */
/* ═══════════════════════════════════════════════════════════════════ */

/**
 * [一键模式] 加载 HTML 并启动 TUI 交互循环。
 *
 * 内部完成: 文件读取 → HTML/CSS 解析 → 样式树 → 布局 → 渲染 → 交互
 * 按 q / Esc 退出。内置 btn-page-NN / btn-back 页面跳转。
 *
 * @param html_path HTML 文件路径 (如 "pages/00-menu.html")
 */
void mini_browser_run(const char* html_path);

/**
 * [高级模式] 打开页面, 返回句柄。
 *
 * 之后调用 mini_browser_run_loop() 运行, 最后 mini_browser_close() 清理。
 * 可在回调中调用 mb_set_status/mb_set_text 等运行时 API。
 *
 * @param html_path HTML 文件路径
 * @param config    配置 (NULL = 使用默认)
 * @return MiniBrowser 句柄, NULL 表示失败
 */
MiniBrowser* mini_browser_open(const char* html_path, MB_Config* config);

/**
 * 运行交互循环 (阻塞, 直到退出或页面切换)。
 * 支持自动页面切换 (在循环内重新加载新页面)。
 */
void mini_browser_run_loop(MiniBrowser* mb);

/**
 * 关闭浏览器, 释放所有资源。
 * 包括恢复终端状态。
 */
void mini_browser_close(MiniBrowser* mb);

/* ═══════════════════════════════════════════════════════════════════ */
/*  运行时 API (在回调中安全调用)                                      */
/* ═══════════════════════════════════════════════════════════════════ */

/** 设置状态栏消息 */
void mb_set_status(MiniBrowser* mb, const char* msg);

/**
 * 按 HTML id 设置元素文本。
 * 立即生效, 且跨布局重建保留。
 */
void mb_set_text(MiniBrowser* mb, const char* element_id, const char* text);

/** 按 HTML id 获取元素文本 */
const char* mb_get_text(MiniBrowser* mb, const char* element_id);

/**
 * 显示/隐藏元素。
 * 注意: 需要回调返回 true 来触发布局重建, 否则无效果。
 */
void mb_set_visible(MiniBrowser* mb, const char* element_id, bool visible);

/** 切换页面 (当前循环结束后加载) */
void mb_switch_page(MiniBrowser* mb, const char* page_path);

/** 退出交互循环 */
void mb_quit(MiniBrowser* mb);

#ifdef __cplusplus
}
#endif

/* ═══════════════════════════════════════════════════════════════════ */
/*  实现 — 仅当定义 MINI_BROWSER_IMPLEMENTATION 时编译                   */
/* ═══════════════════════════════════════════════════════════════════ */
#ifdef MINI_BROWSER_IMPLEMENTATION

/* ── 内部头文件 (单头文件库模式, 仅在此处展开实现) ── */

#define UC_IMPLEMENTATION
#include "uc.h"

#define STYLETREE_IMPLEMENTATION
#include "styletree.h"

/* katana.h (flex) 定义了 ECHO 宏, termbox2.h (via render.h) 的
   <termios.h> 也定义了 ECHO — 必须先 undef 避免重定义警告 */
#undef ECHO

#define LAYOUT_IMPLEMENTATION
#include "layout.h"

#define RENDER_IMPLEMENTATION
#include "render.h"

#define INTERACT_IMPLEMENTATION
#include "interact.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ═══════════════════════════════════════════════════════════════════ */
/*  MiniBrowser 结构体                                                */
/* ═══════════════════════════════════════════════════════════════════ */

struct MiniBrowser {
    MB_Config config;

    char current_path[512];
    bool term_inited;

    /* 当前页面的解析结果 (用于页面切换时清理) */
    char*      raw_html;
    char*      css_buf;
    char*      clean_html;
    GumboOutput*   dom;
    KatanaOutput*  css_output;
    StyledNode*    styled_root;
    LayoutNode*    layout_root;

    /* InteractCallbacks — 内部持有, 用于 mb_set_* */
    InteractCallbacks cb;
};

/* ═══════════════════════════════════════════════════════════════════ */
/*  内部辅助函数                                                      */
/* ═══════════════════════════════════════════════════════════════════ */

static char* mb_read_file(const char* path, size_t* out_len) {
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

typedef enum { MB_EXTRACT_TEXT, MB_EXTRACT_STYLE } MB_ExtractState;

static void mb_extract_css(const char* raw, char* css, size_t cs,
                           char* html, size_t hs) {
    css[0] = '\0'; html[0] = '\0';
    size_t cp = 0, hp = 0;
    MB_ExtractState st = MB_EXTRACT_TEXT;
    const char* p = raw;
    while (*p) {
        if (st == MB_EXTRACT_TEXT) {
            if (p[0] == '<') {
                if ((p[1] == '/' || p[1] == '/') &&
                    (p[2] == 's'||p[2]=='S') && (p[3] == 't'||p[3]=='T') &&
                    (p[4] == 'y'||p[4]=='Y') && (p[5] == 'l'||p[5]=='L') &&
                    (p[6] == 'e'||p[6]=='E') && p[7] == '>') {
                    p += 8; continue;
                }
                if (((p[1] == 's'||p[1]=='S') && (p[2] == 't'||p[2]=='T') &&
                     (p[3] == 'y'||p[3]=='Y') && (p[4] == 'l'||p[4]=='L') &&
                     (p[5] == 'e'||p[5]=='E')) &&
                    (p[6] == '>' || p[6] == ' ' || p[6] == '\t' ||
                     p[6] == '\n' || p[6] == '\r')) {
                    const char* gt = (p[6] == '>') ? p + 6 : strchr(p + 6, '>');
                    if (gt) { p = gt + 1; st = MB_EXTRACT_STYLE; continue; }
                }
            }
            if (hp < hs - 1) html[hp++] = *p++;
            else break;
        } else {
            if (p[0] == '<' && p[1] == '/' &&
                (p[2] == 's'||p[2]=='S') && (p[3] == 't'||p[3]=='T') &&
                (p[4] == 'y'||p[4]=='Y') && (p[5] == 'l'||p[5]=='L') &&
                (p[6] == 'e'||p[6]=='E') && p[7] == '>') {
                p += 8; st = MB_EXTRACT_TEXT;
                if (cp > 0 && cp < cs - 2) css[cp++] = '\n';
                continue;
            }
            if (cp < cs - 1) css[cp++] = *p++;
            else break;
        }
    }
    html[hp] = '\0'; css[cp] = '\0';
}

/* ═══════════════════════════════════════════════════════════════════ */
/*  默认回调 — 内置 btn-page-NN / btn-back 处理                        */
/* ═══════════════════════════════════════════════════════════════════ */

static bool mb_default_on_click(const char* btn_text, int focus_idx,
    LayoutNode** focus_list, int focus_count,
    char input_buf[][4096], int input_buf_count,
    InteractCallbacks* cb)
{
    (void)btn_text;
    MiniBrowser* mb = (MiniBrowser*)cb->user_data;
    if (!mb) return false;

    /* 获取按钮 id */
    const char* btn_id = NULL;
    if (focus_idx >= 0 && focus_idx < focus_count &&
        focus_list[focus_idx]->styled && focus_list[focus_idx]->styled->node &&
        focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* attr = gumbo_get_attribute(
            &focus_list[focus_idx]->styled->node->v.element.attributes, "id");
        if (attr) btn_id = attr->value;
    }

    /* 用户回调优先 */
    if (mb->config.on_button_click) {
        bool ret = mb->config.on_button_click(btn_id, btn_text, mb,
                                              mb->config.userdata);
        if (ret) return true;
    }

    /* btn-page-NN → 页面跳转 */
    if (btn_id && strncmp(btn_id, "btn-page-", 9) == 0) {
        int page_num = atoi(btn_id + 9);
        if (page_num >= 1 && page_num <= 99) {
            const char* known[] = {
                "01-complex-form.html", "02-flex-layout.html",
                "03-selectors-elements.html", "04-new-features.html",
                "05-counter.html",
            };
            if (page_num >= 1 && page_num <= 5) {
                snprintf(cb->switch_page, sizeof(cb->switch_page),
                         "pages/%s", known[page_num - 1]);
                cb->quit_flag = true;
                snprintf(cb->status_msg, sizeof(cb->status_msg),
                         "→ Switching page...");
            }
        }
        return false;
    }

    /* btn-back / btn-back-N → 返回菜单 */
    if (btn_id && (strcmp(btn_id, "btn-back") == 0 ||
                   strncmp(btn_id, "btn-back-", 9) == 0)) {
        snprintf(cb->switch_page, sizeof(cb->switch_page),
                 "pages/00-menu.html");
        cb->quit_flag = true;
        snprintf(cb->status_msg, sizeof(cb->status_msg),
                 "→ Back to menu...");
        return false;
    }

    /* 默认: 显示按钮文字 */
    snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", btn_text);
    return false;
}

static void mb_default_on_focus(int focus_idx, const char* tag_name,
                                 InteractCallbacks* cb) {
    if (focus_idx >= 0)
        snprintf(cb->status_msg, sizeof(cb->status_msg),
                 "🔲 %s (#%d)", tag_name, focus_idx);
}

static bool mb_default_on_key(struct tb_event* ev, InteractCallbacks* cb) {
    MiniBrowser* mb = (MiniBrowser*)cb->user_data;

    /* 用户回调优先 */
    if (mb && mb->config.on_key) {
        if (mb->config.on_key(ev->ch, ev->key, mb, mb->config.userdata))
            return true;
    }

    /* 默认: q / Esc = 退出 */
    if (ev->ch == 'q' || ev->ch == 'Q' || ev->key == TB_KEY_ESC ||
        ev->key == TB_KEY_CTRL_C || ev->key == TB_KEY_CTRL_D) {
        cb->quit_flag = true;
        return true;
    }
    return false;
}

/* ═══════════════════════════════════════════════════════════════════ */
/*  内部: 加载 / 卸载页面                                              */
/* ═══════════════════════════════════════════════════════════════════ */

static bool mb_load_page(MiniBrowser* mb) {
    /* 读取文件 */
    mb->raw_html = mb_read_file(mb->current_path, NULL);
    if (!mb->raw_html) return false;

    size_t raw_len = strlen(mb->raw_html);
    size_t buf_size = raw_len + 4096;
    if (buf_size < 8192) buf_size = 8192;

    mb->css_buf    = (char*)calloc(buf_size, 1);
    mb->clean_html = (char*)calloc(buf_size, 1);
    mb_extract_css(mb->raw_html, mb->css_buf, buf_size,
                   mb->clean_html, buf_size);

    if (strlen(mb->clean_html) == 0)
        strncpy(mb->clean_html,
                "<html><body><p>No content</p></body></html>",
                buf_size - 1);

    /* 解析 */
    mb->dom = gumbo_parse(mb->clean_html);
    if (strlen(mb->css_buf) > 0)
        mb->css_output = katana_parse(mb->css_buf, strlen(mb->css_buf),
                                       KatanaParserModeStylesheet);
    else
        mb->css_output = katana_parse("", 0, KatanaParserModeStylesheet);

    mb->styled_root = build_style_tree(mb->dom->root,
                                        mb->css_output->stylesheet);

    /* 初始化终端 (仅首次) */
    if (!mb->term_inited) {
        if (render_init()) {
            fprintf(stderr, "render_init failed\n");
            return false;
        }
        mb->term_inited = true;
    }

    /* 构建布局 */
    int vw, vh;
    render_size(&vw, &vh);
    mb->layout_root = build_layout_tree(mb->styled_root, vw, vh);

    /* 设置 InteractCallbacks */
    memset(&mb->cb, 0, sizeof(mb->cb));
    mb->cb.on_button_click = mb_default_on_click;
    mb->cb.on_focus_change = mb_default_on_focus;
    mb->cb.on_key          = mb_default_on_key;
    mb->cb.user_data       = mb;
    mb->cb.quit_flag       = false;
    mb->cb.status_msg[0]   = '\0';
    mb->cb.switch_page[0]  = '\0';
    mb->cb.layout_root     = mb->layout_root;
    mb->cb.show_scrollbars = mb->config.show_scrollbars;

    /* 状态栏: 显示当前文件名 */
    const char* base = strrchr(mb->current_path, '/');
    base = base ? base + 1 : mb->current_path;
    snprintf(mb->cb.status_msg, sizeof(mb->cb.status_msg),
             "📄 %s — Tab: focus | Enter: click | q: quit", base);

    return true;
}

static void mb_unload_page(MiniBrowser* mb) {
    if (mb->layout_root)  { free_layout_tree(mb->layout_root);  mb->layout_root  = NULL; }
    if (mb->styled_root)  { free_style_tree(mb->styled_root);   mb->styled_root  = NULL; }
    if (mb->css_output)   { katana_destroy_output(mb->css_output); mb->css_output = NULL; }
    if (mb->dom)          { gumbo_destroy_output(&kGumboDefaultOptions, mb->dom); mb->dom = NULL; }
    if (mb->css_buf)      { free(mb->css_buf);   mb->css_buf    = NULL; }
    if (mb->clean_html)   { free(mb->clean_html); mb->clean_html = NULL; }
    if (mb->raw_html)     { free(mb->raw_html);  mb->raw_html   = NULL; }
}

/* ═══════════════════════════════════════════════════════════════════ */
/*  公开 API 实现                                                      */
/* ═══════════════════════════════════════════════════════════════════ */

MiniBrowser* mini_browser_open(const char* html_path, MB_Config* config) {
    MiniBrowser* mb = (MiniBrowser*)calloc(1, sizeof(MiniBrowser));
    if (!mb) return NULL;

    strncpy(mb->current_path, html_path, sizeof(mb->current_path) - 1);
    mb->current_path[sizeof(mb->current_path) - 1] = '\0';

    if (config) {
        mb->config = *config;
    } else {
        mb->config = (MB_Config)MB_CONFIG_DEFAULT;
    }

    if (!mb_load_page(mb)) {
        free(mb);
        return NULL;
    }
    return mb;
}

void mini_browser_run_loop(MiniBrowser* mb) {
    if (!mb) return;

    while (1) {
        interact_run(mb->layout_root, mb->css_output->stylesheet,
                     mb->styled_root, &mb->cb);

        /* 检查页面切换 */
        if (mb->cb.switch_page[0]) {
            strncpy(mb->current_path, mb->cb.switch_page,
                    sizeof(mb->current_path) - 1);
            mb->current_path[sizeof(mb->current_path) - 1] = '\0';

            mb_unload_page(mb);
            if (!mb_load_page(mb)) break;
        } else {
            break;
        }
    }
}

void mini_browser_close(MiniBrowser* mb) {
    if (!mb) return;
    mb_unload_page(mb);
    if (mb->term_inited) render_shutdown();
    free(mb);
}

void mini_browser_run(const char* html_path) {
    MiniBrowser* mb = mini_browser_open(html_path, NULL);
    if (!mb) return;
    mini_browser_run_loop(mb);
    mini_browser_close(mb);
    printf("Goodbye.\n");
}

/* ═══════════════════════════════════════════════════════════════════ */
/*  运行时 API 实现                                                    */
/* ═══════════════════════════════════════════════════════════════════ */

void mb_set_status(MiniBrowser* mb, const char* msg) {
    if (mb && msg)
        snprintf(mb->cb.status_msg, sizeof(mb->cb.status_msg), "%s", msg);
}

void mb_set_text(MiniBrowser* mb, const char* element_id, const char* text) {
    if (!mb || !element_id || !text) return;
    LayoutNode* n = find_node_by_id(mb->cb.layout_root, element_id);
    if (n) node_set_text(n, text);
    node_override_text(&mb->cb, element_id, text);
}

const char* mb_get_text(MiniBrowser* mb, const char* element_id) {
    if (!mb || !element_id) return "";
    LayoutNode* n = find_node_by_id(mb->cb.layout_root, element_id);
    return n ? node_get_text(n) : "";
}

void mb_set_visible(MiniBrowser* mb, const char* element_id, bool visible) {
    if (!mb || !element_id) return;
    LayoutNode* n = find_node_by_id(mb->cb.layout_root, element_id);
    if (n) node_set_visible(n, visible);
}

void mb_switch_page(MiniBrowser* mb, const char* page_path) {
    if (mb && page_path) {
        snprintf(mb->cb.switch_page, sizeof(mb->cb.switch_page),
                 "%s", page_path);
        mb->cb.quit_flag = true;
    }
}

void mb_quit(MiniBrowser* mb) {
    if (mb) mb->cb.quit_flag = true;
}

#endif /* MINI_BROWSER_IMPLEMENTATION */
#endif /* MINI_BROWSER_H */

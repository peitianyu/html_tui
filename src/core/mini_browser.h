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
 * 可点击元素回调 (data-action / cursor:pointer / <a href> 触发)。
 *
 * @param action     元素的 action 字符串 (data-action 值或 href；无则 NULL)
 * @param element_id 元素 id (无则 NULL)
 * @param tag        标签名 (如 "div" / "a")
 * @param mb         MiniBrowser 句柄
 * @param userdata   MB_Config.userdata
 * @return true 表示需要重建布局树
 */
typedef bool (*MB_ElementClickFn)(const char* action, const char* element_id,
                                  const char* tag, MiniBrowser* mb, void* userdata);

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
    MB_ElementClickFn on_element_click; /* 可点击元素 (NULL=默认) */
    MB_KeyFn         on_key;            /* 全局按键 (NULL=默认) */
    void*            userdata;          /* 透传给回调 */
    bool             show_scrollbars;   /* 显示滚动条 (默认 true) */
    bool             show_statusbar;    /* 显示状态栏 (默认 true) */
} MB_Config;

#define MB_CONFIG_DEFAULT \
    { .on_button_click = NULL, .on_element_click = NULL, \
      .on_key = NULL, \
      .userdata = NULL, .show_scrollbars = true, .show_statusbar = true }

/* ═══════════════════════════════════════════════════════════════════ */
/*  核心 API                                                         */
/* ═══════════════════════════════════════════════════════════════════ */

#define MB_HIST_MAX 32  /* 历史栈深度 */

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

/** 标记链接为已访问 (影响 :visited 匹配) */
void mb_mark_visited(MiniBrowser* mb, const char* url);

/**
 * 加载主题并应用。
 * @param path 主题文件路径; NULL=按默认来源解析 (--theme > theme.ini > 用户配置)
 */
void mb_apply_theme(MiniBrowser* mb, const char* path);

/**
 * 显示/隐藏元素。
 * 注意: 需要回调返回 true 来触发布局重建, 否则无效果。
 */
void mb_set_visible(MiniBrowser* mb, const char* element_id, bool visible);

/** 切换页面 (当前循环结束后加载) */
void mb_switch_page(MiniBrowser* mb, const char* page_path);
void mb_navigate(MiniBrowser* mb, const char* page_path);
void mb_go_back(MiniBrowser* mb);

/** 退出交互循环 */
void mb_quit(MiniBrowser* mb);

/** 显示/隐藏状态栏 (运行时切换, 立即生效) */
void mb_show_statusbar(MiniBrowser* mb, bool show);

#ifdef __cplusplus
}
#endif

/* ═══════════════════════════════════════════════════════════════════ */
/*  实现 — 仅当定义 MINI_BROWSER_IMPLEMENTATION 时编译                   */
/* ═══════════════════════════════════════════════════════════════════ */
#ifdef MINI_BROWSER_IMPLEMENTATION

/* ── 内部头文件 (单头文件库模式, 仅在此处展开实现) ── */

#define THEME_IMPLEMENTATION
#include "theme.h"

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
#include <dirent.h>

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

    /* 历史栈: history[0]=首屏, hist_idx 指向当前页; 导航 push, 返回 pop */
    char history[MB_HIST_MAX][256];
    int  hist_idx;
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

/* 从 <tag ...> 内提取属性值 (attr 大小写不敏感, 值支持单/双引号, 找不到返回 NULL) */
static char* mb_attr_get(const char* tag, size_t tag_len, const char* attr) {
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

/* 路径的目录部分 (不含尾部斜杠; 无斜杠 → 空串) */
static void mb_dirname(const char* path, char* out, size_t cap) {
    const char* slash = path ? strrchr(path, '/') : NULL;
    if (slash && slash != path) {
        size_t n = (size_t)(slash - path);
        if (n >= cap) n = cap - 1;
        memcpy(out, path, n);
        out[n] = '\0';
    } else {
        out[0] = '\0';
    }
}

typedef enum { MB_EXTRACT_TEXT, MB_EXTRACT_STYLE } MB_ExtractState;

static void mb_extract_css(const char* raw, char* css, size_t cs,
                           char* html, size_t hs, const char* base_dir) {
    css[0] = '\0'; html[0] = '\0';
    size_t cp = 0, hp = 0;
    MB_ExtractState st = MB_EXTRACT_TEXT;
    const char* p = raw;
    while (*p) {
        if (st == MB_EXTRACT_TEXT) {
            if (p[0] == '<') {
                /* <link rel="stylesheet" href="X"> — 外部 CSS (整标签从 html 移除) */
                if ((p[1] == 'l'||p[1]=='L') && (p[2] == 'i'||p[2]=='I') &&
                    (p[3] == 'n'||p[3]=='N') && (p[4] == 'k'||p[4]=='K') &&
                    (p[5] == '>' || p[5] == ' ' || p[5] == '\t' ||
                     p[5] == '\n' || p[5] == '\r')) {
                    const char* gt = strchr(p + 5, '>');
                    if (gt) {
                        size_t tag_len = (size_t)(gt - p) + 1;
                        char* rel = mb_attr_get(p, tag_len, "rel");
                        char* href = mb_attr_get(p, tag_len, "href");
                        if (rel && strcasestr(rel, "stylesheet") && href && href[0]) {
                            char path[1024];
                            if (base_dir && base_dir[0])
                                snprintf(path, sizeof(path), "%s/%s", base_dir, href);
                            else
                                snprintf(path, sizeof(path), "%s", href);
                            char* ext = mb_read_file(path, NULL);
                            if (ext) {
                                size_t elen = strlen(ext);
                                if (cp + elen + 2 < cs) {
                                    memcpy(css + cp, ext, elen);
                                    cp += elen;
                                    css[cp++] = '\n';
                                }
                                free(ext);
                            }
                        }
                        free(rel); free(href);
                        p = gt + 1;  /* 跳过整个标签 */
                        continue;
                    }
                }
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
        char prev_status[256];
        snprintf(prev_status, sizeof(prev_status), "%s", cb->status_msg);
        bool ret = mb->config.on_button_click(btn_id, btn_text, mb,
                                              mb->config.userdata);
        if (ret) return true;
        /* 用户回调修改了状态消息 → 保留, 不再覆盖默认文本 */
        if (strcmp(prev_status, cb->status_msg) != 0) return false;
    }

    /* btn-page-NN → 扫描 pages/ 目录找 NN-*.html (自动适配新增页面) */
    if (btn_id && strncmp(btn_id, "btn-page-", 9) == 0) {
        int page_num = atoi(btn_id + 9);
        if (page_num >= 1 && page_num <= 99) {
            char prefix[8];
            snprintf(prefix, sizeof(prefix), "%02d-", page_num);
            DIR* d = opendir("pages");
            if (d) {
                struct dirent* e;
                while ((e = readdir(d))) {
                    if (e->d_name[0] == '.') continue;
                    if (strncmp(e->d_name, prefix, 3) == 0 &&
                        strstr(e->d_name, ".html")) {
                        snprintf(cb->switch_page, sizeof(cb->switch_page),
                                 "pages/%s", e->d_name);
                        cb->quit_flag = true;
                        snprintf(cb->status_msg, sizeof(cb->status_msg),
                                 "→ Switching page...");
                        break;
                    }
                }
                closedir(d);
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

    /* 默认: Esc = 退出 */
    if (ev->key == TB_KEY_ESC) {
        cb->quit_flag = true;
        return true;
    }

    /* b / B = 返回上一页 (历史栈); 输入框编辑中不拦截 (让 b 进入输入) */
    if ((ev->ch == 'b' || ev->ch == 'B') && mb && !cb->edit_active) {
        if (mb->hist_idx > 0)
            mb_go_back(mb);
        else
            mb_set_status(mb, "↩ Already at first page");
        return true;
    }
    return false;
}

/* 可点击元素默认回调: <a> 自动记 visited + 状态栏显示 action */
static bool mb_default_on_element_click(const char* action, const char* eid,
                                        const char* tag, InteractCallbacks* cb)
{
    MiniBrowser* mb = (MiniBrowser*)cb->user_data;
    if (!mb) return false;
    (void)eid;

    /* 用户回调优先 */
    if (mb->config.on_element_click) {
        bool ret = mb->config.on_element_click(action, eid, tag, mb,
                                               mb->config.userdata);
        if (ret) return true;
    }

    /* 默认: <a> 链接自动记 visited; 状态栏显示 action */
    if (tag && strcmp(tag, "a") == 0 && action)
        mb_mark_visited(mb, action);
    snprintf(cb->status_msg, sizeof(cb->status_msg), "⏎ %s",
             action ? action : "(none)");
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
    char base_dir[512];
    mb_dirname(mb->current_path, base_dir, sizeof(base_dir));
    mb_extract_css(mb->raw_html, mb->css_buf, buf_size,
                   mb->clean_html, buf_size, base_dir);

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

    /* 初始化终端 (仅首次) — 需在样式树构建前拿视口 (供 @media) */
    if (!mb->term_inited) {
        if (render_init()) {
            fprintf(stderr, "render_init failed\n");
            return false;
        }
        mb->term_inited = true;
    }
    int vw, vh;
    render_size(&vw, &vh);

    mb->styled_root = build_style_tree(mb->dom->root,
                                        mb->css_output->stylesheet, vw);

    /* 构建布局 (base_dir 供 <img src> 相对路径解析) */
    mb->layout_root = build_layout_tree(mb->styled_root, vw, vh, base_dir);

    /* 设置 InteractCallbacks */
    memset(&mb->cb, 0, sizeof(mb->cb));
    mb->cb.on_button_click = mb_default_on_click;
    mb->cb.on_element_click = mb_default_on_element_click;
    mb->cb.on_focus_change = mb_default_on_focus;
    mb->cb.on_key          = mb_default_on_key;
    mb->cb.user_data       = mb;
    mb->cb.quit_flag       = false;
    mb->cb.status_msg[0]   = '\0';
    mb->cb.switch_page[0]  = '\0';
    mb->cb.layout_root     = mb->layout_root;
    mb->cb.base_dir        = base_dir;
    mb->cb.show_scrollbars = mb->config.show_scrollbars;
    mb->cb.show_statusbar  = mb->config.show_statusbar;

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

    /* 历史栈: 首屏为 history[0] */
    mb->hist_idx = 0;
    strncpy(mb->history[0], mb->current_path, sizeof(mb->history[0]) - 1);
    mb->history[0][sizeof(mb->history[0]) - 1] = '\0';

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

void mb_apply_theme(MiniBrowser* mb, const char* path) {
    (void)mb;
    Theme def; theme_default(&def); theme_apply(&def);
    if (path && path[0]) {
        if (theme_load(&g_theme, path) != 0)
            fprintf(stderr, "theme: cannot load '%s'\n", path);
    } else {
        theme_reload_from_source();
    }
}

void mb_set_status(MiniBrowser* mb, const char* msg) {
    if (mb && msg)
        snprintf(mb->cb.status_msg, sizeof(mb->cb.status_msg), "%s", msg);
}

void mb_mark_visited(MiniBrowser* mb, const char* url) {
    (void)mb;
    if (!url || !url[0]) return;
    visited_mark(url);
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

/* 导航到新页: 先把当前页 push 进历史 (超过深度停止), 再切换 */
void mb_navigate(MiniBrowser* mb, const char* page_path) {
    if (!mb || !page_path || !page_path[0]) return;
    if (mb->hist_idx + 1 < MB_HIST_MAX) {
        mb->hist_idx++;
        strncpy(mb->history[mb->hist_idx], mb->current_path,
                sizeof(mb->history[0]) - 1);
        mb->history[mb->hist_idx][sizeof(mb->history[0]) - 1] = '\0';
    }
    mb_switch_page(mb, page_path);
}

/* 返回上一页: 无历史时仅提示, 不动 */
void mb_go_back(MiniBrowser* mb) {
    if (!mb || mb->hist_idx <= 0) return;
    mb->hist_idx--;
    mb_switch_page(mb, mb->history[mb->hist_idx]);
}

void mb_quit(MiniBrowser* mb) {
    if (mb) mb->cb.quit_flag = true;
}

void mb_show_statusbar(MiniBrowser* mb, bool show) {
    if (mb) mb->cb.show_statusbar = show;
}

#endif /* MINI_BROWSER_IMPLEMENTATION */
#endif /* MINI_BROWSER_H */

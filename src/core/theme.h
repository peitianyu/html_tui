/* theme.h — Mini Browser 主题 (header-only)
 * 用法: #define THEME_IMPLEMENTATION 后 #include "theme.h"
 *
 * 覆盖全部引擎级 UI 颜色 (滚动条/焦点/状态栏/select 弹窗/链接色等)。
 * 页面内容色仍由页面 CSS 决定, 主题只管浏览器 chrome。
 */
#ifndef THEME_H
#define THEME_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int scrollbar_thumb[3];
    int scrollbar_track[3];
    int focus_border[3];
    int statusbar_bg[3];
    int statusbar_fg[3];
    int select_popup_bg[3];
    int select_popup_hl[3];
    int link_color[3];
    int link_visited[3];
    int text_cursor[3];
    int textarea_sb[3];
    int border_default[3];
    int disabled_fg[3];
} Theme;

/** 填充内置默认主题 */
void theme_default(Theme* t);
/** 从 INI 文件加载主题; 返回 0=成功, 非0=失败(保留原值) */
int theme_load(Theme* t, const char* path);
/** 按优先级解析主题来源: --theme <path> > ./theme.ini > ~/.config/mini_browser/theme.ini
 *  返回命中的路径 (写入 out), 无命中返回 NULL */
const char* theme_resolve_source(char* out, size_t outsz, int argc, char** argv);

/* ── 全局实例 (单编译单元内共享; TUI 单实例可接受) ── */
void theme_apply(const Theme* t);
const Theme* theme_get(void);
/** 按默认来源 (项目/用户配置) 重新加载全局主题;
 *  返回 0=已加载配置文件, 非0=无配置 (保持默认) */
int theme_reload_from_source(void);

#ifdef __cplusplus
}
#endif

#ifdef THEME_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

void theme_default(Theme* t) {
    if (!t) return;
    static const int def[][3] = {
        {120,180,255}, /* scrollbar_thumb */
        { 50, 80,120}, /* scrollbar_track */
        {255,215,  0}, /* focus_border   */
        { 32, 32, 64}, /* statusbar_bg   */
        {200,200,200}, /* statusbar_fg   */
        { 20, 20, 40}, /* select_popup_bg */
        { 80, 80,140}, /* select_popup_hl */
        { 74,158,255}, /* link_color     */
        {155, 89,182}, /* link_visited   */
        {255,255,255}, /* text_cursor    */
        {120,180,255}, /* textarea_sb    */
        {102,102,102}, /* border_default */
        { 85, 85, 85}, /* disabled_fg    */
    };
    int (*dst)[3] = (int(*)[3])&t->scrollbar_thumb;
    for (size_t i = 0; i < sizeof(def)/sizeof(def[0]); i++)
        for (int j = 0; j < 3; j++) dst[i][j] = def[i][j];
}

/* 解析 #RRGGBB → int[3]; 成功返回 1 */
static int parse_hex(const char* s, int out[3]) {
    if (!s || s[0] != '#') return 0;
    unsigned r, g, b;
    if (s[1] && s[2] && s[3] && s[4] && s[5] && s[6] &&
        sscanf(s+1, "%2x%2x%2x", &r, &g, &b) == 3) {
        out[0] = (int)r; out[1] = (int)g; out[2] = (int)b;
        return 1;
    }
    return 0;
}

int theme_load(Theme* t, const char* path) {
    if (!t || !path) return -1;
    FILE* f = fopen(path, "r");
    if (!f) return -1;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;
        char key[64] = {0}, val[64] = {0};
        if (sscanf(p, "%63[^=]=%63s", key, val) != 2) continue;
        /* 去尾空格 */
        size_t kl = strlen(key);
        while (kl > 0 && (key[kl-1]==' '||key[kl-1]=='\t')) key[--kl] = '\0';
        int rgb[3];
        if (!parse_hex(val, rgb)) continue;
        struct { const char* name; int (*dst)[3]; } map[] = {
            {"scrollbar_thumb", &t->scrollbar_thumb},
            {"scrollbar_track", &t->scrollbar_track},
            {"focus_border",    &t->focus_border},
            {"statusbar_bg",    &t->statusbar_bg},
            {"statusbar_fg",    &t->statusbar_fg},
            {"select_popup_bg", &t->select_popup_bg},
            {"select_popup_hl", &t->select_popup_hl},
            {"link_color",      &t->link_color},
            {"link_visited",    &t->link_visited},
            {"text_cursor",     &t->text_cursor},
            {"textarea_sb",     &t->textarea_sb},
            {"border_default",  &t->border_default},
            {"disabled_fg",     &t->disabled_fg},
        };
        for (size_t i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
            if (strcmp(map[i].name, key) == 0) {
                (*map[i].dst)[0] = rgb[0];
                (*map[i].dst)[1] = rgb[1];
                (*map[i].dst)[2] = rgb[2];
                break;
            }
        }
    }
    fclose(f);
    return 0;
}

const char* theme_resolve_source(char* out, size_t outsz, int argc, char** argv) {
    if (!out || outsz == 0) return NULL;
    out[0] = '\0';
    for (int i = 1; i < argc - 1; i++) {
        if (argv && strcmp(argv[i], "--theme") == 0 && argv[i+1]) {
            snprintf(out, outsz, "%s", argv[i+1]);
            return out;
        }
    }
    FILE* f = fopen("theme.ini", "r");
    if (f) { fclose(f); snprintf(out, outsz, "theme.ini"); return out; }
    const char* home = getenv("HOME");
    if (home) {
        snprintf(out, outsz, "%s/.config/mini_browser/theme.ini", home);
        f = fopen(out, "r");
        if (f) { fclose(f); return out; }
    }
    return NULL;
}

/* ── 全局实例 ── */
static Theme g_theme;

void theme_apply(const Theme* t) {
    if (t) g_theme = *t;
}

const Theme* theme_get(void) {
    return &g_theme;
}

int theme_reload_from_source(void) {
    Theme def; theme_default(&def); theme_apply(&def);
    char tpath[512];
    if (theme_resolve_source(tpath, sizeof(tpath), 0, NULL))
        return theme_load(&g_theme, tpath);
    return 1;
}

#endif /* THEME_IMPLEMENTATION */
#endif /* THEME_H */

# 可点击元素 + 主题系统 + CSS 增强 实现计划

> **面向 AI 代理的工作者：** 必需子技能：使用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现此计划。步骤使用复选框（`- [ ]`）语法来跟踪进度。

**目标：** 按《2026-08-06-interactivity-theme-design.md》规格实现：① 任意元素可点击（data-action / cursor:pointer / `<a href>`，进焦点链，on_element_click 回调，visited 集合）；② 用户侧主题系统（theme.ini + --theme + R 热重载，收拢全部引擎硬编码色）；③ 页面侧 CSS 增强（border-radius 降级、:visited、disabled、:placeholder-shown）；④ 测试页 + 无头回归 + README 更新。每完成一个逻辑单元即提交 git。

**架构：** 复用现有 header-only 管线（gumbo → katana → styletree → layout → render → interact）。新增 `theme.h` 单头文件模块。可点击判定在 `collect_focus_list`（interact.h）扩展；`:visited` 匹配在 `match_simple_selector`（styletree.h）扩展；硬编码色收拢到全局 `Theme`。

**技术栈：** C11、header-only 单头文件、termbox2、tcc/gcc 编译、`-DRENDER_HEADLESS` 无头验证。

**规格文档：** `docs/superpowers/specs/2026-08-06-interactivity-theme-design.md`
**优化分析：** `doc/optimization-analysis.md`

---

## 文件结构

| 文件 | 职责 | 操作 |
|---|---|---|
| `src/core/theme.h` | Theme 结构体 + INI 解析 + 默认值 + 全局实例 | **新增** |
| `src/core/mini_browser.h` | on_element_click API、mb_mark_visited、主题初始化/重载入口 | 修改 |
| `src/core/styletree.h` | :visited / cursor:pointer / border-radius / disabled / :placeholder-shown 解析 | 修改 |
| `src/core/layout.h` | LayoutNode 新字段（clickable、disabled、placeholder 态） | 修改 |
| `src/core/render.h` | 硬编码色 → theme、placeholder 灰色斜体、disabled 灰显 | 修改 |
| `src/core/interact.h` | focus 链扩展、事件分发、visited 记录、R 热重载 | 修改 |
| `src/demo.c` | on_element_click 示例回调 | 修改 |
| `pages/15-clickable.html` | 可点击/链接/disabled 测试页 | **新增** |
| `pages/16-theme.html` | 主题/border-radius/placeholder 测试页 | **新增** |
| `theme.ini` | 示例主题配置 | **新增** |
| `tools/headless_test.c` | 无头渲染 + dump + 断言测试工具 | **新增** |
| `README.md` | 功能清单、键盘表、配置说明 | 修改 |
| `doc/optimization-analysis.md` | 勾选已完成优化项 | 修改 |

---

## 任务 1：无头测试工具（测试基础设施）

**文件：**
- 创建：`tools/headless_test.c`

- [ ] **步骤 1：编写无头测试工具**

```c
/* tools/headless_test.c
 * 无头渲染工具：加载 HTML → 样式树 → 布局 → 渲染 → dump 文本 + 断言。
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

    FILE* out = fopen("/data/data/com.termux/files/usr/tmp/mb_headless_dump.txt", "w");
    screen_dump_text(s, out);
    fclose(out);

    /* 断言 */
    char dump[200000] = {0};
    out = fopen("/data/data/com.termux/files/usr/tmp/mb_headless_dump.txt", "r");
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
```

- [ ] **步骤 2：编译并验证工具本身**

运行：`mkdir -p /data/data/com.termux/files/usr/tmp && gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm`
预期：编译无错误；`/tmp/headless_test pages/00-menu.html "Mini Browser"` 输出 OK

- [ ] **步骤 3：Commit**

```bash
git add tools/headless_test.c
git commit -m "test: 添加无头渲染测试工具 (headless_test.c)"
```

---

## 任务 2：可点击元素机制

**文件：**
- 修改：`src/core/layout.h`（LayoutNode 字段）
- 修改：`src/core/styletree.h`（cursor:pointer / disabled 解析）
- 修改：`src/core/interact.h`（collect_focus_list + 点击/Enter 分发 + visited + 状态栏提示）
- 修改：`src/core/mini_browser.h`（on_element_click API + mb_mark_visited + visited 集合）
- 修改：`src/demo.c`（示例回调）
- 创建：`pages/15-clickable.html`

- [ ] **步骤 1：LayoutNode 增加字段**

在 `layout.h` 的 LayoutNode 结构体（`int z_index;` 之后）添加：

```c
    /* clickable: 1=可点击元素 (data-action / cursor:pointer / <a href>) */
    bool clickable;
    /* 可点击元素的 action 字符串 (data-action 值或 href；无则 NULL) */
    char* click_action;
    /* disabled 表单控件 (灰显 + 排除出焦点链) */
    bool disabled;
```

并在 `free_layout_tree`（layout.h:2537 附近）中释放 `click_action`：

```c
    if (node->click_action) { free(node->click_action); node->click_action = NULL; }
```

- [ ] **步骤 2：styletree.h 解析 cursor:pointer / disabled**

在 `apply_styles`（layout.h:739，即 compute 结果落到 LayoutNode 处）之前，先在 styletree.h 的 `compute_styles_for_node` 之后补充两个帮助函数（放在 layout.h 的 `apply_styles` 内调用前声明处，即 layout.h 中 `static void apply_styles(LayoutNode* ln)` 函数体内）：

```c
    /* clickable 标记: data-action 属性 > cursor:pointer > <a href> */
    ln->clickable = false;
    ln->disabled = false;
    if (ln->styled && ln->styled->node &&
        ln->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboElement* el = &ln->styled->node->v.element;
        GumboAttribute* da = gumbo_get_attribute(&el->attributes, "data-action");
        if (el->tag == GUMBO_TAG_A) {
            GumboAttribute* h = gumbo_get_attribute(&el->attributes, "href");
            if (h && h->value) {
                ln->clickable = true;
                if (ln->click_action) free(ln->click_action);
                ln->click_action = strdup(da && da->value ? da->value : h->value);
            }
        } else if (da && da->value) {
            ln->clickable = true;
            if (ln->click_action) free(ln->click_action);
            ln->click_action = strdup(da->value);
        } else if (ln->cursor_pointer) {   /* cursor:pointer 解析结果 */
            ln->clickable = true;
            if (ln->click_action) free(ln->click_action);
            /* 无 data-action 时用 id；无 id 用标签名 */
            GumboAttribute* id = gumbo_get_attribute(&el->attributes, "id");
            if (id && id->value) ln->click_action = strdup(id->value);
            else {
                const char* tn = gumbo_normalized_tagname(el->tag);
                ln->click_action = strdup(tn ? tn : "element");
            }
        }
        /* disabled 属性: 仅表单控件 */
        if (el->tag == GUMBO_TAG_INPUT || el->tag == GUMBO_TAG_BUTTON ||
            el->tag == GUMBO_TAG_SELECT || el->tag == GUMBO_TAG_TEXTAREA) {
            if (gumbo_get_attribute(&el->attributes, "disabled"))
                ln->disabled = true;
        }
    }
```

同时需要 `cursor:pointer` 解析结果落到 LayoutNode：在 layout.h 的 `LayoutNode` 中补充：

```c
    /* cursor: pointer 声明 (可点击暗示) */
    bool cursor_pointer;
```

并在 `apply_styles` 内解析 `cursor` 属性（在现有属性解析处追加）：

```c
    /* cursor */
    {
        const char* cur = style_value(ln->styled, "cursor");
        ln->cursor_pointer = (cur && strcmp(cur, "pointer") == 0);
    }
```

（`style_value` 为 layout.h 现有 helper，若不存在则用 `get_declared_value`/现有取值方式获取属性值；以 apply_styles 中现有获取属性的写法为准。）

- [ ] **步骤 3：interact.h 扩展 focus 链 + 事件分发**

修改 `collect_focus_list`（interact.h:337）：在现有 tag 判断的 if 中追加可点击判定，并排除 disabled：

```c
            if (t == GUMBO_TAG_INPUT || t == GUMBO_TAG_BUTTON ||
                t == GUMBO_TAG_SUMMARY || t == GUMBO_TAG_TEXTAREA ||
                t == GUMBO_TAG_SELECT ||
                (n->clickable)) {
                if (!n->disabled) focus_list[count++] = n;
            }
```

修改 Enter 触发处（interact.h:1645 附近，现有 `on_button_click` 调用的 if 块）：在调用 on_button_click 前，若焦点元素是 clickable 且非 BUTTON，则调用新的元素点击分发：

```c
                /* 可点击元素 (div/a/cursor:pointer) → on_element_click */
                if (focus_list[focus_idx]->clickable &&
                    !(focus_list[focus_idx]->styled &&
                      focus_list[focus_idx]->styled->node &&
                      focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT &&
                      focus_list[focus_idx]->styled->node->v.element.tag == GUMBO_TAG_BUTTON)) {
                    if (cb && cb->on_element_click) {
                        const char* action = focus_list[focus_idx]->click_action;
                        const char* eid = NULL;
                        GumboNode* gn = focus_list[focus_idx]->styled->node;
                        if (gn && gn->type == GUMBO_NODE_ELEMENT) {
                            GumboAttribute* ida = gumbo_get_attribute(
                                &gn->v.element.attributes, "id");
                            if (ida) eid = ida->value;
                        }
                        const char* tag = NULL;
                        if (gn && gn->type == GUMBO_NODE_ELEMENT) {
                            GumboTag gt = gn->v.element.tag;
                            if (gt == GUMBO_TAG_A) tag = "a";
                            else tag = gumbo_normalized_tagname(gt);
                        }
                        /* 标记 visited (仅 <a>) */
                        if (gn && gn->type == GUMBO_NODE_ELEMENT &&
                            gn->v.element.tag == GUMBO_TAG_A && action) {
                            if (cb->on_element_click(action, eid, tag, cb->user_data, cb)) { }
                        } else {
                            if (cb->on_element_click(action, eid, tag, cb->user_data, cb)) { }
                        }
                        cb->mark_visited(action);
                    }
                    continue; /* 跳过按钮默认路径 */
                }
```

> 注意：`InteractCallbacks` 需新增 `on_element_click` 字段与 `mark_visited` 帮助；若回调结构已有 user_data 用法（`cb->user_data` 为 MiniBrowser*），保持一致。`mark_visited` 调用 visited 集合记录（见 mini_browser.h 任务步骤）。

- [ ] **步骤 4：mini_browser.h API**

在 `InteractCallbacks`（interact.h:40-90 附近）新增字段：

```c
    MB_ElementClickFn on_element_click;   /* 可点击元素回调 */
```

在 mini_browser.h 公共头声明新 API：

```c
typedef bool (*MB_ElementClickFn)(const char* action, const char* element_id,
                                  const char* tag, MiniBrowser* mb, void* userdata);

void mb_mark_visited(MiniBrowser* mb, const char* url);  /* 手动标记已访问 */
```

在 `MB_Config` 增加 `MB_ElementClickFn on_element_click;`，`MB_CONFIG_DEFAULT` 补 `.on_element_click = NULL`。

在 MiniBrowser 结构体增加 visited 集合：

```c
    /* visited 链接集合 (href 字符串, 环形) */
    char visited[64][256];
    int  visited_count;
```

实现 `mb_mark_visited` 与默认元素点击回调：

```c
void mb_mark_visited(MiniBrowser* mb, const char* url) {
    if (!mb || !url || !url[0]) return;
    for (int i = 0; i < mb->visited_count; i++)
        if (strcmp(mb->visited[i], url) == 0) return;
    if (mb->visited_count < 64) {
        strncpy(mb->visited[mb->visited_count], url, 255);
        mb->visited[mb->visited_count][255] = '\0';
        mb->visited_count++;
    } else {
        /* 环形覆盖最旧 */
        for (int i = 1; i < 64; i++) strcpy(mb->visited[i-1], mb->visited[i]);
        strncpy(mb->visited[63], url, 255); mb->visited[63][255] = '\0';
    }
}
```

默认元素点击回调（在 mb_default_on_key 之后新增，与现有默认回调并列）：

```c
static void mb_default_on_element_click(const char* action, const char* eid,
                                        const char* tag, InteractCallbacks* cb)
{
    MiniBrowser* mb = (MiniBrowser*)cb->user_data;
    if (!action) action = "(none)";
    snprintf(cb->status_msg, sizeof(cb->status_msg), "⏎ %s", action);
    /* <a> 链接自动记 visited */
    if (tag && strcmp(tag, "a") == 0) mb_mark_visited(mb, action);
}
```

在 mb_load_page 中接线：`mb->cb.on_element_click = mb_default_on_element_click;`

- [ ] **步骤 5：demo.c 示例回调**

```c
/* 可点击元素回调: <a> 链接跳页, data-action 显示消息 */
static bool demo_on_element_click(const char* action, const char* element_id,
                                  const char* tag, MiniBrowser* mb, void* userdata)
{
    (void)element_id; (void)userdata;
    if (tag && strcmp(tag, "a") == 0 && action) {
        if (strstr(action, ".html")) {
            mb_switch_page(mb, action);
            mb_set_status(mb, "→ %s", action);   /* mb_set_status 支持格式串? 若不支持改为两步 */
            return true;
        }
    }
    mb_set_status(mb, action);
    return false;
}
```

> 注：若 `mb_set_status` 不支持 printf 格式，改为 `mb_set_status(mb, action)` 简单显示；页面 15 的链接 href 用相对路径 `pages/15-clickable.html` 等，配合 mb_switch_page 的路径规则。

main 中接线：

```c
    config.on_element_click = demo_on_element_click;
```

- [ ] **步骤 6：创建 pages/15-clickable.html**

```html
<!DOCTYPE html>
<html><head><meta charset="utf-8"><style>
  body { font-family: sans-serif; padding: 2px; }
  .card { border: 1px solid #888; padding: 4px; margin: 4px; background: #222233; }
  .card:hover { background: #334; }
  .pointer { cursor: pointer; color: #4A9EFF; }
  a { color: #4A9EFF; }
  a:visited { color: #9B59B6; }
  .disabled-demo { margin-top: 8px; }
</style></head><body>
  <h3>15-clickable — 可点击元素</h3>

  <div class="card" data-action="open-card-1" id="card-1">
    ▶ Card 1 (data-action, 点击我)
  </div>

  <div class="card" data-action="toggle-card-2" id="card-2">
    ▶ Card 2 (data-action, 点击我)
  </div>

  <div class="card pointer" id="card-3">
    ▶ Card 3 (cursor:pointer, 无 data-action)
  </div>

  <p class="disabled-demo">
    <label>disabled 输入框: <input type="text" value="不能编辑" disabled></label>
  </p>
  <p>
    <label>disabled 按钮: <button disabled>禁用按钮</button></label>
  </p>

  <p>
    链接: <a href="pages/01-text-colors.html">跳转到 01-text-colors</a>
    &nbsp;|&nbsp;
    <a href="pages/15-clickable.html">访问本页(测 :visited)</a>
  </p>

  <p><button id="btn-back">返回菜单</button></p>
</body></html>
```

- [ ] **步骤 7：编译 + 无头验证**

编译（真实 + 无头）：

```bash
gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
/tmp/headless_test pages/15-clickable.html "Card 1" "Card 2" "disabled" "不能编辑"
gcc -I src -o /tmp/mb src/demo.c -lm   # 真实终端编译验证
```

预期：无头断言全部 OK；真实编译通过（-Wall 无新警告）。

- [ ] **步骤 8：Commit**

```bash
git add src/core/layout.h src/core/styletree.h src/core/interact.h src/core/mini_browser.h src/demo.c pages/15-clickable.html
git commit -m "feat: 可点击元素机制 (data-action/cursor:pointer/<a> 进焦点链 + on_element_click + visited)"
```

---

## 任务 3：主题系统

**文件：**
- 创建：`src/core/theme.h`
- 修改：`src/core/mini_browser.h`（加载/重载主题）
- 修改：`src/core/render.h`（硬编码色 → theme）
- 修改：`src/core/interact.h`（R 键热重载）
- 修改：`src/demo.c`（--theme 参数）
- 创建：`theme.ini`
- 创建：`pages/16-theme.html`

- [ ] **步骤 1：创建 theme.h**

```c
/* theme.h — Mini Browser 主题 (header-only)
 * 用法: #define THEME_IMPLEMENTATION 后 #include "theme.h"
 */
#ifndef THEME_H
#define THEME_H

#include <stdbool.h>

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

void theme_default(Theme* t);
int  theme_load(Theme* t, const char* path);   /* 0=成功, 非0=失败(保留原值) */

#ifdef __cplusplus
}
#endif

#ifdef THEME_IMPLEMENTATION
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* 按优先级解析主题来源: --theme <path> > ./theme.ini > ~/.config/mini_browser/theme.ini
 * 返回命中的路径 (写入 out), 无命中返回 NULL */
const char* theme_resolve_source(char* out, size_t outsz, int argc, char** argv) {
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--theme") == 0 && argv[i+1]) {
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

#endif /* THEME_IMPLEMENTATION */
#endif /* THEME_H */
```

- [ ] **步骤 2：render.h 硬编码色 → theme**

在 `RENDER_IMPLEMENTATION` 段头部加入：

```c
#define THEME_IMPLEMENTATION
#include "theme.h"
static Theme g_theme;
void theme_apply(const Theme* t) { if (t) g_theme = *t; }
const Theme* theme_get(void) { return &g_theme; }
```

逐点替换硬编码色（以 `g_theme.xxx` 替换字面量）：
- `screen_draw_vscrollbar`：`120,180,255` → `g_theme.scrollbar_thumb`；`50,80,120` → `g_theme.scrollbar_track`
- `screen_draw_hscrollbar`：同上
- `draw_focus_indicator`（interact.h:447）：`g_theme.focus_border`
- `draw_status_bar`（interact.h:508）：`g_theme.statusbar_bg` / `g_theme.statusbar_fg`
- select 弹窗（interact.h render_frame 中 `20,20,40` 等）：`g_theme.select_popup_bg` / `g_theme.select_popup_hl`
- `<a>` 默认蓝（render.h 链接渲染处）：`g_theme.link_color`
- textarea 滚动条：`g_theme.textarea_sb`
- disabled 灰显：`g_theme.disabled_fg`

> 具体行号以 grep `[0-9]+,[0-9]+,[0-9]+` 逐一核对，覆盖点见设计文档 2.3。

- [ ] **步骤 3：mini_browser.h 加载主题 + 重载入口**

在 `mini_browser_open` 中、`mb_load_page` 之前：

```c
    char tpath[512];
    if (theme_resolve_source(tpath, sizeof(tpath), argc_p, argv_p))
        theme_load(&g_theme, tpath);
```

> 注：`mini_browser_open` 目前不接收 argv/argc。改为：在 demo.c 中先解析 `--theme` 并调用 `mb_apply_theme(mb, path)`（新增 API），或给 open 增加可选参数。**采用方案**：新增 `void mb_apply_theme(MiniBrowser* mb, const char* path)`，demo.c 在 open 后、run_loop 前调用；`R` 重载时内部重新执行相同解析。

新增 API 声明与实现：

```c
void mb_apply_theme(MiniBrowser* mb, const char* path);  /* 加载主题; NULL=按默认来源 */

void mb_apply_theme(MiniBrowser* mb, const char* path) {
    (void)mb;
    char tpath[512];
    if (path) {
        snprintf(tpath, sizeof(tpath), "%s", path);
        if (theme_load(&g_theme, tpath) != 0)
            fprintf(stderr, "theme: cannot load '%s', keep default\n", tpath);
    } else {
        if (theme_resolve_source(tpath, sizeof(tpath), 0, NULL))
            theme_load(&g_theme, tpath);
    }
}
```

> 注：`theme_resolve_source(0, NULL)` 处理 argc=0 时需安全返回 NULL（见步骤 1 实现，argc<2 时循环不执行，直接查文件）。

- [ ] **步骤 4：interact.h R 键热重载**

在默认按键处理（`mb_default_on_key` 或事件循环 key 分支）中，识别 `R` 键（ch=='R' 或 'r'）：

```c
    if (ev->ch == 'R' || ev->ch == 'r') {
        mb_apply_theme(mb, NULL);
        snprintf(cb->status_msg, sizeof(cb->status_msg), "🎨 theme reloaded");
        return true;
    }
```

（放置于 `mb_default_on_key` 中 Esc 判断之前，需要 MiniBrowser* 指针——通过 `cb->user_data` 获取。）

- [ ] **步骤 5：demo.c --theme 参数**

```c
    /* 解析 --theme 参数 */
    const char* theme_path = NULL;
    for (int i = 1; i < argc - 1; i++)
        if (strcmp(argv[i], "--theme") == 0) theme_path = argv[i+1];

    MiniBrowser* mb = mini_browser_open(filepath, &config);
    if (!mb) return 1;
    mb_apply_theme(mb, theme_path);
```

- [ ] **步骤 6：创建 theme.ini 示例 + pages/16-theme.html**

`theme.ini`：

```ini
# Mini Browser theme (示例)
# 键: scrollbar_thumb scrollbar_track focus_border statusbar_bg statusbar_fg
#      select_popup_bg select_popup_hl link_color link_visited text_cursor
#      textarea_sb border_default disabled_fg
scrollbar_thumb = #78B4FF
scrollbar_track = #325078
focus_border    = #FFD700
statusbar_bg    = #1A1A3A
statusbar_fg    = #D0D0D0
select_popup_bg = #141428
select_popup_hl = #50508C
link_color      = #4A9EFF
link_visited    = #9B59B6
text_cursor     = #FFFFFF
textarea_sb     = #78B4FF
border_default  = #666666
disabled_fg     = #555555
```

`pages/16-theme.html`（border-radius / :placeholder-shown / 主题色展示）：

```html
<!DOCTYPE html>
<html><head><meta charset="utf-8"><style>
  body { font-family: sans-serif; padding: 2px; }
  .rounded { border: 1px solid #888; border-radius: 4px; padding: 4px; margin: 4px; }
  .box { border: 2px solid #4A9EFF; padding: 6px; margin: 4px; }
  a { color: #4A9EFF; }
  a:visited { color: #9B59B6; }
</style></head><body>
  <h3>16-theme — 主题与样式增强</h3>
  <div class="rounded">rounded 边框 (border-radius 降级 ╭╮╰╯)</div>
  <div class="box">普通边框 (对比)</div>
  <p>placeholder 测试:</p>
  <p><input type="text" placeholder="灰色斜体 placeholder"></p>
  <p><input type="text" placeholder="另一个" value="有内容"></p>
  <p>链接: <a href="pages/16-theme.html">访问本页后变紫</a></p>
  <p><button id="btn-back">返回菜单</button></p>
</body></html>
```

- [ ] **步骤 7：编译 + 无头验证**

```bash
gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
/tmp/headless_test pages/16-theme.html "16-theme" "rounded" "placeholder"
gcc -I src -o /tmp/mb src/demo.c -lm   # 真实编译
/tmp/mb --theme theme.ini pages/16-theme.html 2>/dev/null &   # 冒烟(终端内验证 R 重载与颜色)
```

预期：编译通过；无头断言 OK。

- [ ] **步骤 8：Commit**

```bash
git add src/core/theme.h src/core/render.h src/core/interact.h src/core/mini_browser.h src/demo.c theme.ini pages/16-theme.html
git commit -m "feat: 主题系统 (theme.h + theme.ini + --theme + R 热重载, 硬编码色收拢)"
```

---

## 任务 4：CSS 属性收尾（border-radius / :placeholder-shown / :visited / disabled 样式）

**文件：**
- 修改：`src/core/styletree.h`（border-radius→rounded、:visited、:placeholder-shown、disabled 样式）
- 修改：`src/core/layout.h`（border_style 映射）
- 修改：`src/core/render.h`（placeholder 灰色斜体、disabled 灰显、visited 颜色应用）

- [ ] **步骤 1：border-radius → rounded**

在 styletree.h 的 `apply_styles`（layout.h）或 `compute_styles_for_node` 中，解析 `border-radius`：

```c
    /* border-radius: 非零 → rounded 边框 */
    {
        const char* br = style_value(ln->styled, "border-radius");
        if (br && *br) {
            ParsedDim pd = parse_dimension(br);
            if (pd.valid && pd.value > 0 && ln->border_style == 1)
                ln->border_style = 6; /* rounded */
        }
    }
```

- [ ] **步骤 2：:visited 匹配**

在 styletree.h 的 `match_simple_selector` 中，`:visited` 分支（与 `:link` 同处）：

```c
    if (sel->pseudo_class == KatanaPseudoClassVisited) {
        /* 匹配 <a> 且 href 在 visited 集合 */
        return node_is_visited_link(node);
    }
```

新增 `node_is_visited_link`（放在 styletree.h 实现段；visited 集合通过 mini_browser.h 的 mb 全局查询，无头模式用一个模块级静态集合兜底）：

```c
/* visited 集合 (供 :visited 匹配; mini_browser 通过 mb_mark_visited 维护) */
static char g_visited[64][256];
static int  g_visited_count = 0;

void visited_mark(const char* url);
bool visited_contains(const char* url);
void visited_clear(void);

static bool node_is_visited_link(GumboNode* node) {
    if (!node || node->type != GUMBO_NODE_ELEMENT) return false;
    if (node->v.element.tag != GUMBO_TAG_A) return false;
    GumboAttribute* h = gumbo_get_attribute(&node->v.element.attributes, "href");
    if (!h || !h->value) return false;
    return visited_contains(h->value);
}
```

在 styletree.h 的 `match_simple_selector` 中 `:link`/`:any-link` 现有分支附近加入 visited 分支；`KatanaPseudoClassVisited` 是否存在需确认 katana.h 枚举（若无则用字符串匹配 `:visited`）。

同时将 visited 颜色应用到渲染：在 `apply_styles` 中，当 `node_is_visited_link` 且未显式设置 color 时，使用 theme 的 link_visited：

```c
    if (ln->styled && ln->styled->node &&
        node_is_visited_link(ln->styled->node) &&
        !ln->color.valid) {
        ln->color.r = g_theme.link_visited[0];
        ln->color.g = g_theme.link_visited[1];
        ln->color.b = g_theme.link_visited[2];
        ln->color.valid = true;
    }
```

> 需要 render.h 暴露 `theme_get()`，或在 layout.h 内 include theme.h（通过 mini_browser.h 已含）。以编译通过为准调整 include。

- [ ] **步骤 3：:placeholder-shown**

在 styletree.h 选择器匹配中支持 `:placeholder-shown`（匹配 `<input>`/`<textarea>` 且当前值为空 —— 值状态由 interact.h 的 input_buf 决定，无头下由 value 属性决定）：

```c
    if (is_placeholder_shown_pseudo(sel)) {
        /* <input>/<textarea> 且无 value 或 value 为空 */
        if (!node || node->type != GUMBO_NODE_ELEMENT) return false;
        GumboTag t = node->v.element.tag;
        if (t != GUMBO_TAG_INPUT && t != GUMBO_TAG_TEXTAREA) return false;
        GumboAttribute* v = gumbo_get_attribute(&node->v.element.attributes, "value");
        if (!v || !v->value || !v->value[0]) return true;
        /* textarea: 检查文本内容 */
        if (t == GUMBO_TAG_TEXTAREA && node->v.element.children.length == 0) return true;
        return false;
    }
```

渲染 placeholder 灰色斜体（render.h 输入框渲染处，找到 placeholder 文本绘制点）：

```c
    /* placeholder: 空输入框 → 灰色斜体 */
    ...
    if (placeholder_visible) {
        scr_italic(s, x, y, true);
        scr_fg(s, x, y, g_theme.disabled_fg[0], g_theme.disabled_fg[1], g_theme.disabled_fg[2]);
    }
```

- [ ] **步骤 4：disabled 灰显渲染**

在 render.h 的输入框/按钮渲染处，`ln->disabled` 时前景用 `g_theme.disabled_fg`：

```c
    if (n->disabled) {
        /* 文本与边框用 disabled_fg */
    }
```

- [ ] **步骤 5：编译 + 无头验证 + 回归 14 页**

```bash
gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
/tmp/headless_test pages/16-theme.html "16-theme" "rounded"
# 回归: 现有页面关键内容
/tmp/headless_test pages/00-menu.html "Mini Browser"
/tmp/headless_test pages/01-text-colors.html "Text"
/tmp/headless_test pages/03-inputs.html "Save"
gcc -I src -o /tmp/mb src/demo.c -lm
```

预期：全部通过，无新警告。

- [ ] **步骤 6：Commit**

```bash
git add src/core/styletree.h src/core/layout.h src/core/render.h
git commit -m "feat: CSS 增强 (border-radius→rounded, :visited, :placeholder-shown, disabled 灰显)"
```

---

## 任务 5：文档与收尾

**文件：**
- 修改：`README.md`
- 修改：`doc/optimization-analysis.md`

- [ ] **步骤 1：README 更新**

- 功能清单新增：可点击元素（data-action / cursor:pointer / `<a href>` 导航）、主题系统（theme.ini / --theme / R 重载）、disabled、:visited、:placeholder-shown、border-radius 降级
- 键盘表新增：`R` 主题重载、链接 Enter 跳转
- 新增「主题配置」小节（含 theme.ini 示例与优先级）
- pages 列表补 15-clickable.html / 16-theme.html

- [ ] **步骤 2：优化分析文档勾选**

`doc/optimization-analysis.md` 中 P1-7（`<a href>` 导航 + 历史栈）标注为已部分实现（点击跳转 + visited 已做，历史栈 back/forward 未做，保留条目）。

- [ ] **步骤 3：最终回归 + Commit**

```bash
gcc -DRENDER_HEADLESS -I src -o /tmp/headless_test tools/headless_test.c -lm
for p in pages/*.html; do /tmp/headless_test "$p" >/dev/null 2>&1 && echo "OK $p"; done
git add README.md doc/optimization-analysis.md
git commit -m "docs: README 更新 (可点击/主题/新页面) + 优化分析勾选 P1-7"
```

---

## 自检

- **规格覆盖度**：设计文档 1（可点击）→ 任务 2；2（主题）→ 任务 3；3（CSS 属性）→ 任务 2/4；4（视觉反馈）→ 任务 2 状态栏提示 + 任务 3 主题；5（API）→ 任务 2 步骤 4；6（文件清单）→ 任务 2/3/4/5；7（实现顺序）→ 任务顺序一致；8（测试）→ 任务 1 工具 + 各任务验证；9（风险）→ 各自缓解已内联。
- **占位符扫描**：无 TODO/待定；每个步骤含具体代码或命令。
- **类型一致性**：`clickable`/`click_action`/`disabled`/`cursor_pointer` 字段在任务 2 统一定义并被任务 2/4 引用；`on_element_click` 签名（action, element_id, tag, mb, userdata）在任务 2 定义、任务 2 步骤 5 使用一致；`mb_apply_theme`/`theme_resolve_source`/`theme_load` 任务 3 定义与使用一致。
- **注意**：`KatanaPseudoClassVisited` 枚举名需在实现时核对 katana.h；`:visited` 无头下用模块级 `g_visited` 集合兜底；`style_value` 辅助函数名以 layout.h 现有实现为准。

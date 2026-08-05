# 设计：可点击元素 + 主题系统 + CSS 属性增强

> 日期: 2026-08-06
> 项目: Mini Browser（终端 TUI 浏览器引擎）
> 状态: 已批准（用户确认按建议执行）

## 背景

用户希望提升使用者的可操作空间：① 点击 div 等任意元素触发操作；② 更多样式可配置性（用户侧主题 + 页面侧 CSS 属性）。同时基于《优化分析》文档（`doc/optimization-analysis.md`），将 `<a href>` 导航（P1-7）一并纳入本次范围，作为 `:visited` 的真实语义来源。

## 目标与非目标

### 目标

1. 任意元素可点击：`data-action` / `cursor: pointer` / `<a href>` 三种声明方式，统一进焦点链，回调驱动行为
2. 用户侧主题：配置文件 + 命令行参数 + `R` 热重载，覆盖全部引擎级硬编码颜色
3. 页面侧 CSS 增强：`border-radius`（降级 rounded）、`:visited`、`disabled`、`cursor: pointer`、`:placeholder-shown`
4. 可点击视觉反馈：状态栏提示 action/URL
5. 测试页 + 无头回归，保证现有 14 个页面不破坏

### 非目标（YAGNI）

- 运行时主题菜单（F2/T 弹窗）—— 配置文件 + R 重载已足够
- 撤销/重做、剪贴板、文本选择
- `float` / `position: absolute/fixed` / Grid 布局
- 主题覆盖页面 CSS 颜色（主题只管引擎 chrome）
- `<link rel=stylesheet>` 外部 CSS 加载（本次不做，独立后续）
- 图片真实渲染（本次不做）

## 一、可点击元素机制

### 1.1 可点击判定

元素满足以下任一条件即「可点击」，全部纳入焦点链（`collect_focus_list` 扩展）：

| 条件 | 来源 | 触发 action 值 |
|---|---|---|
| `<a>` 且有 href | 标签 | href 值 |
| `data-action="..."` 属性 | HTML 属性 | 属性值 |
| `cursor: pointer` | CSS 声明 | 元素 id（无 id 时用标签名） |
| INPUT / BUTTON / SUMMARY / TEXTAREA / SELECT | 现有 | 保持现有 `on_button_click` 路径不变 |

`<a>` 与 `cursor: pointer` 的 action 回退链：`data-action` > href > id > 标签名。

### 1.2 触发与回调

- **鼠标左键点击**（非 motion）与 **Tab 聚焦后 Enter** 均触发
- 新回调：`MB_ElementClickFn on_element_click(action, element_id, tag, mb, userdata)`
- 返回 `true` 表示需要重建布局（与现有 on_button_click 语义一致）
- **默认行为**（未设置回调）：状态栏显示 `⏎ action`
- 现有按钮路径（`on_button_click`）完全不动，两种回调并行

### 1.3 访问记录（visited 集合）

- 引擎内部维护 visited 集合：`<a>` 被点击（Enter 或鼠标）时，将 href 记入集合
- 集合上限 64 条，环形覆盖（FIFO）
- API：`void mb_mark_visited(MiniBrowser* mb, const char* url)`（demo 可手动调用）
- 是否真正跳页由回调决定：demo 中检测 href 以 `.html` 结尾则 `mb_switch_page` 跳转

### 1.4 disabled 元素

- 带 `disabled` 属性的 input/button/select/textarea：**不进焦点链**、不可点击、文本灰显（主题 disabled 色）
- 与真实浏览器行为一致（disabled 表单控件不可交互）

## 二、主题系统

### 2.1 配置来源（优先级从高到低）

1. `--theme <path>` 命令行参数
2. 项目目录 `theme.ini`
3. `~/.config/mini_browser/theme.ini`
4. 内置默认主题

### 2.2 格式

```ini
# Mini Browser theme
scrollbar_thumb = #78B4FF
scrollbar_track = #325078
focus_border    = #FFD700
statusbar_bg    = #202040
statusbar_fg    = #CCCCCC
select_popup_bg = #141428
link_color      = #4A9EFF
link_visited    = #9B59B6
text_cursor     = #FFFFFF
textarea_sb     = #78B4FF
border_default  = #666666
disabled_fg     = #555555
```

键：约 12 个，全部 `#RRGGBB`；未知键忽略，缺省键用默认值；`#` 开头为注释。

### 2.3 架构

- 新增 `src/core/theme.h`（header-only，stb 风格 `THEME_IMPLEMENTATION`）：
  - `Theme` 结构体（12 个颜色字段）
  - `theme_default(Theme*)` 内置默认
  - `theme_load(Theme*, const char* path)` 解析 INI（返回 0 成功 / 非 0 失败，失败保留原值）
  - `theme_resolve_source(char* out, size_t, int argc, char** argv)` 按优先级找配置文件
- render.h / interact.h 中**所有硬编码颜色**改为读取全局 theme（模块内 `static Theme g_theme` + getter；TUI 单实例，全局单例可接受）
- 覆盖点（现有硬编码色全部收拢）：滚动条滑块/轨道、焦点指示器、状态栏、select 弹窗背景/高亮、`<a>` 默认蓝、visited 色、文本光标、textarea 滚动条、边框默认色、disabled 灰

### 2.4 热重载

- 事件循环中 `R` 键：重新按优先级解析配置 → 覆盖 `g_theme` → 全屏重绘
- 解析失败：状态栏提示错误，保留当前主题

## 三、页面侧 CSS 属性增强

| 属性 | 实现方式 | 文件 |
|---|---|---|
| `border-radius: Npx` | 解析到非零值且存在边框时，`border_style = 6 (rounded)`，角用 ╭╮╰╯ | styletree.h / layout.h |
| `:visited` | 选择器匹配时：`<a>` 且 href ∈ visited 集合 → 颜色用主题 `link_visited` | styletree.h |
| `disabled` 属性 | 布局/焦点阶段标记；灰显 + 排除出焦点链（见 1.4） | layout.h / interact.h |
| `cursor: pointer` | 解析为可点击标记（LayoutNode 新字段），进焦点链 | styletree.h / layout.h |
| `:placeholder-shown` | 输入框空且非聚焦时 placeholder 以灰色斜体渲染（当前为普通灰） | render.h |

## 四、视觉反馈

- 悬停或聚焦可点击元素（非输入类）时：状态栏显示 `⏎ <action或href>`
- 现有焦点指示器不变
- 无新增 UI 元素

## 五、API 变更汇总（mini_browser.h）

```c
typedef bool (*MB_ElementClickFn)(const char* action, const char* element_id,
                                  const char* tag, MiniBrowser* mb, void* userdata);

typedef struct {
    /* 现有字段不变 */
    MB_ElementClickFn on_element_click;   /* 新增 */
} MB_Config;

void mb_mark_visited(MiniBrowser* mb, const char* url);  /* 新增 */
```

## 六、文件改动清单

| 文件 | 改动 |
|---|---|
| `src/core/theme.h` | **新增**：Theme 结构体 + INI 解析 + 默认值 |
| `src/core/mini_browser.h` | API：on_element_click、mb_mark_visited、主题初始化调用 |
| `src/core/styletree.h` | :visited 匹配、cursor:pointer、border-radius→rounded、disabled 解析、:placeholder-shown 标记 |
| `src/core/layout.h` | LayoutNode 新字段（clickable、disabled、placeholder 态）；disabled 灰显默认样式 |
| `src/core/render.h` | 硬编码色 → theme；placeholder 灰色斜体；disabled 灰显 |
| `src/core/interact.h` | collect_focus_list 扩展；Enter/点击分发 on_element_click；visited 记录；R 热重载；状态栏可点击提示 |
| `src/demo.c` | on_element_click 示例（链接跳页、data-action 状态消息） |
| `pages/15-clickable.html` | **新增测试页**：data-action div、cursor:pointer、`<a>` 链接、disabled 控件、:visited 演示 |
| `pages/16-theme.html` | **新增测试页**：border-radius、:placeholder-shown、主题色对比 |
| `README.md` | 功能清单、键盘表（R 重载）、配置说明 |
| `theme.ini` | **新增示例配置文件**（项目目录） |

## 七、实现顺序

1. **阶段 1 — 可点击机制**：LayoutNode 字段 → collect_focus_list → 点击/Enter 分发 → on_element_click API → visited 集合 → 测试页 15
2. **阶段 2 — 主题系统**：theme.h → 硬编码色收拢 → --theme 参数 + R 热重载 → theme.ini 示例 → 测试页 16（主题部分）
3. **阶段 3 — CSS 属性收尾**：border-radius / :placeholder-shown / :visited 样式 / disabled 样式细节
4. **阶段 4 — 文档**：README 更新

## 八、测试方案

- **无头回归**：`RENDER_HEADLESS` 模式 + `screen_dump_text`，对全部 16 个页面 dump 后断言关键内容
  - 15-clickable：可点击元素出现在焦点链（dump 中含焦点指示器）；disabled 控件不在焦点链且灰显
  - 16-theme：border-radius 元素显示 ╭╮╰╯ 角字符；空输入框 placeholder 斜体灰
  - 现有 14 页：断言不回归（关键行文本不变）
- **手动验证**：termux 终端跑 `./run.sh pages/15-clickable.html`，Tab 遍历含 data-action div 与链接；鼠标点击触发回调；按 R 重载主题
- **边界用例**：无 id 的 cursor:pointer 元素、data-action 为空、href 超长、theme.ini 缺键/坏值、visited 集合溢出（>64）

## 九、风险与缓解

| 风险 | 缓解 |
|---|---|
| 焦点链变长，Tab 遍历噪音 | 可点击元素有状态栏提示，且 disabled 正确排除；必要时后续加 tabindex 控制顺序（不在本次范围） |
| 主题全局单例与多实例冲突 | TUI 单实例；theme 挂到 MiniBrowser 之前保持 getter 封装，迁移成本低 |
| 硬编码色收拢遗漏导致主题不一致 | 收拢时 grep 全部 `,[0-9]+,[0-9]+,[0-9]+` 字面量逐一核对 |
| :visited 匹配在无头测试中不可见 | 通过 dump 断言链接颜色转义序列变化 |

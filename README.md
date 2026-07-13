# Mini Browser — 终端 TUI 浏览器

一个基于终端的迷你浏览器引擎，在终端中渲染 HTML + CSS，支持交互操作（焦点切换、按钮点击、输入框编辑）。

## 快速开始

```bash
# 运行默认页面（测试菜单）
./run.sh

# 指定页面
./run.sh pages/01-complex-form.html
```

或直接使用 tcc 编译运行：

```bash
tcc -I src -run src/demo_main.c [page.html]
```

> 依赖 [tcc](https://bellard.org/tcc/)（Tiny C Compiler），无需 Makefile 或其他构建工具。

## 使用方法

### 页面文件

测试页面位于 `pages/` 目录下，每个 HTML 文件包含内联 `<style>` CSS：

```
pages/
├── 00-menu.html              # 主菜单（默认页面）
├── 01-complex-form.html      # 表单演示：输入框、按钮、选择器、边框
├── 02-flex-layout.html       # Flex 布局展示：对齐、间距、弹性
├── 03-selectors-elements.html# CSS 选择器与元素：伪类、表格、列表
└── 04-new-features.html      # 新功能展示：折叠面板、多行输入、下拉选择、colspan、flex-wrap
```

### 键盘操作

| 按键 | 功能 |
|:--|:--|
| `Tab` | 下一个可聚焦元素（按钮 / 输入框 / select / textarea / summary） |
| `Shift+Tab` | 上一个可聚焦元素 |
| `Enter` | 点击当前聚焦的按钮 / 切换 `<summary>` 折叠 / 循环 `<select>` 选项 |
| `Enter`（textarea） | 输入框中换行 |
| `q` / `Q` / `Esc` | 退出 |
| 任意字符键 | 在输入框中输入文本 |
| `←` / `→` | 输入框中光标左/右移动 |
| `Home` / `End` | 输入框中光标到开头/末尾 |
| `Delete` | 输入框中删除光标处字符 |

> 鼠标点击也支持（终端需支持鼠标事件）。

### 页面跳转

- 按钮 `id` 为 `btn-page-NN` 时，自动跳转到 `pages/NN-*.html`（当前支持 01~04）
- 按钮 `id` 为 `btn-back` 或 `btn-back-N` 时，返回菜单 `00-menu.html`

---

## 运行原理

```
HTML (.html 含 <style>)
    │
    ▼
┌──────────────────┐
│Gumbo (HTML 解析) │  →  DOM 树
└──────────────────┘
    │
    ▼
┌──────────────────┐
│Katana (CSS 解析) │  →  样式表
└──────────────────┘
    │
    ▼
┌──────────────────┐
│  StyleTree       │  →  样式树（选择器匹配 + 属性继承）
└──────────────────┘
    │
    ▼
┌──────────────────┐
│Layout (Box/Flex) │  →  布局树（盒模型 + Flexbox）
└──────────────────┘
    │
    ▼
┌──────────────────┐
│Render (Termbox2) │  → 终端 24-bit 真彩色渲染
└──────────────────┘
    │
    ▼
┌──────────────────┐
│Interact          │  → 交互循环（焦点、点击、输入）
└──────────────────┘
```

### 核心模块

| 模块 | 文件 | 职责 |
|:--|:--|:--|
| **用户页面层** | `src/demo_page.h` | 页面配置、按钮行为注册（用户可编辑） |
| **交互框架** | `src/core/interact.h` | 交互循环、焦点遍历、事件分发 |
| **渲染引擎** | `src/core/render.h` | 屏幕缓冲、节点渲染、边框/表格绘制 |
| **布局引擎** | `src/core/layout.h` | 盒模型、Flexbox 布局计算 |
| **样式引擎** | `src/core/styletree.h` | 样式树构建、选择器匹配、属性继承 |
| **HTML 解析** | `src/core/gumbo.h` | Gumbo HTML 解析器（header-only） |
| **CSS 解析** | `src/core/katana.h` | Katana CSS 解析器（header-only） |
| **终端 I/O** | `src/core/termbox2.h` | 终端输入/输出 |
| **Unicode 工具** | `src/core/uc.h` | UTF-8 编解码支持 |
| **入口** | `src/demo_main.c` | 组装所有模块，调用 `demo_run()` |

所有核心模块均为 **header-only**（通过 `#define XXX_IMPLEMENTATION` 控制实例化）。

---

## 已支持的功能

### CSS 选择器

| 类型 | 示例 | 状态 |
|:--|:--|:--:|
| 类型选择器 | `div`, `p`, `h1` | ✅ |
| 通用选择器 | `*` | ✅ |
| 类选择器 | `.box`, `.foo.bar` | ✅ |
| ID 选择器 | `#header` | ✅ |
| 属性选择器 | `[id]`, `[type="text"]`, `[attr^="val"]`, `[attr$="val"]`, `[attr*="val"]`, `[attr~="val"]`, `[attr\|="val"]` | ✅ |
| 伪类（结构） | `:first-child`, `:last-child`, `:nth-child(n)`, `:nth-last-child`, `:first-of-type`, `:last-of-type`, `:only-of-type`, `:only-child`, `:empty` | ✅ |
| 伪类（链接/交互） | `:link`, `:any-link`, `:hover`, `:focus`, `:active` | ✅ |
| 后代组合器 | `div span` | ✅ |
| 子组合器 | `div > p` | ✅ |
| 相邻兄弟 | `h1 + p` | ✅ |
| 后续兄弟 | `h1 ~ p` | ✅ |

### CSS 属性（有视觉效果）

| 属性 | 说明 |
|:--|:--|
| `display` | `block`, `inline`, `flex`, `grid`（降级为 block）, `table`, `inline-block`（降级为 block）, `none` |
| `position` | ✅ `static`, `relative`（`top`/`left`/`right`/`bottom` 偏移） |
| `width` / `height` | `px`, `%` 单位 |
| `padding` | 简写 1~4 值 |
| `margin` | 简写 1~4 值（透明间距，margin 折叠已修复） |
| `border` | 简写 + `px`，Unicode 框线绘制 |
| `border-style` | `solid`, `dashed`, `dotted`, `double`, `heavy`, `rounded`, `groove`/`ridge`/`inset`/`outset`（后四种降级为 solid） |
| `border-color` | `#RRGGBB`, 命名色 |
| `border-width` | 数值 |
| `color` | 前景色 |
| `background-color` | 背景色填充 |
| `text-align` | `left`, `center`, `right` |
| `font-weight` | `bold`, `700`, `bolder`（终端粗体 ANSI） |
| `text-decoration` | `underline`（终端下划线 ANSI），`overline`（Macron `¯`），`line-through`（删除线 `-`） |
| `flex-direction` | `row`, `column` |
| `flex-wrap` | ✅ 换行支持（`wrap` 自动折行，空间不足时换新行） |
| `justify-content` | `start`, `center`, `end`, `space-between`, `space-around`, `space-evenly` |
| `align-items` | `start`, `center`, `end`, `stretch` |
| `gap` | flex 子元素间距 |
| `flex-grow` | 剩余空间分配 |
| `flex-shrink` | ✅ 超出容器时按比例收缩 |
| `flex-basis` | `px` 单位 |
| `outline` | 边框外轮廓（支持 `width` + `color` 简写） |
| `overflow` | `hidden`, `auto`, `scroll`（递归裁剪子节点） |
| `visibility` | ✅ `hidden`（元素占位不可见） |
| `white-space` | ✅ `pre`/`pre-wrap`/`pre-line`（保留空白） |
| `line-height` | 行高（数值，默认 1） |
| `letter-spacing` | 字符间距（`px` 单位） |
| `word-spacing` | 单词间距（`px` 单位） |
| `box-sizing` | `content-box`（默认），`border-box` |
| `max-width` / `min-width` | 宽度约束（`px` 单位） |
| `max-height` / `min-height` | 高度约束（`px` 单位） |
| `vertical-align` | `top`/`middle`/`bottom` |

### HTML 标签

| 类别 | 标签 | 状态 |
|:--|:--|:--:|
| Block 元素 | `html`, `body`, `div`, `p`, `h1`-`h6`, `ul`, `ol`, `li`, `header`, `footer`, `section`, `article`, `nav`, `main`, `form`, `fieldset`, `legend`, `hr`, `pre`, `details`, `summary`, `textarea`, `select`, `table`, `tr`, `td`, `th` | ✅ 默认 block |
| Inline 元素 | `span`, `a`, `em`, `strong`, `b`, `i`, `u`, `code`, `small`, `br`, `img`, `input`, `button` | ✅ 默认 inline |
| Block 元素 | `select` | ✅ 默认 block（经调整，确保 block 流中正确换行） |
| 折叠面板 | `<details>` / `<summary>` | ✅ 点击 `<summary>` 切换展开/折叠（`open` 属性控制默认状态） |
| 多行输入 | `<textarea>` | ✅ 支持多行文本编辑，Enter 换行 |
| 下拉选择 | `<select>` / `<option>` | ✅ Tab 聚焦，Enter 循环切换选项，`<option>` 隐藏 |
| 输入框 | `<input>` | ✅ 聚焦后键盘输入，支持 `type="password"` 和 `placeholder` |
| 按钮 | `<button>` | ✅ 背景色 + 粗体，可点击 |
| 超链接 | `<a href>` | ✅ 默认蓝色 + 下划线 |
| 图片 | `<img>` | ✅ `[img]` 占位符显示 |
| 表格 | `<table>` / `<tr>` / `<td>` / `<th>` | ✅ 自动列宽 + 等高等列，支持 `colspan` |
| 列表 | `<ul>` / `<ol>` / `<li>` | ✅ `•` 和 `1.` 前缀 |
| 水平线 | `<hr>` | ✅ `─` 字符横线 |
| 预格式化 | `<pre>` | ✅ 保留空白和换行 |
| 脚本/样式 | `<script>`, `<style>` | ✅ 自动跳过/隐藏 |

### 渲染能力

| 功能 | 说明 |
|:--|:--|
| 24-bit 真彩色 | ANSI `\033[38;2;R;G;Bm` / `\033[48;2;R;G;Bm` |
| 粗体 / 下划线 | ANSI 转义序列 |
| 边框绘制 | 6 种样式（solid/dashed/dotted/double/heavy/rounded），Unicode 框线字符 |
| 表格边框交点 | 自动识别 `┼├┤┴┬` 等交叉字符 |
| 文本对齐 | 居中/右对齐 |
| 换行 | `<br>` → 新行 |
| 滚动 | 屏幕滚动支持 |
| 显示隐藏 | `display: none` |
| 内容裁剪 | `overflow: hidden` |
| 文本截断 | 超出宽度显示 `…` |
| 焦点指示器 | 选中元素外周条状高亮 |
| 状态栏 | 底部固定行，交互反馈 |
| 输入框编辑 | 实时键盘输入 |
| UTF-8 支持 | 中文 / Emoji 多字节字符 |

---

## ❌ 不支持的功能

### CSS 选择器

| 选择器 | 说明 |
|:--|:--|
| `:not()` | ✅ 已实现（否定伪类，Katana 已解析，支持嵌套选择器） |
| `::before` / `::after` | ✅ 已实现（`content` 属性生成文本内容，前置或附加到元素文本） |
| `:nth-child(an+b)` 复杂公式 | ✅ 已实现（支持 `3n+1`、`n+2`、`-n+3` 等公式） |

### CSS 布局属性

| 属性 | 说明 |
|:--|:--|
| `float`, `clear` | 未实现，推荐 Flexbox 替代 |
| `position: absolute / fixed` | 未实现，仅支持 `static` 和 `relative` |
| `z-index` | 无层叠上下文，元素顺序依赖 DOM |
| `grid-template-*` 及 Grid 相关 | Grid 降级为 `block` 处理 |
| `flex-wrap` | ✅ 已实现（`wrap` 折行支持，`align-content` 未实现） |
| `flex-shrink` | ✅ 已实现（超出容器时按比例收缩） |
| `max-width` / `min-width` / `max-height` / `min-height` | ✅ 已实现（尺寸约束，仅支持 `px` 单位） |
| `box-sizing` | ✅ 已实现（支持 `border-box`，padding/border 从宽高中扣除） |
| `margin: auto` | ✅ 已实现（水平居中） |
| 负 `margin` | ✅ 已实现（块布局中允许，可产生重叠效果） |
| margin 折叠 | ✅ 已修复（相邻垂直 margin 取最大值） |

### CSS 视觉样式

| 属性 | 说明 |
|:--|:--|
| `vertical-align` | ✅ 已实现（`top`/`middle`/`bottom`） |
| `opacity`, `box-shadow`, `border-radius` | 终端无法渲染透明、阴影、圆角 |
| `transform`, `transition`, `animation` | 终端渲染无实际意义 |
| 字体相关：`font-family`, `font-size`, `font-style` | 仅支持 `font-weight` 粗体 |
| `text-decoration` 的 `overline` / `line-through` | ✅ 已实现（Macron `¯` / 删除线 `-`） |
| `line-height` | ✅ 已实现（支持数值，行距可调） |
| `text-transform` | ✅ 已实现（`uppercase`/`lowercase`/`capitalize`） |
| `letter-spacing`, `word-spacing` | ✅ 已实现（字符间距 + 单词间距，`px` 单位） |
| `white-space` | ✅ 已实现（`pre`/`pre-wrap`/`pre-line` 保留空白） |
| `visibility` | ✅ 已实现（`hidden` 占位不可见） |
| `outline` | ✅ 已实现（`outline-width` + `outline-color`，简写 `outline`） |
| `cursor` | 终端光标样式不可控 |

### 颜色格式

| 格式 | 说明 |
|:--|:--|
| `rgb(r,g,b)` / `rgba(r,g,b,a)` | ✅ 已实现（支持函数式 RGB，含 `%` 单位） |
| `hsl()` / `hsla()` | ✅ 已实现（HSL→RGB 转换） |
| `transparent` 关键字 | ✅ 已实现（`transparent` → 无色） |
| `currentColor` 关键字 | ✅ 已实现（解析为当前 `color` 值） |

### 渲染与盒模型

| 功能 | 说明 |
|:--|:--|
| `display: inline-block` | 该中间模式未实现（降级为 block 处理） |
| 独立滚动容器（`overflow: auto / scroll`） | ✅ 已实现（`overflow: auto/scroll` 裁剪+状态栏滚动指示 `▲▼`） |
| 层叠顺序 | 重叠元素仅按 DOM 顺序，无真正层叠 |
| 终端 resize 信号 | ✅ SIGWINCH 信号处理 + 事件轮询双检测 |
| 文本选中/复制 | 终端 TUI 无选择复制机制 |
| `<img>` 的 `alt` 属性 | ✅ 已实现（读取 `alt` 属性，显示 `[alt文本]`） |

### HTML 标签/元素

| 标签 | 说明 |
|:--|:--|
| `<iframe>`, `<video>`, `<audio>`, `<canvas>` | 嵌入/多媒体/绘图元素不支持 |
| `<input type="radio/checkbox/email/password/number/range/date">` 等 | 仅支持 `type="text"` 和 `type="password"`（`•` 掩码），支持 `placeholder` 属性 |
| 折叠面板 | `<details>` / `<summary>` | ✅ 已实现（Tab+Enter 切换展开/折叠，`open` 属性控制默认状态） |
| 多行输入 | `<textarea>` | ✅ 已实现（Enter 换行，多行编辑） |
| 下拉选择 | `<select>` / `<option>` | ✅ 已实现（Tab 聚焦，Enter 循环选项） |
| 语义标签 | `<dialog>`, `<figure>` 等 | 不支持 |
| `<label for>` | ✅ 已实现（点击 `<label for="id">` 自动聚焦目标输入框） |
| `<fieldset>`, `<legend>` | 已解析（默认 block 渲染，legend 自动放到 fieldset 顶部） |
| `<optgroup>` | 不支持 |

### 表格

| 特性 | 说明 |
|:--|:--|
| `colspan` | ✅ 已实现（跨列合并，单元格自动均分列宽） |
| `rowspan` | 跨行合并未实现 |
| `<thead>` / `<tbody>` / `<tfoot>` | 表格行组已解析（统一按 `display: table-row` 处理） |
| `border-collapse` | 始终使用 collapsed 网格绘制模式 |

### CSS 其他

| 特性 | 说明 |
|:--|:--|
| `@media` 查询 | ✅ 基础支持（`min-width`/`max-width`，匹配 `screen`/`all` 类型） |
| `background` 简写（除 `background-color`）| 仅支持纯色背景填充，`background-image` 等不支持 |
| `border` 部分样式（`groove` / `ridge` / `inset` / `outset`）| 降级为 `solid` 绘制 |
| `@font-face` | 字体定义无意义 |
| `@keyframes` / `animation` | 动画不支持 |
| `inherit` / `initial` / `unset` 关键字 | ✅ 已实现（`inherit` 从父层继承，`initial`/`unset` 恢复默认） |

### 交互

| 功能 | 说明 |
|:--|:--|
| `Shift+Tab` | ✅ 已实现（`TB_KEY_BACK_TAB`，反向遍历焦点） |
| `<details>` 折叠 | ✅ 聚焦 `<summary>` 后按 Enter/Space 切换展开/折叠 |
| `<textarea>` 编辑 | ✅ 多行文本编辑，Enter 换行，方向键导航 |
| `<select>` 选择 | ✅ 聚焦后按 Enter/Space 循环切换选项 |
| 鼠标悬停 `:hover` | ✅ 键盘输入自动清除悬停，鼠标离开检测 |
| 终端 Resize 信号 | ✅ SIGWINCH 信号处理 + 事件轮询双检测 |
| 键盘/鼠标事件扩展 | 仅支持焦点切换、点击、输入，无拖拽、右键菜单等 |
| 输入框光标位置 | ✅ 已实现（`←`/`→`/`Home`/`End` 移动，`Delete`/`Backspace` 删除） |

---

## 项目结构

```
├── run.sh                        # 运行脚本
├── pages/                        # 测试页面（HTML + 内联 CSS）
├── src/
│   ├── demo_main.c               # 入口：组装模块并启动
│   ├── demo_page.h               # 用户页面配置（可编辑）
│   └── core/
│       ├── gumbo.h               # HTML 解析器
│       ├── interact.h            # 交互框架
│       ├── katana.h              # CSS 解析器
│       ├── layout.h              # 布局引擎
│       ├── render.h              # 渲染引擎
│       ├── styletree.h           # 样式引擎
│       ├── termbox2.h            # 终端 I/O
│       └── uc.h                  # Unicode 工具
├── doc/
│   ├── 实现状态.md               # 详细的功能实现状态
│   ├── 解耦方案.md               # 架构解耦设计文档
│   └── test_pages/               # 历史测试页面 (已迁移到 pages/)
└── README.md
```

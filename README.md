# Mini Browser — 终端 TUI 浏览器

一个基于终端的迷你浏览器引擎，在终端中渲染 HTML + CSS，支持交互操作（焦点切换、按钮点击、输入框编辑）。

## 快速开始

```bash
# 运行默认页面（测试菜单）
./run.sh

# 指定页面
./run.sh doc/test_pages/01-complex-form.html
```

或直接使用 tcc 编译运行：

```bash
tcc -I src -run src/demo_main.c [page.html]
```

> 依赖 [tcc](https://bellard.org/tcc/)（Tiny C Compiler），无需 Makefile 或其他构建工具。

## 使用方法

### 页面文件

测试页面位于 `doc/test_pages/` 目录下，每个 HTML 文件包含内联 `<style>` CSS：

```
doc/test_pages/
├── 00-menu.html              # 主菜单（默认页面）
├── 01-complex-form.html      # 表单演示：输入框、按钮、选择器、边框
├── 02-flex-layout.html       # Flex 布局展示：对齐、间距、弹性
└── 03-selectors-elements.html# CSS 选择器与元素：伪类、表格、列表
```

### 键盘操作

| 按键 | 功能 |
|:--|:--|
| `Tab` | 下一个可聚焦元素（按钮 / 输入框） |
| `Shift+Tab` | 上一个可聚焦元素 |
| `Enter` | 点击当前聚焦的按钮 |
| `q` / `Q` / `Esc` | 退出 |
| 任意字符键 | 在输入框中输入文本 |

> 鼠标点击也支持（终端需支持鼠标事件）。

### 页面跳转

- 按钮 `id` 为 `btn-page-NN` 时，自动跳转到 `doc/test_pages/NN-*.html`
- 按钮 `id` 为 `btn-back` 或 `btn-back-N` 时，返回菜单 `00-menu.html`

---

## 运行原理

```
HTML (.html 含 <style>)
    │
    ▼
┌──────────────────┐
│  Gumbo (HTML 解析) │  →  DOM 树
└──────────────────┘
    │
    ▼
┌──────────────────┐
│  Katana (CSS 解析)  │  →  样式表
└──────────────────┘
    │
    ▼
┌──────────────────┐
│  StyleTree         │  →  样式树（选择器匹配 + 属性继承）
└──────────────────┘
    │
    ▼
┌──────────────────┐
│  Layout (Box/Flex) │  →  布局树（盒模型 + Flexbox）
└──────────────────┘
    │
    ▼
┌──────────────────┐
│  Render (Termbox2) │  → 终端 24-bit 真彩色渲染
└──────────────────┘
    │
    ▼
┌──────────────────┐
│  Interact          │  → 交互循环（焦点、点击、输入）
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
| `display` | `block`, `inline`, `flex`, `grid`（降级为 block）, `table`, `none` |
| `width` / `height` | `px`, `%` 单位 |
| `padding` | 简写 1~4 值 |
| `margin` | 简写 1~4 值（透明间距） |
| `border` | 简写 + `px`，Unicode 框线绘制 |
| `border-style` | `solid`, `dashed`, `dotted`, `double`, `heavy`, `rounded`, `groove`/`ridge`/`inset`/`outset`（后四种降级为 solid） |
| `border-color` | `#RRGGBB`, 命名色 |
| `border-width` | 数值 |
| `color` | 前景色 |
| `background-color` | 背景色填充 |
| `text-align` | `left`, `center`, `right` |
| `font-weight` | `bold`, `700`, `bolder`（终端粗体 ANSI） |
| `text-decoration` | `underline`（终端下划线 ANSI） |
| `flex-direction` | `row`, `column` |
| `justify-content` | `start`, `center`, `end`, `space-between`, `space-around`, `space-evenly` |
| `align-items` | `start`, `center`, `end`, `stretch` |
| `gap` | flex 子元素间距 |
| `flex-grow` | 剩余空间分配 |
| `flex-basis` | `px` 单位 |
| `overflow` | `hidden`（递归裁剪子节点） |

### HTML 标签

| 类别 | 标签 | 状态 |
|:--|:--|:--:|
| Block 元素 | `html`, `body`, `div`, `p`, `h1`-`h6`, `ul`, `ol`, `li`, `header`, `footer`, `section`, `article`, `nav`, `main`, `form`, `hr`, `pre`, `table`, `tr`, `td`, `th` | ✅ 默认 block |
| Inline 元素 | `span`, `a`, `em`, `strong`, `b`, `i`, `u`, `code`, `small`, `br`, `img`, `input`, `button` | ✅ 默认 inline |
| 输入框 | `<input>` | ✅ 聚焦后键盘输入 |
| 按钮 | `<button>` | ✅ 背景色 + 粗体，可点击 |
| 超链接 | `<a href>` | ✅ 默认蓝色 + 下划线 |
| 图片 | `<img>` | ✅ `[img]` 占位符显示 |
| 表格 | `<table>` / `<tr>` / `<td>` | ✅ 自动列宽 + 等高等列 |
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
| `:not()` | 否定伪类（katana 已解析但未参与匹配） |
| `::before` / `::after` | 伪元素（katana 已解析但未生成内容） |

### CSS 属性

| 属性 | 原因 |
|:--|:--|
| `float`, `clear` | Flexbox 可替代 |
| `position: absolute/fixed` | 需层叠上下文支持 |
| `z-index` | 子元素天然覆盖父元素 |
| `transform`, `transition`, `animation` | 终端无意义 |
| `vertical-align` | `align-items` 已足够 |
| `opacity`, `box-shadow`, `border-radius` | 终端不支持 |
| `grid-template-*` | grid 降级为 block |

### 渲染

| 功能 | 说明 |
|:--|:--|
| `display: inline-block` | 未实现该中间模式 |
| 重叠元素层叠 | 仅依赖 DOM 顺序 |
| 交互重排内存泄漏 | 伪类触发 layout rebuild 时，旧 tree 未 free |

---

## 项目结构

```
├── run.sh                        # 运行脚本
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
│   └── test_pages/               # 测试页面（HTML + 内联 CSS）
└── README.md
```

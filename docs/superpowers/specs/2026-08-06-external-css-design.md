# 外部 CSS 加载（`<link rel=stylesheet>`）设计

> 日期: 2026-08-06
> 对应优化文档 P1-8（第 3 轮：从 demo 引擎到可用的终端浏览器）

## 目标

支持 `<link rel="stylesheet" href="X">`：解析本地 CSS 文件并合并进样式表，使真实页面（几乎必有外部样式表）能正确渲染。

## 设计决策

1. **识别范围**：仅 `<link rel="stylesheet" href="X">`（rel 大小写不敏感，href 支持单/双引号）。其他 `<link>`（icon/favicon/font 等）忽略。`<style>` 内联提取逻辑不变。
2. **路径解析**：href 相对**当前 HTML 文件所在目录**（`dirname(current_path) + href`）。TUI 本地无网络，仅文件读取，不做路径限制。
3. **层叠顺序**：按文档出现顺序把外部 CSS 内容**插入 css_buf 对应位置**（link 与 style 交替时保持真实层叠序，后出现者覆盖）。css_buf 仍是单一 Katana 样式表，下游管线（katana_parse → build_style_tree → layout）零改动。
4. **错误处理**：文件读取失败 → stderr 警告 + 跳过该 link，**不崩溃**（真实页面常有缺失资源）。

## 实现位置

- `src/core/mini_browser.h` `mb_extract_css`：状态机在 `MB_EXTRACT_TEXT` 态识别 `<link>` 标签，解析 rel/href，读取文件插入 css。需要传 `base_dir` 参数（函数签名从 `(raw, css, cs, html, hs)` 增加 `base_dir`）。
- `tools/headless_test.c` `extract_css`：独立复制版，同步支持（保持测试与运行时一致）。

## 测试

- 新页面 `pages/20-external-css.html`：`<link rel="stylesheet" href="20-ext.css">` + `.ext-box` 元素 + 与内联 style 的层叠顺序测试（link 在 style 前/后各一例）。
- 新文件 `pages/20-ext.css`：定义独特背景色/边框。
- `run_tests.sh`：headless 断言外部样式生效（bg/边框在布局 dump 中）；link 缺失文件页面不崩溃断言。
- 回归：现有 19 页无 `<link>`，行为零变化（全套件 49 项需保持通过）。

## 范围外（YAGNI）

- 网络 URL（`http(s)://`）加载
- `<link>` 的 media 属性（如 `media="print"`）
- CSS `@import`
- 多页面共享样式缓存

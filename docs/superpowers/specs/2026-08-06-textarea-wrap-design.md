# textarea 自动换行设计

> 日期: 2026-08-06

## 目标

textarea 长行文本自动软换行显示（真实 textarea 默认 wrap=soft），不再截断超宽字符。

## 现状

- `render_preserved_ws`（render.h）对 textarea 超宽字符 `continue`（丢弃）
- 滚动 `scroll_skip` / wheel 的 `total_lines` 按硬换行（`\n`）计算
- 文本存储/编辑按硬行 + 列语义（不动）

## 设计决策

1. **渲染软换行**：`render_preserved_ws` textarea 分支——字符超宽（`render_cx >= width`）时 `cx=lnx, cy+=lh` **换行继续绘制**（不丢字符）；`\n` 仍是硬换行。
2. **显示行数一致性**：滚动 `scroll_skip` 从"跳过 N 个硬行"改为"跳过 N 个**显示行**"——渲染时维护 `disp_row` 计数（硬/软换行都 +1），`disp_row < scroll_skip` 的行跳过不绘制。wheel 的 `total_lines`（max_scroll 边界）按显示行数计算：每个硬行段的软换行额外行数 = `段文本宽 ÷ textarea 宽`。
3. **编辑兼容**：文本存储/光标/插入逻辑**不变**（硬行语义）；软换行仅影响显示。**已知限制**：光标在软换行文本中的显示位置可能错位（逻辑硬行 ↔ 显示行映射未实现，文档记录）。

## 实现位置

- `src/core/render.h`：`render_preserved_ws` textarea 软换行 + disp_row 滚动
- `src/core/interact.h`：wheel 的 `total_lines` 显示行数计算

## 测试

- 新页 `pages/27-textarea-wrap.html`：宽 10 的 textarea + 16 字符无换行文本 → 显示 2 行
- headless：`--expect-line` 断言第 2 显示行内容可见
- interact：wheel 滚动到长行第 2 显示行（`--expect-scroll`/文本断言）；现有 textarea 编辑/滚动断言回归

## 范围外

- `wrap="off"` 属性（横向滚动模式）
- 软换行光标精确定位
- `white-space: nowrap` 变体

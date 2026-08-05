# CSS Grid 布局设计（display:grid 务实子集）

> 日期: 2026-08-06

## 目标

`display: grid` + `grid-template-columns` 基础列布局：子元素按行填充，支持固定列、`1fr` 均分列、gap。真实页面常用布局能力（相对回退 block 的现状）。

## 现状

- `DISPLAY_GRID` 枚举已存在（layout.h:21），但 `compute_child_layouts` 的 `case DISPLAY_GRID` 回退 `layout_block_children`（layout.h:2070）
- flex 有完整实现（layout_flex_children）可参照；`gap` 字段 flex 已用，grid 复用

## 设计决策（务实子集）

1. **解析**：`display: grid` → 现有 DISPLAY_GRID；`grid-template-columns` 解析为 `grid_cols_def[16]`（`int`：正值=固定 cell 宽，`-1`=1fr 均分剩余，`-2`=auto 按内容——简化 auto 按 fr 处理）；`gap` 复用 flex gap 字段。无列定义 → 单列。
2. **布局函数** `layout_grid_children(parent, content_w, content_h)`：
   - 第一遍：固定列求和 + gap → 剩余宽度 / fr 列数 → 各列宽（`col_w[16]`）
   - 第二遍：可见子元素（跳过 absolute/fixed/display:none）**按行填充**——x = 列起点（前列宽和 + gap），y = 行 y_cursor，width = 所在列宽；行高 = 该行最高子元素，列满换行（y += row_h + gap）
   - 子元素 `compute_child_layouts(child, child->width)` 递归（宽=列宽）
3. **渲染**：零改动——grid 容器内子元素是普通布局节点，现有渲染/交互/z-index/滚动全复用。
4. **测试**：新页 `pages/26-grid.html`（固定列 + 1fr 列 + gap + 6 item → 2 行×3 列）→ headless 断言各 item abs 坐标（列对齐、换行、列宽）与 gap 间距。
5. **范围外**：`grid-template-rows`、`grid-area`/`grid-column` 显式放置、`span`、`repeat()`、百分比列、嵌套 grid 特殊处理（自然递归可用）。

## 实现位置

- `src/core/layout.h`：`grid_cols_def[16]`/`grid_col_count` 字段 + `grid-template-columns` 解析 + `layout_grid_children` + `compute_child_layouts` case
- `tools/headless_test.c`：布局 dump 加 grid 列标记（`grid=N`，便于断言）

## 测试

- `pages/26-grid.html`：`grid-template-columns: 30 20 1fr; gap: 1`，6 个 item
- headless：item abs x 断言（0、30+1=31、剩余列起点）、y 换行断言（第 2 行 = row_h + gap）
- 回归：现有 72 项不受影响（无 grid 页面，grid 从 block 回退改为真布局——行为变化但无现有依赖）

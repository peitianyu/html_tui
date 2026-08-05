# position: absolute 定位设计

> 日期: 2026-08-06
> 用户选定方向 A（替代 select popup 硬编码绘制的第一步）

## 目标

支持 `position: absolute`：元素脱离文档流，相对最近 positioned 祖先（或视口）按 top/right/bottom/left 定位，z-index 控制层叠。

## 现状

- `position_type` 只支持 0=static / 1=relative（layout.h:1137）
- top/right/bottom/left/z-index 属性已解析为 `position_top` 等字段
- `apply_position_offset` 只对 relative 生效
- select popup 用硬编码 `calc_select_popup_rect` 绘制（本次不重构，范围外）

## 设计决策

1. **解析**：`position: absolute` → `position_type = 2`。新增 4 个显式标记 `pos_left_set/top_set/right_set/bottom_set`（区分「未设置」与「显式 0」——`right: 0` 必须触发右对齐）。
2. **脱离文档流**：`layout_block_children` / `layout_flex_children` 子元素循环跳过 `position_type == 2`（不占流内位置、不影响兄弟）；absolute 元素自身子树照常布局（`compute_child_layouts` 正常递归）。
3. **定位 pass**（`position_absolute_pass`，build_layout_tree 末尾、compute_abs_coords 之后，DFS 父先于子保证 containing block 已定位）：
   - containing block = 最近 `position_type != 0` 祖先的 content 盒；无则根（视口）
   - `left` 设置 → x = cb_x + left；否则 `right` → x = cb_x + cb_w − w − right；否则默认 x = cb_x
   - `top` 设置 → y = cb_y + top；否则 `bottom` → y = cb_y + cb_h − h − bottom；否则默认 y = cb_y
   - 写回 `x/y = target − 直接父的 abs`（渲染时 `screen_render_offset` 父坐标累加自动正确），子树 abs 坐标用 `set_abs_and_children` 重算
4. **z-index**：absolute 是 positioned 元素，现有 z 排序逻辑（render.h `has_z` 分支）直接生效——浮层天然盖住流内内容。
5. **范围外**：`fixed`（依赖视口滚动机制）、`sticky`、`float`、`inset` 简写、百分比 top/left、absolute 撑开父容器、select popup 重构。

## 实现位置

- `src/core/layout.h`：解析（position_type=2 + pos_*_set 标记）、布局跳过、`position_absolute_pass` + `set_abs_and_children`、build_layout_tree 末尾调用

## 测试

- 新页 `pages/22-position.html`：
  - 视口 absolute（top/left）浮层
  - 祖先 relative 容器内的 absolute（top/left）
  - `right`/`bottom` 定位（右/下对齐）
  - z-index 覆盖（absolute 盖住流内元素）
- headless 断言布局 dump `abs=(x,y)` 坐标
- 回归：现有 56 项不受影响（relative 行为不变）

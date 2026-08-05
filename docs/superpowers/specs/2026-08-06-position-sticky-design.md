# position: sticky 定位设计

> 日期: 2026-08-06
> position 系列收尾（0 static / 1 relative / 2 absolute / 3 fixed / 4 sticky）

## 目标

`position: sticky`：滚动到阈值前按流内定位（占位），越过 `top` 阈值后粘在视口（吸顶导航）。

## 设计决策

1. **解析**：`position: sticky` → `position_type = 4`。
2. **不脱离文档流**：sticky 保留流内位置（占位，区别于 absolute/fixed）——布局跳过条件 `position_type >= 2` 改为 `== 2 || == 3`（layout_block_children 分支、flex 6 处跳过 + ac 内容布局分支、shrink_absolute、position_absolute_pass 收集条件共 4 处；sticky=4 全部排除）；定位 pass 不动 sticky（无需定位）。
3. **渲染粘性**：`render_child_offset` 加 sticky 分支——
   `sticky_off = max(0, top − (abs_y − scroll_y))`，`sticky_off > 0` 时临时 `child->y += sticky_off` 再递归（子树跟随），完事恢复。
   - 视口位置 `abs_y − scroll_y >= top`：正常滚动
   - 视口位置 `< top`：固定到视口 `top` 行（粘住）
4. **top/bottom**：先支持 `top`（吸顶最常见）；`bottom` 同公式反向（`sticky_off = max(0, bottom − (视口底 − abs_y − h))`），一并实现（对称）。

## 实现位置

- `src/core/layout.h`：解析（position_type=4）、布局跳过条件收紧
- `src/core/render.h`：`render_child_offset` sticky 分支

## 测试

- 新页 `pages/24-sticky.html`：前置内容 → sticky 条（`position:sticky; top:0`）→ 后置长内容
- `--scroll=X,Y` 断言（文本行位置）：
  - scroll=0：sticky 在流内行
  - scroll 未达阈值：正常滚动（行 = abs − scroll）
  - scroll 越过阈值：**sticky 文本固定在第 0 行**（top:0），内容继续滚动
- 回归：现有 62 项不受影响（absolute/fixed 行为不变，sticky 不参与定位/收缩）

## 范围外（YAGNI）

- 多 sticky 元素同容器交互（后一个顶前一个）
- sticky 容器边界（overflow 容器内的 sticky）
- 百分比 top

# position: fixed 定位设计

> 日期: 2026-08-06
> 复用 position:absolute 基础设施的增量

## 目标

支持 `position: fixed`：滚动时固定在视口的元素（导航条/页脚/浮层），滚动内容时不动。

## 设计决策（复用 absolute 基础设施）

1. **解析**：`position: fixed` → `position_type = 3`（0 static / 1 relative / 2 absolute / 3 fixed）。
2. **脱离文档流 + shrink**：与 absolute 完全一致——布局跳过条件 `position_type == 2` 统一改为 `>= 2`（block 分支 + flex 6 处跳过 + flex absolute 内容布局分支）；`shrink_absolute` 同样 `>= 2`（无显式 CSS width → shrink-to-fit）。
3. **定位**：fixed 的 containing block **始终是视口（根）**，无视 positioned 祖先——`position_absolute_pass` 里 `position_type == 3` 时直接 `cb = root`（absolute 仍找最近 positioned 祖先）。
4. **滚动固定（核心）**：渲染时 `screen_render_offset` 子节点循环识别 `position_type == 3` 的子节点，临时将 `s->scroll_x/scroll_y` 置 0 再递归（完事恢复）——fixed 子树不随滚动平移；z-index 排序逻辑不变（fixed 是 positioned）。

## 实现位置

- `src/core/layout.h`：解析（position_type=3）、布局跳过/shrink 条件 `>= 2`、定位 pass fixed 分支
- `src/core/render.h`：`screen_render_offset` 子循环 fixed 临时 scroll=0

## 测试

- 新页 `pages/23-fixed.html`：顶部 fixed 导航条（`top:0 left:0`）+ 长内容
- `tools/headless_test.c` 加 `--scroll=X,Y` 参数（渲染前设置 `s->scroll_x/y`）→ 断言：
  - fixed 条 abs 坐标不随 scroll 变（仍在视口顶部）
  - 流内内容 abs 随 scroll 偏移
- 回归：现有 58 项不受影响（absolute 行为不变）

## 范围外（YAGNI）

- `sticky`（relative+fixed 混合语义）
- fixed 在视口外的 clip 边界（滚动到极端时的裁剪）
- 百分比 top/left

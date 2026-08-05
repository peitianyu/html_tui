# overflow:auto 子容器滚动设计

> 日期: 2026-08-06

## 目标

固定高度容器内独立滚动（代码查看器/聊天窗口/表格区）：内容溢出时容器内滚动，滚轮悬停其上滚容器内容，到边界才滚全局。

## 现状

- `overflow: hidden` 已有（layout.h overflow_hidden + 渲染 my_clip 裁剪）
- textarea 有独立滚动（`LayoutNode.content_scroll_y` + interact 按 focus 索引的状态数组 + wheel 特判）

## 设计决策

1. **解析**：`overflow: auto|scroll` → `overflow_hidden=true` + 新标记 `overflow_auto=true`（`visible` 不变；`hidden` 仅裁剪不可滚）。
2. **布局**：容器 `height` 固定（CSS），内容照常布局、溢出被现有 my_clip 裁剪。
3. **渲染**：`screen_render_offset` 处理 overflow_auto 容器时，递归子节点前临时 `s->scroll_y += container->content_scroll_y`（完事恢复，同 fixed 的 scroll=0 模式）——内容按容器滚动偏移显示，clip 不变。
4. **交互**：wheel 先查鼠标下最深节点的 overflow_auto 祖先（`find_deepest_node_at` + 向上找 + `find_layout_by_node`）→ 滚其 `content_scroll_y`（边界 = 内容底 − 容器底，≤0 回退滚全局）；**状态持久化**：容器 scroll 按 id 存 `g_scrollbox[16]{id,y}` 映射，restyle 重建布局树后按 id 恢复（textarea 按 focus 索引，子容器无焦点需按 id）。
5. **范围外**：横向容器滚动、容器滚动条绘制、嵌套可滚动容器、overflow:auto 撑开弹性布局。

## 实现位置

- `src/core/layout.h`：`overflow_auto` 标记 + `content_scroll_y`（已存在则复用）
- `src/core/render.h`：`screen_render_offset` 容器子树 scroll 偏移
- `src/core/interact.h`：wheel 路由 + `g_scrollbox` 映射 + restyle 恢复
- `tools/interact_test.c`：`--expect-scroll=class|N` 断言

## 测试

- 新页 `pages/25-scrollbox.html`：固定高（5 行）容器 + 20 行内容 + `overflow:auto`
- headless：容器 height 固定、布局正常
- interact：wheel 在容器上 → `content_scroll_y` 增加（`--expect-scroll`）；滚到边界再 wheel → 全局 scroll 变化；restyle（key:R）后容器 scroll 保留
- 回归：现有 67 项不受影响（textarea 滚动逻辑不动）

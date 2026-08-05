# 历史栈（b 返回上一页）设计

> 日期: 2026-08-06
> 对应优化文档 P1-7 未实现部分（第 3 轮：从 demo 引擎到可用的终端浏览器）

## 目标

`<a href>` 点击跳页后可返回上一页（`b` 键），形成基本的页面导航体验。

## 现状

- `<a href>` 可聚焦 + Enter/点击触发 `on_element_click` 回调 + `:visited` 变色 ✅（已实现）
- demo.c 链接跳页用 `mb_switch_page` → 无历史，无法返回 ⬜（本设计实现）

## 设计决策

1. **数据**：`MiniBrowser` 结构体加 `char history[MB_HIST_MAX][256]` + `int hist_idx`（MB_HIST_MAX=32 固定深，TUI 够用）。`mini_browser_open` 时 `history[0]` = 首屏路径，`hist_idx=0`。状态归入浏览器实例（符合 P1-6「状态应移入实例」方向）。
2. **API 语义**：
   - `mb_navigate(mb, path)`：`hist_idx++` 存**当前页** `current_path` 进 `history[hist_idx]`（超过 32 停止），再 `mb_switch_page(path)`。跳转后再次导航天然截断 forward 分支（覆盖写入）。
   - `mb_go_back(mb)`：`hist_idx > 0` 时 `hist_idx--` → `mb_switch_page(history[hist_idx])`；`hist_idx == 0` 时仅状态栏提示「Already at first page」，不动作。
   - `mb_switch_page` **保持原语义不 push**：`btn-back` 返回菜单等非导航跳转不污染历史。
3. **按键**：`mb_default_on_key` 加 `b`/`B` → `mb_go_back`（在 Esc 处理前；用户自定义 `on_key` 仍优先，返回 false 才到默认）。
4. **接入**：demo.c `demo_on_element_click` 链接跳转分支改用 `mb_navigate`；`tools/interact_test.c` 同步改用，使测试覆盖历史路径。

## 数据流

```
<a href> 点击 → mb_navigate(action) → history[hist_idx+1]=current_path, hist_idx++
                                    → mb_switch_page(action) → loop 加载新页, current_path=action
b 键 → mb_default_on_key → mb_go_back → hist_idx-- → mb_switch_page(history[hist_idx]) → loop 加载
返回后再跳转 → mb_navigate 覆盖 history[hist_idx+1] (截断 forward)
```

## 测试

- interact: 15-clickable → `tab tab tab tab enter`（跳到 01-text-colors）→ `b` → 断言最终状态栏含 `15-clickable.html`
- interact: 15-clickable → `b`（无历史）→ 断言提示
- 回归：现有 51 项不受影响（btn-back 用 `mb_switch_page` 不 push）

## 范围外（YAGNI）

- forward 导航（f/→）
- 地址栏 / 手动 URL 输入
- 页面内 hash 锚点（`#section`）
- 历史条数上限之外的 LRU/淘汰策略（32 深足够）

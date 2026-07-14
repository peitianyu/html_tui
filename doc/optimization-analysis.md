# mini_test_c 源码优化分析报告

> 分析日期: 2026-07-14
> 分析范围: 除 gumbo.h, katana.h, termbox2.h 三个第三方库以外的所有源文件

---

## 总览

| 文件 | 行数(约) | 职责 | 优化等级 |
|------|---------|------|---------|
| `src/demo.c` | 35 | 入口 | ⭐ 低 |
| `src/core/styletree.h` | ~550 | 样式树/选择器匹配 | ⭐⭐⭐ 高 |
| `src/core/layout.h` | ~1800 | 布局引擎 | ⭐⭐⭐ 高 |
| `src/core/render.h` | ~550 | 渲染引擎 | ⭐⭐ 中 |
| `src/core/interact.h` | ~1200 | 交互框架 | ⭐⭐⭐ 高 |
| `src/core/uc.h` | ~300 | UTF-8 工具库 | ⭐ 低 |

---

## 一、死代码 / 未使用代码

### 1.1 `uc.h` — `find()` 函数从未被调用

`find()` 是一个静态二分搜索函数，但 `uc_wid()` 中实现了内联的二分搜索，从未使用 `find()`。

```c
// uc.h: ~150行 — 死代码
static int find(int c, int tab[][2], int n) {
    // ... 从未被任何地方调用
}
```

**建议**: 删除 `find()` 或让 `uc_wid()` 使用它。

### 1.2 `layout.h` — `flex_wrap` 的 `line_starts` 数组分配后未使用

```c
// layout.h: layout_flex_children() 中
int* line_starts = (int*)calloc(visible + 1, sizeof(int));
line_starts[line_count++] = 0;
// ... line_starts 被赋值但从未被读取
```

**建议**: 删除 `line_starts` 分配或实现完整的 flex-wrap 换行后对齐逻辑。

### 1.3 `styletree.h` — `compute_specificity` 中 default 分支与 tag 分支重复

```c
case KatanaSelectorMatchTag:
case KatanaSelectorMatchPseudoElement:
    c++;
    break;
default:
    c++;  // 与上面重复
    break;
```

**建议**: 移除 `default: c++`，改为 `default: break;`。

---

## 二、性能优化

### 2.1 `layout.h` — `parse_color()` 命名颜色用链式 strcmp

17 种命名颜色使用链式 `strcmp`，每次颜色解析最多需要 17 次字符串比较。

**建议**: 使用排序数组 + 二分搜索，或编译期生成 perfect hash/gperf 跳转表。对于少量颜色，可将 `strcmp` 改为首字符 switch（如按首字母 'r'→red, 'g'→green/gray, 'b'→blue/black 等分派）。

### 2.2 `layout.h` — `get_default_display()` 链式 strcmp

30+ 个标签名使用链式 `strcmp` 逐一比较。每次调用可能遍历整个链。

**建议**: 
- 使用排序数组 + 二分搜索（标签名是固定的）
- 或使用首字符分派（绝大多数标签首字符不同: d→div, p→p, h→h1-h6, s→span/section 等）

### 2.3 `styletree.h` — `collect_matching_rules()` O(n×m) 全量扫描

对每个 DOM 节点，遍历所有 CSS 规则的所有选择器，复杂度 O(nodes × rules × selectors)。

**建议**: 预建索引：
- 按标签名索引规则（tag → rules[]）
- 按 class 名索引规则（class → rules[]）
- 按 id 索引规则（id → rules[]）
- 通用选择器单独列表

这样可以将匹配从 O(rules) 降到 O(relevant_rules)。

### 2.4 `interact.h` — 鼠标悬停检测每次遍历整棵布局树

每次鼠标移动事件都遍历整棵树找 deepest match。在大页面中每次 O(n)。

**建议**: 
- 缓存上一次的 hover 节点路径，先检查是否仍在同一节点内
- 或使用空间哈希/四叉树加速空间查找

### 2.5 `interact.h` — `input_update_text` 每次自动滚屏时扫描整个缓冲区

```c
// 每次按键后扫描整个 textarea 缓冲区计算总行数和最大行宽
int _total_lines = 1, _max_line_w = 0;
for (int _i = 0; _i < _ta_len; _i++) { ... }
```

**建议**: 增量维护行数和最大宽度：只在插入/删除 `\n` 或超过当前最大宽度时更新。

### 2.6 `interact.h` — `node_abs_box()` 每次都从头遍历 parent 链

```c
void node_abs_box(...) {
    while (p) { ax += p->x; ay += p->y; p = p->parent; }
}
```
在单帧内被调用数十次（hover 检测、点击检测、聚焦指示器绘制、滚动条计算等）。

**建议**: 在 layout 完成后做一次 DFS 预计算绝对坐标并缓存到 `LayoutNode.abs_x/abs_y` 字段。重建布局时重新计算。

### 2.7 `styletree.h` — Hover 变化时 `recompute_style_subtree` 被多次调用

```c
// interact.h restyle 块
if (prev_hover ...) recompute_style_subtree(sn, ...);
if (g_interact_hover) recompute_style_subtree(sn, ...);
if (g_interact_focus && g_interact_focus != g_interact_hover) recompute_style_subtree(sn, ...);
if (prev_active ...) recompute_style_subtree(sn, ...);
```

如果某节点既是 hover 又是 focus，它可能被重复重算。

**建议**: 先用一个集合收集所有需要重算的节点（去重），然后一次性重算。

### 2.8 `interact.h` — 全局重建布局树

任何 hover/focus/active 状态变化都会触发 `build_layout_tree()` 全量重建。对于包含数千节点的页面，每次鼠标移动都会重建。

**建议**: 
- 仅重建受影响的子树（需要跟踪 DOM→LayoutNode 映射）
- 或只在样式变化影响布局属性（width/height/display/margin 等）时才重建布局，纯装饰性样式（color/bg/border）只重渲染

### 2.9 `render.h` — `screen_flush()` ANSI 转义序列优化

当前每个 cell 变化都输出完整的 ANSI 转义序列：
```c
printf("\033[38;2;%d;%d;%dm", ...);  // 每次 ~20 字节
printf("\033[48;2;%d;%d;%dm", ...);
printf("\033[1m" or "\033[22m");     // bold
printf("\033[4m" or "\033[24m");     // underline
```

**建议**: 
- 使用更紧凑的 ANSI 编码（256 色调色板 vs truecolor 可节省）
- 批量写入：用 `writev()` 或累积到缓冲区再一次性 `write()`
- 考虑只对变化的行进行增量刷新（脏矩形）

---

## 三、代码结构优化

### 3.1 `interact.h` — `interact_run()` 函数过长 (~800行)

主循环混合了：渲染帧、聚焦指示器、select 弹出层绘制、状态栏绘制、滚动条绘制、鼠标事件处理（hover/click/scrollbar/select-popup-click）、键盘事件处理（select-popup-key/input-editing/button-click/summary-toggle/scroll）、滚轮、resize、restyle 重建——全部在一个函数中。

**建议** 拆分为独立函数:
- `draw_frame()` — 渲染帧（聚焦指示器+弹出层+状态栏+滚动条）
- `handle_mouse_event()` — 鼠标事件分发
- `handle_key_event()` — 键盘事件分发
- `handle_select_popup()` — 下拉菜单状态机
- `handle_input_editing()` — 输入框编辑逻辑
- `rebuild_after_restyle()` — restyle→rebuild 流程

### 3.2 `layout.h` — `build_layout_tree_recursive()` 过长 (~500行)

包含大量特化处理（input/textarea/select/button/details/summary/a/img/hr/列表标记/text-transform/::before::after 等），每个都需要特殊逻辑。

**建议** 拆分为:
- `apply_tag_defaults()` — 标签默认样式
- `apply_text_transform()` — text-transform 处理
- `apply_pseudo_content()` — ::before/::after
- `build_inline_text_fragments()` — 文本片段处理

### 3.3 `render.h` — `screen_render_node()` 可拆分

preserve_ws 模式和正常 word-wrap 模式是两个完全不同的大分支，textare 滚动条绘制也是一个独立的大块。

**建议** 拆分为:
- `render_preserved_ws()` — pre/textarea 文本渲染
- `render_word_wrapped()` — 正常文本渲染
- `render_textarea_scrollbar()` — textarea 滚动条

### 3.4 重复代码 — position:relative 偏移

在 `layout_block_children` 和 `layout_flex_children` 中都有相同的代码：

```c
if (child->position_type == 1) {
    child->x += child->position_left - child->position_right;
    child->y += child->position_top - child->position_bottom;
}
```

**建议**: 提取为 `apply_position_offset(LayoutNode* n)` 内联函数。

### 3.5 重复代码 — margin:auto 计算

`layout_block_children` 中有 margin:auto 的三分支计算逻辑，但只适用于 block。flex 布局中没有类似的 margin:auto 处理。

**建议**: 统一 margin 解析（见下文"正确性"）。

### 3.6 重复代码 — select popup 维度计算

`interact_run` 中绘制弹出层和鼠标点击处理弹出层时，各自独立计算了一遍弹出层的尺寸（pop_x, pop_y, pop_w, max_vis 等）。

**建议**: 提取为 `calc_select_popup_rect()` 函数。

---

## 四、正确性 / 健壮性

### 4.1 无 malloc/calloc 返回值检查

几乎所有 `malloc`/`calloc`/`strdup` 调用都没有检查 NULL 返回。在内存不足时会导致崩溃。

**建议**: 至少为关键路径（布局树构建、屏幕缓冲分配）添加 OOM 检查。

### 4.2 `styletree.h` — specificity 打包溢出风险

```c
return (a << 24) | (b << 12) | c;
```

如果 b ≥ 4096（链式选择器中超过 4096 个 class/attribute/pseudo-class），会溢出到 a 的位。虽然在实际 CSS 中几乎不可能出现，但从工程角度应考虑边界情况。

**建议**: 使用更大位宽或结构体比较。

### 4.3 `layout.h` — flex-wrap 不完整

`line_starts` 数组被分配但未使用，`line_max_cross` 被计算但 flex-wrap 的实际换行对齐逻辑不完整（新行的 cross_cursor 计算后，已换行的元素 Y 位置不会调整先前行的元素）。

### 4.4 `styletree.h` — `@media` 仅匹配 viewport_w

`media_query_matches()` 只检查 `min-width`/`max-width`，不支持 `min-height`/`max-height`/`orientation`/`prefers-color-scheme` 等常用媒体特性。

### 4.5 `interact.h` — 固定大小缓冲区可能溢出

```c
LayoutNode* focus_list[256];      // 最多 256 个可聚焦元素
char input_buf[256][4096];        // 每个 4KB
int textarea_scroll_y[256];
```

大型页面可能超过 256 个可聚焦元素导致栈溢出。

**建议**: 使用动态分配或至少添加边界检查。

---

## 五、内存优化

### 5.1 `layout.h` — `extract_text()` 中过度分配

```c
char** pieces = (char**)calloc(children->length, sizeof(char*));
```
为所有子节点分配指针数组，但只使用 TEXT 和 BR 节点。对于有很多元素子节点的节点（如 `<body>`），这是浪费。

**建议**: 先两遍扫描——第一遍计数 TEXT/BR 节点，第二遍只分配所需大小。

### 5.2 `styletree.h` — `compute_styles_for_node()` 中 `concat_values_to_str()` 分配

每次计算样式时，`concat_values_to_str()` 都为每个 CSS 属性值分配新字符串。许多属性值较短且重复（如 "1px", "solid", "block"）。

**建议**: 使用字符串驻留（intern）或小型字符串优化（SSO）减少分配。

### 5.3 布局树和样式树双份存储

`StyledNode` 和 `LayoutNode` 各自存储 children 数组，各自有完整的树结构。对于大型 DOM，这是 ~2x 内存开销。

**建议**: 考虑将样式信息直接嵌入 LayoutNode，或在 restyle 时复用 LayoutNode 而非全量重建。

---

## 六、优先级建议

### 高优先级（显著改进用户体验/性能）

1. **绝对坐标缓存**（§2.6）— 减少每帧数十次 O(depth) 遍历
2. **interact_run 拆分**（§3.1）— 提升可维护性，降低 bug 引入风险
3. **样式变化增量更新**（§2.8）— 减少 hover 时的全量布局重建
4. **malloc 返回值检查**（§4.1）— 防止 OOM 崩溃

### 中优先级（提升代码质量 + 适度性能）

5. **get_default_display / parse_color 优化**（§2.1, §2.2）— 减少链式 strcmp
6. **collect_matching_rules 索引**（§2.3）— 加速样式匹配
7. **select popup 维度计算去重复**（§3.6）
8. **line_starts 死代码清理**（§1.2）

### 低优先级（锦上添花）

9. **uc.h find() 删除**（§1.1）
10. **字符串驻留**（§5.2）
11. **ANSI 序列优化**（§2.9）

---

## 八、已优化确认（无需改动）

这些是在之前迭代中已优化的部分，状态良好：

- ✅ CSS 提取 O(n) 状态机（v2: O(n²) → O(n)）
- ✅ 焦点收集 `collect_focus_list()` / `patch_input_text()` 辅助函数消除重复
- ✅ 边框字符表 `get_border_chars()` + `BorderChars` 结构体消除 switch 重复
- ✅ textarea 滚动限制 + 可视化滚动条
- ✅ textarea 双向滚动（horizontal + vertical）
- ✅ SIGWINCH resize 支持
- ✅ 全局滚动条（垂直+水平）带点击跳转和拖拽
- ✅ `scroll_to` / `scroll_by` 死代码已清理
- ✅ `find_styled_node` 已归位到 styletree.h

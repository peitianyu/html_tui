# Mini Browser 优化分析

> 日期: 2026-08-06
> 范围: 项目现状梳理 + 与真实浏览器对比 + 优化点优先级

## 一、项目架构梳理

**Mini Browser**：C 语言实现的终端 TUI 浏览器引擎，header-only 单头文件架构，termbox2 做终端 I/O。

流水线（复刻浏览器引擎五大阶段）：

```
HTML 文件 → 手工抽取 <style>（mb_extract_css）
  → Gumbo 解析 DOM
  → Katana 解析 CSS
  → StyleTree：选择器匹配 + 继承 + 伪类(:hover/:focus/:nth-child)
  → Layout：盒模型 + Flexbox + Table（content 宽高递归计算）
  → Render：Screen 缓冲 + 24bit 真彩色 ANSI + Unicode 框线
  → Interact：焦点循环、鼠标点击、滚动、输入编辑
```

**架构特点：**

- 公共 API 极小（`mini_browser.h` 526 行），stb 风格 `#define XXX_IMPLEMENTATION`
- 已实现 14 个测试页面覆盖的 CSS 能力相当可观（伪类公式 `an+b`、flex 全套、表格 colspan/rowspan、details/select/textarea 交互、@media 等）
- **滚动模型**：整个文档是一张画布，全局 `scroll_x/scroll_y` 平移视口；渲染时用 clip 矩形裁剪

## 二、与真实浏览器的关键差距（按引擎阶段）

| 阶段 | 真实浏览器 | 本项目现状 |
|---|---|---|
| **渲染** | 脏矩形 + 只重绘变化区域；frame 缓冲 diff 后只输出变化 cell | **每帧全清全画全输出**：`screen_clear()` 清空全部 cell → `screen_render_tree()` 遍历整棵树 → `screen_flush()` 写 `\033[H` 后逐 cell 输出（不对比上一帧，每帧至少输出 宽×高 个字符 + 转义序列） |
| **事件→重算** | 样式/布局失效只影响子树，几何未变则只重绘 | **任何 hover/focus/active 变化都触发 `do_restyle()` → `build_layout_tree()` 全量重建布局树**（interact.h:1868），再全量重绘 |
| **样式匹配** | 规则按 tag/class/id 建索引（hash 桶），另有样式共享缓存 | `collect_matching_rules` 对**每个元素遍历全部规则**，逐条向上回溯祖先匹配，无缓存、无索引（styletree.h:650） |
| **换行** | 布局期一次性断行生成 line boxes，缓存复用 | `render_word_wrapped` 在**渲染期每次重算**（render.h:248），每帧重复断行 |
| **文本存储** | 文本只存在于叶子 text node（文本运行） | `extract_text` 让**每个元素节点都拼接一份子树文本**（layout.h:656），深层树退化为 O(n²) 复制 |
| **导航** | 地址栏、history、bfcache、链接跳转 | `<a href>` 完全未处理（interact.h 无 GUMBO_TAG_A 处理）；页面切换 = 重新读盘 + 全量解析重来，无历史栈 |
| **资源加载** | `<link rel=stylesheet>`、图片、iframe | 只解析内联 `<style>`，img 只显示 `[alt]` 占位符 |
| **表单** | 表单校验、disabled、value 状态模型、撤销/重做 | 无校验；输入缓冲区已改**动态堆分配**（按可聚焦元素数，上限 8192，301 焦点实测峰值 RSS 5.5MB），无撤销/剪贴板 |
| **容量** | 任意 DOM | focus_list/input_buf 已改动态（上限 8192）；剩余固定数组：select_popup_vals[32]/types[32]、details 状态 32 槽、visited 256 条 |

## 三、优化点清单（按优先级）

### 🔴 P0 — 性能（可感知的卡顿源头）

#### 1. 全量布局重建（最大问题）  ✅ 已实现 (2026-08-06)

`do_restyle` 在鼠标移动经过元素时触发，而鼠标 motion 事件是高频的 —— **每移过一个元素 = 整棵布局树从零重建 + 全屏重绘**。复杂页面必然卡顿。

- 方案：区分「几何变化」与「非几何变化」。`:hover` 只改 `color/background/border-color` 时不需要重建布局 —— 只需局部重绘（脏矩形）。只有尺寸/位置类属性变化才做**子树级增量布局**（父链向上传递高度变化，兄弟节点不动）。
- ✅ 已实现（快速路径，commit 60957c3）：`do_restyle` 先对受影响节点做 `geo_summary` 快照 → `recompute_style_subtree` → 再对比几何摘要；**几何未变则 `apply_paint_only` 只同步绘制字段，不重建布局树**（`background` 简写需作为 `background-color` 回退同步）。hover 实测：card bg 34,34,51 → motion 到 card-1 变 51,68,85 → 移出恢复。
- ⬜ 未实现：几何变化的**子树级增量布局**（目前仍全量 `build_layout_tree`）；`:hover` 之外的重绘仍全屏（结合 P0-2 的 flush diff 后仅输出变化 cell）。
- 收益：★★★★★ 成本：★★

#### 2. 全屏无 diff 输出  ✅ 已实现 (2026-08-06)

`screen_flush` 每帧逐 cell 输出。80×24 终端每帧至少 ~2KB 字符 + 大量 ANSI 转义；鼠标拖动/滚动时连续触发。SSH/慢终端下就是明显的「打字延迟」。

- 方案 A（推荐，小改动）：保留 `Screen` 缓冲，flush 时**对比上一帧 buffer，只输出变化的 cell**（缓存上一帧，逐行 diff）。常见 diff 后每帧只输出几十字节。
- ✅ 已实现（方案 A，commit 227400b）：`screen_flush_to_buffer` + `cell_equal`，与上一帧比较只输出变化 cell。实测：15-clickable 首帧 9505B、静止帧 0B、单 cell 变化 43B；00-menu 首帧 20359B。
- 收益：★★★★★ 成本：★

#### 3. 渲染期重复断行  ⬜ 已评估跳过 (2026-08-06)

换行计算应发生在布局期（生成行盒缓存），渲染期直接读缓存。目前每帧对所有可见文本节点重算断行，乘以全屏重绘的帧率，浪费可观。

- **决策：跳过**。实测渲染帧 0.56–0.62ms（00-menu/08-tables/14-text-styles），远小于人眼 16ms 感知阈值；配合 flush diff 后静止帧输出 0B，断行重算的 CPU 成本被完全掩盖。缓存引入失效复杂度（文本/宽度/样式变化时需逐级失效）而收益无感。
- 收益：★（实测后降级）成本：★★

### 🟡 P1 — 健壮性 / 内存安全

#### 4. 1MB 栈上数组  ✅ 已实现 (2026-08-06)

interact.h:750 `input_buf[256][4096]` + 各种 256 数组。在嵌入式/小栈环境直接爆栈。改为堆分配，且**跟随 focus_list 数量动态扩容**，顺带解决 256 上限问题（页面超过 256 个可聚焦元素时目前会静默截断）。

- ✅ 已实现（commit 77e04db）：`count_focusable` 预计数 → `focus_cap` 动态分配 `input_buf`/`input_cursor`/`textarea_*`/`focus_list`（下限 64，上限 8192 保护）；`collect_focus_list`/`count_focusable` 内部栈改堆分配（1024 槽，遇深层 DOM 可再扩容）；`do_restyle` 增加 `focus_cap` 参数。新增 pages/19-many.html（300 按钮）验证焦点链 >256：`tabx:300 enter` → btn-299、`tabx:301` → #300。301 焦点实测峰值 RSS 5.5MB，8192 上限时约 32MB（可接受）。
- 收益：★★★ 成本：★

#### 5. extract_text O(n²)  ⬜ 已核查无需修改 (2026-08-06)

每个元素节点都 strdup 一份子树文本。深层树（如嵌套列表/表格）内存和复制时间翻倍增长。改为「仅叶子持有文本，非叶子惰性拼接或持有指针」。

- **核查结论：非 O(n²)，无需修改**。当前实现（layout.h:668）只收集**直接 TEXT 子节点与 `<br>`**，跳过 ELEMENT 子节点（`/* Only collect direct TEXT nodes and <br>; skip ELEMENT children */`），三次遍历（计数/收集/拼接）复杂度 O(直接文本总长)。每节点文本量 = 自身直接子文本，整树合计 O(总文本量)。
- 内存仍为 O(总文本量)（每元素持有自身直接文本），合理。
- 收益：无 成本：无

#### 6. 全局静态可变状态  ⬜ 已评估暂不处理 (2026-08-06)

`g_det_state[]`（details 开合，layout.h:237）、`g_interact_hover`、`g_visited[]`（styletree.h:104）、`g_det_count` 是模块级全局 —— 不可重入、无法支持多实例，且 details 状态用固定 32 槽 + 64 字节 id（id 截断会串状态）。真实浏览器状态在文档/浏览上下文里。可移入 MiniBrowser 结构体。

- **决策：暂不处理**。当前为单实例浏览器（demo 单页面循环），多实例非当前需求；全局状态使跨页面访问保持（:visited、details 开合在页面切换后仍生效）。若未来支持多实例/嵌入式，将状态移入 `MiniBrowser` 结构体即可，改动面清晰。
- 收益：★★（未来多实例时）成本：★★

### 🟡 P1 — 浏览器行为缺口

#### 7. `<a href>` 导航 + 历史栈  ✅ 全部完成 (2026-08-06)

链接是网页最基本的交互。现在 `<a>` 只是蓝色下划线文本，点了没反应。加：可聚焦 + Enter 跳转 + back/forward 栈（b 返回）+ 状态栏显示 URL。

- ✅ 已实现：`<a href>` 可聚焦 + Enter/点击触发 `on_element_click` 回调 + 访问记录（:visited 变色）+ demo 中 .html 链接自动跳页
- ✅ 已实现（commit d54e572）：历史栈 —— `MiniBrowser.history[32]` + `mb_navigate`（push 当前页再切，覆盖写入截断 forward）/ `mb_go_back`（b 返回）；`InteractCallbacks.edit_active` 防止输入框编辑中误触发 b；无历史时状态栏提示

#### 8. 外部 CSS 加载（`<link rel=stylesheet>`） ✅ 已实现 (2026-08-06)

真实页面几乎必有外部样式表。解析 `<link>` 读本地文件并合并进样式表即可（TUI 无网络也成立）。

- ✅ 已实现（commit 0a7db44）：`mb_extract_css` 识别 `<link rel=stylesheet href=X>`（rel 大小写不敏感，单/双引号），按文档出现顺序读文件插入 css_buf（与 `<style>` 保持真实层叠序）；href 相对 HTML 文件目录解析；缺失文件仅 stderr 警告不崩溃；非 stylesheet 的 link（icon 等）忽略。新增 pages/20-external-css.html + 20-ext.css 验证（内联覆盖外部）。
- 收益：★★★★ 成本：★★

#### 9. 图片渲染  ✅ 已实现 (2026-08-06)

现在是 `[alt]` 占位符。终端可退化为**降采样块字符图**（类似 chafa：每 cell 用前景/背景半块字符混合色，输出近似图像）。

- ✅ 已实现（commit b972e95）：零依赖 PPM P6/P3 + PGM P5/P2 解码，按路径缓存；`<img>` 保持纵横比缩放到块字符图（上限 40x20 cells，width/height 属性可覆盖）；每 cell `▀` 上像素=前景/下像素=背景（同色用空格+背景）；src 相对页面目录；解码失败回退 `[alt]`。新增 pages/21-image.html + 21-img.ppm/pgm（逐行交替色触发 ▀ 混合）+ `--expect-cell` 断言。
- 收益：★★★ 成本：★★★

#### 10. 视口虚拟化

整页文本内容已全部常驻内存（text_content），超大页面内存线性增长。真实浏览器对离屏内容做懒惰分配/淘汰。TUI 上收益中等，页面达到数万行才明显 —— 优先级可放后。

### 🟢 P2 — 工程 / 文档

#### 11. 文档与仓库不一致

README 引用了不存在的 `doc/optimization-analysis.md` —— 已在本文件创建后解决。

#### 12. 无自动化测试  ✅ 已实现 (2026-08-06)

render.h 已有 `RENDER_HEADLESS` 无头模式 —— 已扩展为完整测试套件：

| 工具 | 职责 |
|---|---|
| `tools/headless_test.c` | 加载页面 → 渲染 → 屏幕文本 dump + 布局树 dump（tag/id/class + 绝对坐标 + 盒模型 + 计算样式 + clickable/disabled/type），`--expect`/`--expect-line`/`--expect-count`/`--viewport`/`--flush-ok` 断言 |
| `tools/interact_test.c` | 事件注入队列驱动交互循环：Tab/Shift+Tab/Enter/鼠标点击/悬停/字符键/退格/`tabx:N` 批量 Tab；回调日志 + 状态消息 + 元素文本 + 链接颜色 + 背景色断言 |
| `tools/run_tests.sh` | **47 项断言**：20 页渲染/布局 + 23 项交互场景，全部无头执行（新增 19-many：300 按钮焦点链 >256 验证） |

**测试发现并修复的 3 个真实 bug：**
1. `collect_focus_list` 栈式遍历逆序 → Tab 焦点顺序与文档序相反（interact.h 已修复）
2. 按钮回调设置的状态消息被默认分支 `✓ %s` 覆盖 → demo 的 btn-save 收集值无法显示（mini_browser.h 已修复：回调修改状态则保留）
3. 输入框光标初始在 0 → 输入插到开头而非末尾（interact.h 已修复：播种时光标到末尾）

**后续测试轮次发现并修复：**
4. `border: 1px solid #888` 标准简写不解析（layout.h 新增 `parse_border_shorthand`）
5. `margin-top/left` 等四向独立属性未解析 + `margin: auto` 单边不生效（layout.h apply_styles 补齐独立属性与 auto 检测，含负 margin）
6. `@media` 无类型前缀 (`@media (min-width: N)`) 因 `mq->type` 为空被 early-return 永不匹配；且 collect_matching_rules 曾无条件提取 media 内规则（styletree.h 修复，build_style_tree 新增视口参数供测试）

运行：`bash tools/run_tests.sh`（需 gcc + RENDER_HEADLESS）

#### 13. 编译产物清理 / run.sh 权限

`run.sh` 生成 `demo.exe`/`mini_browser.exe`，注意 .gitignore 覆盖；当前 run.sh 有未提交的 chmod 变更（100644→100755）。

## 四、建议执行顺序

| 轮次 | 内容 | 预期效果 |
|---|---|---|
| **第 1 轮** | P0-1 + P0-2：hover 局部重绘 + flush diff | 鼠标移动、滚动的卡顿感消失，「体验」级别的提升 |
| **第 2 轮** | P1-4 + P1-5：动态数组 + 文本存储优化 | 内存安全、大页面不再截断 |
| **第 3 轮** | P1-7 + P1-8：链接导航 + 历史 + 外部 CSS | 从「demo 引擎」变成「可用的终端浏览器」 |
| **第 4 轮** | P0-3 + P2-12：断行缓存 + 无头测试套件 | 性能收尾 + 可回归验证 |

**执行进度（2026-08-06）**：第 1 轮 ✅（P0-1 快速路径 60957c3、P0-2 flush diff 227400b）；第 2 轮 ✅（P1-4 动态数组 77e04db，P1-5 核查无需修改）；第 3 轮 ✅（P1-8 外部 CSS 0a7db44、P1-7 历史栈 d54e572）；P1-9 图片渲染 ✅（b972e95）；第 4 轮半 ✅（P2-12 测试套件 56 项，P0-3 评估跳过）。剩余：P1-6 多实例（暂缓）、P0-1 几何变化子树级增量布局、P1-10 视口虚拟化（可后置）。

**单点推荐**：P0-2（flush diff）改动面最小（只动 `screen_flush` 一个函数，加一个上一帧缓冲）、收益立竿见影、不触碰布局逻辑、无回归风险。

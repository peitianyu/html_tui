# 图片渲染（`<img>` 块字符图）设计

> 日期: 2026-08-06
> 对应优化文档 P1-9（可选，锦上添花）

## 目标

`<img>` 从 `[alt]` 占位符升级为终端块字符图（chafa 同款思路：每 cell 用半块字符 + 前景/背景色输出近似图像）。

## 设计决策

1. **格式（零依赖）**：PPM P6/P3（RGB）+ PGM P5/P2（灰度→RGB），手写解码。不引入 libpng（避免链接依赖）；PNG/JPEG 范围外。
2. **解码时机与缓存**：布局构建时从 `src` 解码 → **全局按路径缓存**（`g_img_cache[16]`）。页面切换 / hover restyle 重建布局树时不重复读图（图文件小，解码 <1ms，缓存避免 restyle 卡顿）。缓存生命周期 = 进程（性能缓存，文档记录为可接受的全局态）。
3. **布局**：`LayoutNode` 加 `struct ImgData* img`（非 NULL = 图片元素）。cell 尺寸缩放：保持纵横比（字符 1:2 像素比），上限 **40×20 cells**、不放大；`width`/`height` HTML 属性（纯数字 = cell 数）可覆盖。img 的 `content 宽高` = cell 尺寸。
4. **渲染**：`screen_render_node` 中 img 优先于 text_content 绘制。每 cell = 2 像素行 × 1 像素列，取上/下中心像素：上下同色 → 空格+背景色；不同色 → `▀`(0x2580) + 前景=上色、背景=下色。随滚动平移、按 clip 裁切。
5. **路径解析**：`src` 相对**页面文件目录**——`build_layout_tree` 加可选 `base_dir` 参数（NULL = 相对 cwd）；mini_browser/headless_test 传页面 dir；`do_restyle` 经 `InteractCallbacks.base_dir`（mini_browser 设置）传递。
6. **回退**：无 `src` / 文件缺失 / 格式不支持 → 现有 `[alt]`/`[img]` 文本，不崩溃。

## 实现位置

- `src/core/layout.h`：`ImgData` 结构、PPM/PGM 解码、`g_img_cache`、`build_layout_tree` 加 `base_dir`、img 节点布局
- `src/core/render.h`：`draw_img`（块图绘制）+ `screen_render_node` 分支
- `src/core/interact.h`：`InteractCallbacks.base_dir`、`do_restyle` 加参数并透传
- `src/core/mini_browser.h` / `tools/headless_test.c`：调用点传 base_dir
- `tools/headless_test.c`：新增 `--expect-cell=X,Y=fg:r,g,b;bg:r,g,b` 断言（直接读 Screen cell）

## 测试

- Python 生成 `pages/21-img.ppm`（8×8 四象限分色块）与 `pages/21-img-gray.pgm`
- `pages/21-image.html`：正常 img + width/height 覆盖 + 缺失文件（回退 alt）
- headless 断言：布局 content=WxH（缩放）、`--expect-cell` 验证块图颜色、缺失文件回退 `[img]`
- 回归：现有 54 项不受影响

## 范围外（YAGNI）

- PNG/JPEG 解码（需库）
- CSS 百分比尺寸 / object-fit
- 跨页共享缓存淘汰策略（16 项够用）
- 动画 / 交互式缩放

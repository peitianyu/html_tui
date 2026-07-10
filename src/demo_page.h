/* ======================== demo_page.h ========================
 * User-editable page configuration for the TUI browser demo.
 *
 * Edit the HTML, CSS, and button callbacks in this file.
 * The engine code (render.h / interact.h / layout.h) stays untouched.
 * ============================================================== */

#ifndef DEMO_PAGE_H
#define DEMO_PAGE_H

/* ======================== HTML Content ======================== */
/* Edit this to change the page structure.
 * Add id="xxx" attributes to elements you want to find later
 * in button callbacks (via find_node_by_id). */
#define DEMO_HTML \
    "<html><body>" \
    "  <h1 id='main-title'>🌐 Mini Browser Demo</h1>" \
    "  <div class='nav'>" \
    "    <span>Home</span> <span>About</span> <span>Contact</span>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2>🎮 交互测试</h2>" \
    "    <p>Tab 切换焦点 / Enter/Space 点击按钮 / 箭头键滚动</p>" \
    "    <hr>" \
    "    <div class='interact-row'>" \
    "      <span>输入框 A:</span> <input id='input-a' value='Hello'>" \
    "    </div>" \
    "    <div class='interact-row'>" \
    "      <span>输入框 B:</span> <input id='input-b' value='World'>" \
    "    </div>" \
    "    <div class='interact-row'>" \
    "      <span>输入框 C:</span> <input id='input-c' value='中文测试'>" \
    "    </div>" \
    "    <hr>" \
    "    <div class='interact-row'>" \
    "      <span>按钮区域:</span> <button id='btn-submit'>提交</button>" \
    "      <button id='btn-reset'>重置</button> <button id='btn-exit'>✕ 退出</button>" \
    "    </div>" \
    "    <hr>" \
    "    <div class='interact-row'>" \
    "      <span>清空输入:</span> <button id='btn-clr-a'>清空A</button> <button id='btn-clr-b'>清空B</button> <button id='btn-clr-c'>清空C</button>" \
    "    </div>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2 id='card1-title'>Card Title</h2>" \
    "    <p>Paragraph with <strong>bold</strong> and <u>underline</u>.</p>" \
    "    <p id='card1-note' style='color:#ff6600;'>Orange inline-styled text.</p>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2>中文测试</h2>" \
    "    <p>你好世界！这是一段中文文本，用于测试 UTF-8 多字节字符渲染。</p>" \
    "    <p>🌟 Star / ★ Star / ☆ 空心星号 — 各种符号测试。</p>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2>&lt;pre&gt; 测试</h2>" \
    "    <pre>  前面两个空格  中间  多个   空格  结尾  </pre>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2>表格测试</h2>" \
    "    <table>" \
    "      <tr><th>名称</th><th>数量</th><th>价格</th></tr>" \
    "      <tr><td>苹果</td><td>3</td><td>¥5.00</td></tr>" \
    "      <tr><td>香蕉</td><td>2</td><td>¥3.50</td></tr>" \
    "      <tr><td>橙子</td><td>5</td><td>¥8.00</td></tr>" \
    "    </table>" \
    "  </div>" \
    "  <hr>" \
    "  <div class='card'>" \
    "    <h2>列表 &amp; 链接 &amp; 图片</h2>" \
    "    <ul><li id='li-first'>苹果</li><li>香蕉</li><li>橙子</li></ul>" \
    "    <ol><li>第一步</li><li>第二步</li><li>第三步</li></ol>" \
    "    <p>访问 <a href='https://baidu.com'>Example</a> 了解更多</p>" \
    "    <p>图片: <img>  <input value='hello'>  <button id='btn-go'>Go</button></p>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2>Another Card</h2>" \
    "    <p>Scroll down for more content below here. Word wrap should keep words intact across line breaks.</p>" \
    "    <p>Arrow keys / PgUp/Dn / Home/End / q=quit</p>" \
    "  </div>" \
    "  <div class='card'>" \
    "    <h2>Third Card</h2>" \
    "    <p>Even more content to scroll through comfortably.</p>" \
    "    <p>The footer is below this card.</p>" \
    "  </div>" \
    "  <div class='footer'>" \
    "    <p id='footer-text'>Footer - Press q to quit</p>" \
    "  </div>" \
    "</body></html>"

/* ======================== CSS Content ======================== */
/* Edit this to change the page styling */
#define DEMO_CSS \
    "body{background-color:#1a1a2e;color:#e0e0e0;padding:1;}" \
    "h1{color:#e94560;text-align:center;font-weight:bold;margin:1;}" \
    "h2{color:#53a8b6;font-weight:bold;margin:1;}" \
    ".nav{display:flex;justify-content:center;gap:4;" \
    "     background-color:#16213e;padding:1;margin-bottom:1;}" \
    ".nav span{color:#53a8b6;font-weight:bold;}" \
    ".card{background-color:#0f3460;padding:2;margin:1;" \
    "      border:1;border-style:dotted;border-color:#e94560;}" \
    ".card p{color:#e0e0e0;}" \
    ".interact-row{display:flex;align-items:center;gap:2;margin-bottom:1;}" \
    ".interact-row span{color:#53a8b6;min-width:14;}" \
    "table{color:#e0e0e0;width:40;border-style:solid;border-color:#53a8b6;text-align:center;}" \
    "td,th{padding:0 0;border:1;border-style:solid;border-color:#0f3460;}" \
    "th{font-weight:bold;}" \
    "hr{color:#53a8b6;}" \
    "strong{color:#e94560;font-weight:bold;}" \
    "u{color:#53a8b6;text-decoration:underline;}" \
    ".footer{text-align:center;color:#888;margin-top:1;}" \
    "pre{background-color:#1a1a2e;color:#0f0;padding:1;border:1;border-style:solid;border-color:#53a8b6;}" \
    "button{background-color:#e94560;color:#fff;font-weight:bold;}" \
    "button:hover{background-color:#ff6b81;}" \
    "button:active{background-color:#c23150;}" \
    "input{background-color:#1a1a2e;color:#e0e0e0;width:20;}" \
    "input:focus{background-color:#16213e;color:#fff;}"

/* ======================== Button Callbacks ======================== */
/*
 * 元素绑定方法说明:
 *
 * 【方法一】通过 focus_idx + focus_list 获取按钮本身:
 *   LayoutNode* btn = focus_list[focus_idx];
 *   // btn->styled->node 就是目标元素的 GumboNode
 *   GumboAttribute* id = gumbo_get_attribute(&btn->styled->node->v.element.attributes, "id");
 *   // 这样就可以按 id 识别按钮，不依赖文本内容
 *
 * 【方法二】通过 HTML id 查找任意元素:
 *   LayoutNode* target = find_node_by_id(cb->layout_root, "my-element-id");
 *   node_set_text(target, "新文本");   // 修改内容
 *
 * 【方法三】通过标签名 + 序号查找:
 *   LayoutNode* p2 = find_nth_node_by_tag(cb->layout_root, GUMBO_TAG_P, 2);
 *
 * 【方法四】通过 class 查找:
 *   LayoutNode* card = find_node_by_class(cb->layout_root, "card");
 *
 * 修改后立刻生效(无需重建布局)。也可返回 true 触发完整 layout rebuild。
 *
 * ── 自定义退出键 ──
 * 设置 cb.on_key 可以覆盖默认的退出键 (q/Q/ESC/Ctrl+C/D)。
 * 示例: 实现"按 x 退出"而不影响 q:
 *   static bool my_on_key(struct tb_event* ev, InteractCallbacks* cb) {
 *       if (ev->ch == 'x' || ev->key == TB_KEY_ESC) {
 *           cb->quit_flag = true; return true;
 *       }
 *       return false;
 *   }
 */

#include <string.h>
#include <stdio.h>

#include "core/interact.h"

/* ── 默认按键处理器 (q/Q/ESC/Ctrl+C/D 退出) ── */
static bool demo_default_on_key(struct tb_event* ev, InteractCallbacks* cb) {
    if (ev->ch == 'q' || ev->ch == 'Q' || ev->key == TB_KEY_ESC ||
        ev->key == TB_KEY_CTRL_C || ev->key == TB_KEY_CTRL_D) {
        cb->quit_flag = true;
        return true;
    }
    return false;
}

static bool demo_on_button_click(const char* btn_text, int focus_idx,
                                  LayoutNode** focus_list, int focus_count,
                                  char input_buf[][64], int input_buf_count,
                                  InteractCallbacks* cb)
{
    (void)focus_idx; (void)focus_list; (void)focus_count;
    (void)input_buf; (void)input_buf_count;
    bool need_restyle = false;

    /* ── 获取按钮的 DOM 属性 (不依赖文本匹配) ── */
    const char* btn_id = NULL;
    if (focus_idx >= 0 && focus_idx < focus_count &&
        focus_list[focus_idx]->styled && focus_list[focus_idx]->styled->node &&
        focus_list[focus_idx]->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* attr = gumbo_get_attribute(
            &focus_list[focus_idx]->styled->node->v.element.attributes, "id");
        if (attr) btn_id = attr->value;
    }

    /* ── 按 id 识别按钮 ── */
    if (btn_id && strcmp(btn_id, "btn-submit") == 0) {
        /* 收集输入框值 */
        char vals[512] = "";
        int n = 0;
        for (int i = 0; i < focus_count && i < input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                char tmp[80];
                snprintf(tmp, sizeof(tmp), "%s%s=%s", n>0?", ":"",
                    gumbo_get_attribute(&focus_list[i]->styled->node->v.element.attributes, "value") ? "inp" : "inp",
                    input_buf[i]);
                strncat(vals, tmp, sizeof(vals)-strlen(vals)-1);
                n++;
            }
        }
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ 提交: %s", n > 0 ? vals : "(无输入框)");

        /* ── 示范：持久化修改其他元素 (会用 override 跨 rebuild 保持) ── */
        node_override_text(cb, "card1-note", "✓ 提交成功! (文本已持久化)");

    } else if (btn_id && strcmp(btn_id, "btn-reset") == 0) {
        for (int i = 0; i < focus_count && i < input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                input_buf[i][0] = '\0';
            }
        }
        for (int i = 0; i < focus_count && i < input_buf_count; i++) {
            if (focus_list[i]->styled && focus_list[i]->styled->node &&
                focus_list[i]->styled->node->type == GUMBO_NODE_ELEMENT &&
                focus_list[i]->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
                input_buf[i][0] = '\0';
            }
        }
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ 重置: 所有输入框已清空");

    } else if (btn_id && strcmp(btn_id, "btn-exit") == 0) {
        /* 关闭 → 退出程序 */
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ 退出中…");
        cb->quit_flag = true;

    } else if (btn_id && strcmp(btn_id, "btn-clr-a") == 0) {
        if (input_buf_count >= 1) { input_buf[0][0] = '\0'; }
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ 输入框 A 已清空");

    } else if (btn_id && strcmp(btn_id, "btn-clr-b") == 0) {
        if (input_buf_count >= 2) { input_buf[1][0] = '\0'; }
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ 输入框 B 已清空");

    } else if (btn_id && strcmp(btn_id, "btn-clr-c") == 0) {
        if (input_buf_count >= 3) { input_buf[2][0] = '\0'; }
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ 输入框 C 已清空");

    } else if (btn_id && strcmp(btn_id, "btn-go") == 0) {
        /* ── 示范：持久化修改标题文本 ── */
        static int click_count = 0;
        click_count++;
        char buf[64];
        snprintf(buf, sizeof(buf), "🌐 Mini Browser (Go x%d)", click_count);
        node_override_text(cb, "main-title", buf);
        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ Go button clicked");

    } else {
        /* 按文本匹配作为后备 */
        if (strcmp(btn_text, "提交") == 0) { /* 旧式匹配, 保持兼容 */ }
        else if (strcmp(btn_text, "重置") == 0) {}
        else if (strcmp(btn_text, "清空A") == 0) {}
        else if (strcmp(btn_text, "清空B") == 0) {}
        else if (strcmp(btn_text, "清空C") == 0) {}
        else if (strcmp(btn_text, "Go") == 0) {}

        snprintf(cb->status_msg, sizeof(cb->status_msg), "✓ %s", btn_text);
    }

    /* Refresh input text from buffers after action */
    for (int fi = 0; fi < focus_count && fi < input_buf_count; fi++) {
        LayoutNode* n = focus_list[fi];
        if (n->styled && n->styled->node &&
            n->styled->node->type == GUMBO_NODE_ELEMENT &&
            n->styled->node->v.element.tag == GUMBO_TAG_INPUT) {
            char buf[128];
            const char* val = input_buf[fi];
            if (val && val[0]) snprintf(buf, sizeof(buf), "%s", val);
            else snprintf(buf, sizeof(buf), " ");
            if (n->text_content) free(n->text_content);
            n->text_content = strdup(buf);
        }
    }

    return need_restyle;
}

/* ======================== Entry Point ======================== */

static void demo_run(void) {
    /* Parse HTML + CSS */
    GumboOutput* dom = gumbo_parse(DEMO_HTML);
    KatanaOutput* css_out = katana_parse(DEMO_CSS, strlen(DEMO_CSS), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);

    /* Init terminal */
    if (render_init()) {
        fprintf(stderr, "render_init failed\n");
        return;
    }

    /* Build layout */
    int vw, vh;
    render_size(&vw, &vh);
    LayoutNode* lt = build_layout_tree(st, vw, vh);

    /* Setup callbacks */
    InteractCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.on_button_click = demo_on_button_click;
    cb.on_focus_change = NULL;
    cb.on_key = demo_default_on_key;  /* q/Q/ESC/Ctrl+C/D → quit_flag */
    cb.quit_flag = false;

    cb.status_msg[0] = '\0';

    /* Run interaction loop */
    interact_run(lt, css_out->stylesheet, st, &cb);

    /* Cleanup */
    render_shutdown();
    free_layout_tree(lt);
    free_style_tree(st);
    katana_destroy_output(css_out);
    gumbo_destroy_output(&kGumboDefaultOptions, dom);
    printf("Goodbye.\n");
}

#endif /* DEMO_PAGE_H */

/*
 * demo_main.c - Interactive TUI browser: HTML+CSS → Layout → Termbox2
 *
 * Build: tcc src/demo_main.c -I src -run
 */

#include <stdio.h>
#include <string.h>

#define UC_IMPLEMENTATION
#include "core/uc.h"

#define STYLETREE_IMPLEMENTATION
#include "core/styletree.h"

/* Undef katana.h's flex ECHO macro to avoid redefinition warning
   when termbox2.h (via render.h) includes <termios.h> which also defines ECHO. */
#undef ECHO

#define LAYOUT_IMPLEMENTATION
#include "core/layout.h"

#define RENDER_IMPLEMENTATION
#include "core/render.h"

int main(void) {
    const char* html =
        "<html><body>"
        "  <h1>🌐 Mini Browser Demo</h1>"
        "  <div class='nav'>"
        "    <span>Home</span> <span>About</span> <span>Contact</span>"
        "  </div>"
        "  <div class='card'>"
        "    <h2>Card Title</h2>"
        "    <p>Paragraph with <strong>bold</strong> and <u>underline</u>.</p>"
        "    <p style='color:#ff6600;'>Orange inline-styled text.</p>"
        "  </div>"
        "  <div class='card'>"
        "    <h2>中文测试</h2>"
        "    <p>你好世界！这是一段中文文本，用于测试 UTF-8 多字节字符渲染。</p>"
        "    <p>🌟 Star / ★ Star / ☆ 空心星号 — 各种符号测试。</p>"
        "  </div>"
        "  <div class='card'>"
        "    <h2>&lt;pre&gt; 测试</h2>"
        "    <pre>  前面两个空格  中间  多个   空格  结尾  </pre>"
        "  </div>"
        "  <div class='card'>"
        "    <h2>表格测试</h2>"
        "    <table>"
        "      <tr><th>名称</th><th>数量</th><th>价格</th></tr>"
        "      <tr><td>苹果</td><td>3</td><td>¥5.00</td></tr>"
        "      <tr><td>香蕉</td><td>2</td><td>¥3.50</td></tr>"
        "      <tr><td>橙子</td><td>5</td><td>¥8.00</td></tr>"
        "    </table>"
        "  </div>"
        "  <hr>"
        "  <div class='card'>"
        "    <h2>列表 &amp; 链接 &amp; 图片</h2>"
        "    <ul><li>苹果</li><li>香蕉</li><li>橙子</li></ul>"
        "    <ol><li>第一步</li><li>第二步</li><li>第三步</li></ol>"
        "    <p>访问 <a href='https://baidu.com'>Example</a> 了解更多</p>"
        "    <p>图片: <img>  <input value='hello'>  <button>Go</button></p>"
        "  </div>"
        "  <div class='card'>"
        "    <h2>Another Card</h2>"
        "    <p>Scroll down for more content below here. Word wrap should keep words intact across line breaks.</p>"
        "    <p>Arrow keys / PgUp/Dn / Home/End / q=quit</p>"
        "  </div>"
        "  <div class='card'>"
        "    <h2>Third Card</h2>"
        "    <p>Even more content to scroll through comfortably.</p>"
        "    <p>The footer is below this card.</p>"
        "  </div>"
        "  <div class='footer'>"
        "    <p>Footer - Press q to quit</p>"
        "  </div>"
        "</body></html>";

    const char* css =
        "body{background-color:#1a1a2e;color:#e0e0e0;padding:1;}"
        "h1{color:#e94560;text-align:center;font-weight:bold;margin:1;}"
        "h2{color:#53a8b6;font-weight:bold;margin:1;}"
        ".nav{display:flex;justify-content:center;gap:4;"
        "     background-color:#16213e;padding:1;margin-bottom:1;}"
        ".nav span{color:#53a8b6;font-weight:bold;}"
        ".card{background-color:#0f3460;padding:2;margin:1;"
        "      border:1;border-style:solid;border-color:#e94560;}"
        ".card p{color:#e0e0e0;}"
        "table{color:#e0e0e0;width:100%;border:1;border-style:solid;border-color:#53a8b6;}"
        "td,th{padding:1;border:1;border-style:solid;border-color:#0f3460;}"
        "th{font-weight:bold;}"
        "hr{color:#53a8b6;}"
        "strong{color:#e94560;font-weight:bold;}"
        "u{color:#53a8b6;text-decoration:underline;}"
        ".footer{text-align:center;color:#888;margin-top:1;}"
        "pre{background-color:#1a1a2e;color:#0f0;padding:1;border:1;border-style:solid;border-color:#53a8b6;}";

    /* Parse → Style → Layout */
    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);

    /* Init termbox2 */
    if (render_init()) {
        fprintf(stderr, "render_init failed\n");
        return 1;
    }

    /* Build layout with real terminal size */
    int vw, vh;
    render_size(&vw, &vh);
    LayoutNode* lt = build_layout_tree(st, vw, vh);

    /* Run interaction loop */
    render_run(lt);

    /* Cleanup */
    render_shutdown();
    free_layout_tree(lt);
    free_style_tree(st);
    katana_destroy_output(css_out);
    gumbo_destroy_output(&kGumboDefaultOptions, dom);
    printf("Goodbye.\n");
    return 0;
}

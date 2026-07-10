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
        "strong{color:#e94560;font-weight:bold;}"
        "u{color:#53a8b6;text-decoration:underline;}"
        ".footer{text-align:center;color:#888;margin-top:1;}";

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

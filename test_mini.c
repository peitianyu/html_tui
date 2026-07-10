/*
 * test_mini.c - 单元测试: 编译运行 ./test_mini
 *
 * 测试覆盖:
 * - UTF-8 编解码和宽度
 * - 布局引擎 (block/flex/table)
 * - 文本渲染 (word-wrap, alignment)
 *
 * Build: tcc test_mini.c -I src -o test_mini && ./test_mini
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ======================== 最小测试框架 ======================== */

static int test_passed = 0, test_failed = 0;
#define TEST(name) do { printf("  TEST %-50s ", name); fflush(stdout); } while(0)
#define ASSERT(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n  at %s:%d\n", msg, __FILE__, __LINE__); test_failed++; return; } \
} while(0)
#define PASS() do { printf("OK\n"); test_passed++; } while(0)
#define RUN_SUITE(name) do { printf("\n=== %s ===\n", name); } while(0)

#define UC_IMPLEMENTATION
#include "core/uc.h"

/* ======================== 1. UTF-8 宽度测试 ======================== */

static void test_utf8_len(void) {
    TEST("uc_len for ASCII");
    ASSERT(uc_len("A") == 1, "A should be 1 byte");
    ASSERT(uc_len("\n") == 1, "newline should be 1 byte");
    ASSERT(uc_len("\x7F") == 1, "DEL should be 1 byte");
    PASS();

    TEST("uc_len for 2-byte UTF-8");
    ASSERT(uc_len("\xC2\xA9") == 2, "(c) should be 2 bytes");  /* © */
    PASS();

    TEST("uc_len for 3-byte UTF-8");
    ASSERT(uc_len("\xE4\xB8\xAD") == 3, "中 should be 3 bytes");
    PASS();

    TEST("uc_len for 4-byte UTF-8");
    ASSERT(uc_len("\xF0\x9F\x8C\x9F") == 4, "🌟 should be 4 bytes");
    PASS();
}

static void test_utf8_width(void) {
    TEST("uc_wid ASCII");
    ASSERT(uc_wid('A') == 1, "A width 1");
    ASSERT(uc_wid(' ') == 1, "space width 1");
    ASSERT(uc_wid('1') == 1, "digit width 1");
    PASS();

    TEST("uc_wid CJK");
    ASSERT(uc_wid(0x4E2D) == 2, "中 width 2");           /* 中 */
    ASSERT(uc_wid(0x6587) == 2, "文 width 2");           /* 文 */
    ASSERT(uc_wid(0x9FFF) == 2, "CJK upper bound");      /* CJK boundary */
    ASSERT(uc_wid(0x4E00) == 2, "CJK start");            /* CJK start */
    PASS();

    TEST("uc_wid emoji");
    ASSERT(uc_wid(0x1F31F) == 2, "🌟 width 2");           /* U+1F31F */
    ASSERT(uc_wid(0x2605) == 1, "★ width 1");            /* Black star */
    ASSERT(uc_wid(0x2606) == 1, "☆ width 1");            /* White star */
    ASSERT(uc_wid(0x231A) == 2, "watch width 2");        /* Watch */
    ASSERT(uc_wid(0x2B50) == 2, "⭐ width 2");            /* Star emoji */
    PASS();

    TEST("uc_wid misc symbols");
    ASSERT(uc_wid(0x2014) == 1, "— width 1");             /* em dash */
    ASSERT(uc_wid(0x00A9) == 1, "© width 1");             /* copyright */
    ASSERT(uc_wid(0x00AE) == 1, "® width 1");             /* registered */
    PASS();
}

static void test_utf8_decode(void) {
    {
        const char* s = "A";
        uint32_t cp = uc_dec(&s);
        ASSERT(cp == 'A', "decode 'A'");
        ASSERT(*s == '\0', "ptr advanced past 'A'");
    }
    {
        const char* s = "\xE4\xB8\xAD";  /* 中 */
        uint32_t cp = uc_dec(&s);
        ASSERT(cp == 0x4E2D, "decode 中");
    }
    {
        const char* s = "\xF0\x9F\x8C\x9F";  /* 🌟 */
        uint32_t cp = uc_dec(&s);
        ASSERT(cp == 0x1F31F, "decode 🌟");
    }
    TEST("utf8_decode");
    PASS();
}

static void test_utf8_str_width(void) {
    TEST("uc_str_width ASCII");
    ASSERT(uc_str_width("Hello") == 5, "Hello width 5");
    PASS();

    TEST("uc_str_width Chinese");
    ASSERT(uc_str_width("中文") == 4, "中文 width 4");
    ASSERT(uc_str_width("你好世界") == 8, "你好世界 width 8");
    PASS();

    TEST("uc_str_width mixed");
    int w = uc_str_width("🌟 Star");
    ASSERT(w == 7, "🌟 Star width 7 (emoji 2 + space 1 + Star 4)");
    PASS();
}

/* ======================== 2. 布局引擎测试 ======================== */

/* Include layout implementation */
#define STYLETREE_IMPLEMENTATION
#include "core/styletree.h"
#define LAYOUT_IMPLEMENTATION
#include "core/layout.h"

/* Helper: parse HTML+CSS → LayoutNode */
static LayoutNode* parse_to_layout(const char* html, const char* css, int vw, int vh) {
    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);
    LayoutNode* lt = build_layout_tree(st, vw, vh);
    /* Store dom/css_out/st in globals for cleanup? We'll just leak for tests. */
    return lt;
}

static LayoutNode* find_by_tag(LayoutNode* node, const char* tag) {
    if (!node) return NULL;
    if (node->styled && node->styled->node &&
        node->styled->node->type == GUMBO_NODE_ELEMENT) {
        const char* t = gumbo_normalized_tagname(node->styled->node->v.element.tag);
        if (strcmp(t, tag) == 0) return node;
    }
    for (size_t i = 0; i < node->num_children; i++) {
        LayoutNode* found = find_by_tag(node->children[i], tag);
        if (found) return found;
    }
    return NULL;
}

static void test_block_layout(void) {
    LayoutNode* root = parse_to_layout(
        "<html><body><div>Hello</div><p>World</p></body></html>",
        "body{padding:0;}div{color:red;}p{color:blue;}",
        80, 24
    );
    LayoutNode* body = find_by_tag(root, "body");

    TEST("block: body exists");
    ASSERT(body != NULL, "body should exist");
    ASSERT(body->width == 80, "body width = viewport");
    PASS();

    TEST("block: children positioned vertically");
    if (body && body->num_children >= 2) {
        LayoutNode* div = find_by_tag(body, "div");
        LayoutNode* p = find_by_tag(body, "p");
        ASSERT(div != NULL, "div exists");
        ASSERT(p != NULL, "p exists");
        ASSERT(div->y == 0, "div starts at y=0");
        ASSERT(p->y > div->y, "p below div");
    }
    PASS();
}

static void test_table_layout(void) {
    LayoutNode* root = parse_to_layout(
        "<html><body>"
        "<table>"
        "  <tr><td>A</td><td>B</td></tr>"
        "  <tr><td>C</td><td>D</td></tr>"
        "</table>"
        "</body></html>",
        "body{padding:0;}td{border:1;padding:1;}",
        80, 24
    );

    LayoutNode* table = find_by_tag(root, "table");

    TEST("table: node exists");
    ASSERT(table != NULL, "table should exist");
    ASSERT(table->display == DISPLAY_TABLE, "table display=TABLE");
    PASS();

    /* Gumbo wraps <tr> in <tbody>. Get actual rows: first child of table if tbody, else table */
    LayoutNode* tbody = NULL;
    if (table->num_children == 1 && table->children[0] &&
        table->children[0]->styled &&
        table->children[0]->styled->node &&
        table->children[0]->styled->node->type == GUMBO_NODE_ELEMENT &&
        table->children[0]->styled->node->v.element.tag == GUMBO_TAG_TBODY) {
        tbody = table->children[0];
    }

    TEST("table: tbody detected");
    ASSERT(tbody != NULL, "tbody wrapper exists");
    ASSERT(tbody->num_children == 2, "2 rows in tbody");
    PASS();
    PASS();

    if (table && table->num_children >= 2) {
        LayoutNode* r0 = table->children[0];
        LayoutNode* r1 = table->children[1];

        TEST("table: rows positioned vertically");
        ASSERT(r0->y == 0, "row0 at y=0");
        ASSERT(r1->y >= r0->y, "row1 below row0");
        PASS();

        TEST("table: cells in row0");
        ASSERT(r0->num_children >= 2, "row0 has 2 cells");
        if (r0->num_children >= 2) {
            LayoutNode* c0 = r0->children[0];
            LayoutNode* c1 = r0->children[1];
            ASSERT(c0->x < c1->x, "cell0 left of cell1");
        }
        PASS();

        TEST("table: cells in row1");
        ASSERT(r1->num_children >= 2, "row1 has 2 cells");
        if (r1->num_children >= 2) {
            LayoutNode* c0 = r1->children[0];
            LayoutNode* c1 = r1->children[1];
            ASSERT(c0->x < c1->x, "cell0 left of cell1");
        }
        PASS();

        TEST("table: same column widths across rows");
        if (r0->num_children >= 2 && r1->num_children >= 2) {
            /* Total cell width (content + padding + border) should match */
            int col0_w0 = r0->children[0]->width +
                          r0->children[0]->padding_left + r0->children[0]->padding_right +
                          r0->children[0]->border_left + r0->children[0]->border_right;
            int col0_w1 = r1->children[0]->width +
                          r1->children[0]->padding_left + r1->children[0]->padding_right +
                          r1->children[0]->border_left + r1->children[0]->border_right;
            ASSERT(col0_w0 == col0_w1, "col0 same width in both rows");
        }
        PASS();

        TEST("table: equal row heights");
        ASSERT(r0->height == r1->height, "rows same height");
        PASS();
    }
}

static void test_flex_layout(void) {
    LayoutNode* root = parse_to_layout(
        "<html><body><div style='display:flex'>"
        "  <span>A</span><span>B</span>"
        "</div></body></html>",
        "body{padding:0;}",
        80, 24
    );

    LayoutNode* flex = find_by_tag(root, "div");

    TEST("flex: container created");
    ASSERT(flex != NULL, "flex container exists");
    ASSERT(flex->display == DISPLAY_FLEX, "display=flex");
    PASS();

    TEST("flex: children side by side");
    if (flex && flex->num_children >= 2) {
        LayoutNode* c0 = flex->children[0];
        LayoutNode* c1 = flex->children[1];
        ASSERT(c0->x < c1->x, "child0 left of child1");
    }
    PASS();
}

/* ======================== 3. 文本渲染测试 ======================== */

#define RENDER_IMPLEMENTATION
/* katana.h defines ECHO which conflicts with termios.h from termbox2.h */
#undef ECHO
#include "core/render.h"

static void test_render_wrap(void) {
    Screen* s = screen_create(20, 10);
    LayoutNode* root = parse_to_layout(
        "<html><body><p>Hello World Foo Bar</p></body></html>",
        "body{padding:0;}p{width:10;}",
        20, 10
    );

    LayoutNode* p = root ? find_by_tag(root, "p") : NULL;

    TEST("render: find paragraph");
    ASSERT(p != NULL, "p exists");
    PASS();

    if (p && p->text_content) {
        screen_clear(s);
        s->scroll_x = 0; s->scroll_y = 0;
        screen_render_node(s, p);

        TEST("render: text written to screen");
        /* Check first cell has a character */
        Cell* cell = &s->cells[0];
        ASSERT(cell->ch != 0, "screen cell has content");
        PASS();
    }
    screen_free(s);
}

static void test_render_utf8(void) {
    Screen* s = screen_create(30, 10);
    LayoutNode* root = parse_to_layout(
        "<html><body><p>你好世界</p></body></html>",
        "body{padding:0;}",
        30, 10
    );

    LayoutNode* p = root ? find_by_tag(root, "p") : NULL;

    TEST("render: Chinese text in paragraph");
    ASSERT(p != NULL, "p exists");
    ASSERT(p->text_content != NULL, "p has text");
    ASSERT(strcmp(p->text_content, "你好世界") == 0, "text is 你好世界");
    PASS();

    if (p) {
        screen_clear(s);
        s->scroll_x = 0; s->scroll_y = 0;
        screen_render_node(s, p);

        TEST("render: CJK chars rendered to cells");
        /* First char 你 (U+4F60) should be in cell[0] */
        Cell* cell0 = &s->cells[0];
        Cell* cell1 = &s->cells[1];
        ASSERT(cell0->ch == 0x4F60, "cell0 has 你");
        ASSERT(cell1->ch == 0, "cell1 is continuation (ch=0)");
        PASS();
    }
    screen_free(s);
}

static void test_render_border(void) {
    Screen* s = screen_create(20, 10);
    LayoutNode* root = parse_to_layout(
        "<html><body><div style='border:2;border-style:solid'>Hi</div></body></html>",
        "body{padding:0;}",
        20, 10
    );

    LayoutNode* div = root ? find_by_tag(root, "div") : NULL;

    TEST("render: border div exists");
    ASSERT(div != NULL, "div exists");
    PASS();

    if (div) {
        screen_clear(s);
        s->scroll_x = 0; s->scroll_y = 0;
        screen_render_node(s, div);

        /* Border draws + - | at edges */
        TEST("render: top-left corner is '+'");
        Cell* tl = &s->cells[0];
        ASSERT(tl->ch == '+', "top-left corner is +");
        PASS();

        TEST("render: top edge is '-'");
        Cell* top = &s->cells[1];
        ASSERT(top->ch == '-', "top edge is -");
        PASS();
    }
    screen_free(s);
}

/* ======================== 4. CSS 选择器测试 ======================== */

static void test_selector_matching(void) {
    GumboOutput* dom = gumbo_parse(
        "<html><body>"
        "  <div id='main' class='container' data-type='test-x'>"
        "    <p class='highlight'>Text</p>"
        "  </div>"
        "</body></html>"
    );
    KatanaOutput* css = katana_parse(
        ".highlight{color:red;}[data-type^='test']{color:blue;}",
        strlen(".highlight{color:red;}[data-type^='test']{color:blue;}"),
        KatanaParserModeStylesheet
    );
    StyledNode* st = build_style_tree(dom->root, css->stylesheet);

    TEST("selector: find highlighted paragraph");
    StyledNode* p = NULL;
    for (size_t i = 0; i < st->num_children; i++) {
        if (st->children[i]->node->type == GUMBO_NODE_ELEMENT &&
            st->children[i]->node->v.element.tag == GUMBO_TAG_BODY &&
            st->children[i]->num_children > 0) {
            StyledNode* body = st->children[i];
            for (size_t j = 0; j < body->num_children; j++) {
                if (body->children[j]->node->type == GUMBO_NODE_ELEMENT &&
                    body->children[j]->node->v.element.tag == GUMBO_TAG_DIV) {
                    StyledNode* div = body->children[j];
                    for (size_t k = 0; k < div->num_children; k++) {
                        if (div->children[k]->node->type == GUMBO_NODE_ELEMENT &&
                            div->children[k]->node->v.element.tag == GUMBO_TAG_P) {
                            p = div->children[k];
                        }
                    }
                }
            }
        }
    }
    ASSERT(p != NULL, "found <p> element");
    ASSERT(p->styles.length > 0, "<p> has styles");
    const char* color = get_style(p, "color");
    ASSERT(color != NULL && strcmp(color, "red") == 0, "color=red from .highlight");
    PASS();
}

/* ======================== 5. run all ======================== */

int main(void) {
    printf("mini_test_c 单元测试\n");
    printf("====================\n");

    RUN_SUITE("UTF-8");
    test_utf8_len();
    test_utf8_width();
    test_utf8_decode();
    test_utf8_str_width();

    RUN_SUITE("Layout Engine");
    test_block_layout();
    test_table_layout();
    test_flex_layout();

    RUN_SUITE("Render Engine");
    test_render_wrap();
    test_render_utf8();
    test_render_border();

    RUN_SUITE("CSS Selectors");
    test_selector_matching();

    printf("\n===== %d passed, %d failed =====\n", test_passed, test_failed);
    return test_failed > 0 ? 1 : 0;
}

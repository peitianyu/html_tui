/*
 * test_demo.c - 自动渲染测试: 验证 demo 中每个元素的Screen buffer输出
 *
 * Build: tcc test_demo.c -I src -o test_demo && ./test_demo
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int test_passed = 0, test_failed = 0;
#define TEST(name) do { printf("  TEST %-55s ", name); fflush(stdout); } while(0)
#define ASSERT(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n  at %s:%d\n", msg, __FILE__, __LINE__); test_failed++; return; } \
} while(0)
#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { printf("FAIL: %s (expected %d, got %d)\n  at %s:%d\n", msg, (int)(b), (int)(a), __FILE__, __LINE__); test_failed++; return; } \
} while(0)
#define PASS() do { printf("OK\n"); test_passed++; } while(0)
#define RUN_SUITE(name) do { printf("\n=== %s ===\n", name); } while(0)

#define UC_IMPLEMENTATION
#include "core/uc.h"
#define STYLETREE_IMPLEMENTATION
#include "core/styletree.h"
#undef ECHO
#define LAYOUT_IMPLEMENTATION
#include "core/layout.h"
#define RENDER_IMPLEMENTATION
#include "core/render.h"

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

/* Parse HTML+CSS and return layout tree */
static LayoutNode* parse(const char* html, const char* css, int vw, int vh) {
    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);
    LayoutNode* lt = build_layout_tree(st, vw, vh);
    return lt;
}

/* ========== 1. Input 渲染 ========== */

static void test_render_input_with_value(void) {
    LayoutNode* root = parse(
        "<html><body><input value='hello'></body></html>",
        "body{padding:0;}", 40, 10);
    LayoutNode* input = find_by_tag(root, "input");

    Screen* s = screen_create(40, 10);
    s->scroll_x = 0; s->scroll_y = 0;
    screen_render_tree(s, root);

    TEST("render input: top-left has '['");
    ASSERT(s->cells[0].ch == '[', "'[' at (0,0)");
    PASS();

    TEST("render input: cells have [hello] sequentially");
    ASSERT(s->cells[0].ch == '[', "cell[0] = '['");
    ASSERT(s->cells[1].ch == 'h', "cell[1] = 'h'");
    ASSERT(s->cells[2].ch == 'e', "cell[2] = 'e'");
    ASSERT(s->cells[3].ch == 'l', "cell[3] = 'l'");
    ASSERT(s->cells[4].ch == 'l', "cell[4] = 'l'");
    ASSERT(s->cells[5].ch == 'o', "cell[5] = 'o'");
    ASSERT(s->cells[6].ch == ']', "cell[6] = ']'");
    PASS();

    TEST("render input: foreground color is gray (180,180,180)");
    ASSERT(s->cells[0].fg_r == 180 && s->cells[0].fg_g == 180 && s->cells[0].fg_b == 180, "gray fg");
    PASS();

    screen_free(s);
}

static void test_render_input_empty(void) {
    LayoutNode* root = parse(
        "<html><body><input></body></html>",
        "body{padding:0;}", 40, 10);
    Screen* s = screen_create(40, 10);
    s->scroll_x = 0; s->scroll_y = 0;
    screen_render_tree(s, root);

    TEST("render input(empty): shows [  ] placeholder");
    ASSERT(s->cells[0].ch == '[', "cell[0] = '['");
    ASSERT(s->cells[1].ch == ' ', "cell[1] = ' '");
    ASSERT(s->cells[2].ch == ' ', "cell[2] = ' '");
    ASSERT(s->cells[3].ch == ']', "cell[3] = ']'");
    PASS();

    screen_free(s);
}

/* ========== 2. Button 渲染 ========== */

static void test_render_button_text(void) {
    LayoutNode* root = parse(
        "<html><body><button>Go</button></body></html>",
        "body{padding:0;}", 40, 10);
    Screen* s = screen_create(40, 10);
    s->scroll_x = 0; s->scroll_y = 0;
    screen_render_tree(s, root);

    TEST("render button: shows text");
    ASSERT(s->cells[0].ch == 'G', "cell[0] = 'G'");
    ASSERT(s->cells[1].ch == 'o', "cell[1] = 'o'");
    PASS();

    TEST("render button: bold");
    ASSERT(s->cells[0].bold == true, "bold on");
    PASS();

    screen_free(s);
}

static void test_render_button_bg(void) {
    LayoutNode* root = parse(
        "<html><body><button>X</button></body></html>",
        "body{padding:0;}", 40, 10);
    LayoutNode* btn = find_by_tag(root, "button");
    Screen* s = screen_create(40, 10);
    s->scroll_x = 0; s->scroll_y = 0;
    screen_render_tree(s, root);

    TEST("render button: background color present");
    ASSERT(btn != NULL, "button exists");
    ASSERT(btn->bg_color.valid, "bg_color valid");
    PASS();

    screen_free(s);
}

/* ========== 3. Input + Button 混合渲染 ========== */

static void test_render_mixed_inline(void) {
    LayoutNode* root = parse(
        "<html><body><p>Search: <input value='hi'> <button>Go</button></p></body></html>",
        "body{padding:0;}", 80, 10);
    Screen* s = screen_create(80, 10);
    s->scroll_x = 0; s->scroll_y = 0;
    screen_render_tree(s, root);

    TEST("render mixed: inline children side by side");
    LayoutNode* p = find_by_tag(root, "p");
    if (p) {
        printf("\n    <p>: x=%d y=%d w=%d h=%d\n", p->x, p->y, p->width, p->height);
        for (size_t i = 0; i < p->num_children && i < 5; i++) {
            LayoutNode* ch = p->children[i];
            printf("    child[%zu]: x=%d y=%d w=%d h=%d display=%d text='%s'\n", i,
                   ch->x, ch->y, ch->width, ch->height, ch->display,
                   ch->text_content ? ch->text_content : "NULL");
        }
    }
    /* Render and check positions in screen buffer */
    screen_clear(s);
    s->scroll_x = 0; s->scroll_y = 0;
    screen_render_tree(s, root);
    printf("    First row cells: ");
    for (int c = 0; c < 30; c++) {
        char ch = (char)s->cells[c].ch;
        if (ch < 32 || ch > 126) ch = '.';
        printf("%c", ch);
    }
    printf("\n");
    fflush(stdout);
    bool found_S = false;
    for (int c = 0; c < 30; c++) {
        if (s->cells[c].ch == 'S') { found_S = true; break; }
    }
    ASSERT(found_S, "'S' visible on screen");
    PASS();

    TEST("render mixed: [hi] visible on screen");
    bool found_hi = false;
    for (int c = 0; c < 20; c++) {
        if (s->cells[c].ch == '[' && s->cells[c+1].ch == 'h' && s->cells[c+2].ch == 'i' && s->cells[c+3].ch == ']') {
            found_hi = true;
            break;
        }
    }
    ASSERT(found_hi, "[hi] rendered on screen");

    /* Find where Go appears */
    bool found_button = false;
    for (int c = 0; c < 30; c++) {
        if (s->cells[c].ch == 'G' && s->cells[c+1].ch == 'o') {
            found_button = true;
            break;
        }
    }
    ASSERT(found_button, "Go rendered on screen");
    PASS();

    screen_free(s);
}

/* ========== 4. Input 文本编辑测试 (模拟) ========== */

static void test_input_text_edit(void) {
    LayoutNode* root = parse(
        "<html><body><input value='ab'></body></html>",
        "body{padding:0;}", 40, 10);
    LayoutNode* input = find_by_tag(root, "input");

    TEST("input edit: initial text is [ab]");
    ASSERT(input != NULL && input->text_content, "input has text");
    ASSERT(strcmp(input->text_content, "[ab]") == 0, "[ab]");
    PASS();

    /* Simulate typing 'c' at end: [abc] */
    if (input && input->text_content) {
        int cur_len = (int)strlen(input->text_content); /* "[ab]" = 4 */
        int cpos = cur_len - 2; /* after '[' = 1 */
        /* Make room and insert */
        input->text_content = (char*)realloc(input->text_content, cur_len + 2);
        memmove(&input->text_content[1 + cpos + 1],
                &input->text_content[1 + cpos],
                cur_len - 1 - cpos + 1);
        input->text_content[1 + cpos] = 'c';
    }

    TEST("input edit: after typing 'c' → [abc]");
    ASSERT(input != NULL && input->text_content, "text after edit");
    ASSERT(strcmp(input->text_content, "[abc]") == 0, "[abc] after insert");
    PASS();

    /* Simulate backspace: [ac] */
    if (input && input->text_content) {
        int cur_len = (int)strlen(input->text_content);
        memmove(&input->text_content[1 + 1],
                &input->text_content[1 + 2],
                cur_len - 2 - 1);
        input->text_content[cur_len - 1] = '\0';
    }

    TEST("input edit: after backspace 'b' → [ac]");
    ASSERT(input != NULL && input->text_content, "text after backspace");
    ASSERT(strcmp(input->text_content, "[ac]") == 0, "[ac] after backspace");
    PASS();
}

/* ========== 5. Button 在 flex 容器中 ========== */

static void test_button_in_flex(void) {
    LayoutNode* root = parse(
        "<html><body><div style='display:flex'><button>A</button><button>B</button></div></body></html>",
        "body{padding:0;}", 40, 10);
    LayoutNode* div = find_by_tag(root, "div");

    TEST("button in flex: container is flex");
    ASSERT(div != NULL && div->display == DISPLAY_FLEX, "display:flex");
    PASS();

    TEST("button in flex: two children");
    ASSERT(div != NULL && div->num_children >= 2, "two buttons");
    LayoutNode* b1 = div->children[0];
    LayoutNode* b2 = div->children[1];
    ASSERT(b1->x < b2->x, "A left of B");
    PASS();
}

/* ========== run all ========== */

int main(void) {
    printf("Demo 渲染自测试\n");
    printf("==============\n");

    RUN_SUITE("Input 渲染");
    test_render_input_with_value();
    test_render_input_empty();

    RUN_SUITE("Button 渲染");
    test_render_button_text();
    test_render_button_bg();

    RUN_SUITE("Input + Button 混合");
    test_render_mixed_inline();

    RUN_SUITE("Input 编辑逻辑");
    test_input_text_edit();

    RUN_SUITE("Button 在 Flex 中");
    test_button_in_flex();

    printf("\n===== %d passed, %d failed =====\n", test_passed, test_failed);
    return test_failed > 0 ? 1 : 0;
}

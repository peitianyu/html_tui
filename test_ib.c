/*
 * test_ib.c - 测试 <input> 和 <button> 的渲染与交互
 * Build: tcc test_ib.c -I src -o test_ib && ./test_ib
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int test_passed = 0, test_failed = 0;
#define TEST(name) do { printf("  TEST %-55s ", name); fflush(stdout); } while(0)
#define ASSERT(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n  at %s:%d\n", msg, __FILE__, __LINE__); test_failed++; return; } \
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

static LayoutNode* parse(const char* html, const char* css, int vw) {
    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* st = build_style_tree(dom->root, css_out->stylesheet);
    LayoutNode* lt = build_layout_tree(st, vw, 24);
    return lt;
}

/* ========== input 测试 ========== */

static void test_input_value_attribute(void) {
    LayoutNode* root = parse(
        "<html><body><input value='hello'></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* input = find_by_tag(root, "input");

    TEST("input: node exists");
    ASSERT(input != NULL, "input node");
    PASS();

    TEST("input: display=INLINE");
    ASSERT(input->display == DISPLAY_INLINE, "display inline");
    PASS();

    TEST("input: text_content is [hello]");
    ASSERT(input->text_content != NULL, "has text");
    ASSERT(strcmp(input->text_content, "[hello]") == 0, "[hello]");
    PASS();
}

static void test_input_no_value(void) {
    LayoutNode* root = parse(
        "<html><body><input></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* input = find_by_tag(root, "input");

    TEST("input (no value): placeholder [  ]");
    ASSERT(input != NULL && input->text_content != NULL, "has text");
    ASSERT(strcmp(input->text_content, "[  ]") == 0, "[  ] placeholder");
    PASS();
}

static void test_input_gray_color(void) {
    LayoutNode* root = parse(
        "<html><body><input value='x'></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* input = find_by_tag(root, "input");

    TEST("input: gray foreground color");
    ASSERT(input != NULL, "input exists");
    ASSERT(input->color.valid, "color valid");
    ASSERT(input->color.r == 180 && input->color.g == 180 && input->color.b == 180, "180,180,180 gray");
    PASS();
}

static void test_input_inline_flow(void) {
    LayoutNode* root = parse(
        "<html><body><p>Name: <input value='test'></p></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* p = find_by_tag(root, "p");
    LayoutNode* input = find_by_tag(root, "input");

    TEST("input: inside <p> as inline child");
    ASSERT(p != NULL, "p exists");
    ASSERT(input != NULL, "input exists");
    /* input should be a child of p */
    bool found = false;
    for (size_t i = 0; i < p->num_children; i++) {
        if (p->children[i] == input) { found = true; break; }
    }
    ASSERT(found, "input is child of p");
    PASS();
}

/* ========== button 测试 ========== */

static void test_button_default_display(void) {
    LayoutNode* root = parse(
        "<html><body><button>Click</button></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* btn = find_by_tag(root, "button");

    TEST("button: node exists");
    ASSERT(btn != NULL, "button node");
    PASS();

    TEST("button: display=INLINE");
    ASSERT(btn->display == DISPLAY_INLINE, "display inline");
    PASS();

    TEST("button: has text content");
    ASSERT(btn->text_content != NULL && strcmp(btn->text_content, "Click") == 0, "Click");
    PASS();
}

static void test_button_bold(void) {
    LayoutNode* root = parse(
        "<html><body><button>Submit</button></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* btn = find_by_tag(root, "button");

    TEST("button: bold text");
    ASSERT(btn != NULL, "button exists");
    ASSERT(btn->font_bold, "font_bold is true");
    PASS();
}

static void test_button_background(void) {
    LayoutNode* root = parse(
        "<html><body><button>Go</button></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* btn = find_by_tag(root, "button");

    TEST("button: has background color");
    ASSERT(btn != NULL, "button exists");
    ASSERT(btn->bg_color.valid, "bg_color valid");
    PASS();
}

static void test_button_text(void) {
    LayoutNode* root = parse(
        "<html><body><button>Go</button></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* btn = find_by_tag(root, "button");

    TEST("button: text is 'Go'");
    ASSERT(btn != NULL && btn->text_content, "button text");
    ASSERT(strcmp(btn->text_content, "Go") == 0, "Go");
    PASS();
}

/* ========== 混合测试 ========== */

static void test_input_button_mix(void) {
    LayoutNode* root = parse(
        "<html><body><p>Search: <input value='query'> <button>Find</button></p></body></html>",
        "body{padding:0;}", 80);
    LayoutNode* input = find_by_tag(root, "input");
    LayoutNode* btn = find_by_tag(root, "button");

    TEST("mixed: input and button both exist");
    ASSERT(input != NULL, "input exists");
    ASSERT(btn != NULL, "button exists");
    PASS();

    TEST("mixed: input has correct text");
    ASSERT(strcmp(input->text_content, "[query]") == 0, "[query]");
    PASS();

    TEST("mixed: button has correct text");
    ASSERT(strcmp(btn->text_content, "Find") == 0, "Find");
    PASS();

    TEST("mixed: input is before button in parent");
    LayoutNode* p = find_by_tag(root, "p");
    bool found_input = false;
    bool order_ok = false;
    for (size_t i = 0; i < p->num_children; i++) {
        if (p->children[i] == input) { found_input = true; }
        else if (p->children[i] == btn && found_input) { order_ok = true; break; }
    }
    ASSERT(order_ok, "input before button");
    PASS();
}

/* ========== run all ========== */

int main(void) {
    printf("Input & Button 测试\n");
    printf("===================\n");

    RUN_SUITE("Input");
    test_input_value_attribute();
    test_input_no_value();
    test_input_gray_color();
    test_input_inline_flow();

    RUN_SUITE("Button");
    test_button_default_display();
    test_button_bold();
    test_button_background();
    test_button_text();

    RUN_SUITE("Mixed");
    test_input_button_mix();

    printf("\n===== %d passed, %d failed =====\n", test_passed, test_failed);
    return test_failed > 0 ? 1 : 0;
}

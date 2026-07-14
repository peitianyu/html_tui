/* ── Unit tests for checkbox/radio/range/password rendering ──
 *   Build: tcc -I src -run src/test_input.c
 *   Asserts text_content format of various INPUT types after layout.
 */
#define MINI_BROWSER_IMPLEMENTATION
#include "core/mini_browser.h"

#include <string.h>
#include <stdlib.h>

static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg) do {                    \
    if (cond) { printf("  ✓ %s\n", msg); g_pass++; } \
    else      { printf("  ✗ %s FAILED\n", msg); g_fail++; } \
} while(0)

#define CHECK_STR(s, expected, msg) do {                 \
    if (s && strcmp((s), (expected)) == 0) {             \
        printf("  ✓ %s = \"%s\"\n", msg, (s)); g_pass++; \
    } else {                                             \
        printf("  ✗ %s: got \"%s\", expected \"%s\"\n", \
               msg, (s) ? (s) : "(null)", (expected));   \
        g_fail++;                                        \
    }                                                    \
} while(0)

#define CHECK_COLOR(n, r, g, b, msg) do {                      \
    if ((n)->color.r == (r) && (n)->color.g == (g) && (n)->color.b == (b)) { \
        printf("  ✓ %s color=#%02x%02x%02x\n", msg, (n)->color.r, (n)->color.g, (n)->color.b); g_pass++; \
    } else {                                                    \
        printf("  ✗ %s: color is #%02x%02x%02x, expected #%02x%02x%02x\n", \
               msg, (n)->color.r, (n)->color.g, (n)->color.b, (r), (g), (b)); g_fail++; \
    }                                                           \
} while(0)

static LayoutNode* find_by_id(LayoutNode* root, const char* id) {
    if (!root) return NULL;
    if (root->styled && root->styled->node &&
        root->styled->node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* a = gumbo_get_attribute(
            &root->styled->node->v.element.attributes, "id");
        if (a && a->value && strcmp(a->value, id) == 0) return root;
    }
    for (size_t i = 0; i < root->num_children; i++) {
        LayoutNode* f = find_by_id(root->children[i], id);
        if (f) return f;
    }
    return NULL;
}

/* ── Parse HTML → Layout ── */
static LayoutNode* parse_and_layout(const char* html, const char* css,
                                     int vw, int vh) {
    GumboOutput* dom = gumbo_parse(html);
    KatanaOutput* css_out = katana_parse(css, strlen(css), KatanaParserModeStylesheet);
    StyledNode* styled = build_style_tree(dom->root, css_out->stylesheet);
    LayoutNode* layout = build_layout_tree(styled, vw, vh);
    /* Note: don't free dom/css/styled here — layout nodes reference them */
    return layout;
}

static void free_layout_deep(LayoutNode* tree);

/* ══════════════════════════════════════════════════════════════════ */
/*  Test 1: checkbox rendering                                         */
/* ══════════════════════════════════════════════════════════════════ */
static void test_checkbox(void) {
    printf("\n── Test: Checkbox ──\n");
    const char* html =
        "<html><body>"
        "<input type='checkbox' id='cb1' value='Option A' checked>"
        "<input type='checkbox' id='cb2' value='Option B'>"
        "</body></html>";
    const char* css = "";

    LayoutNode* root = parse_and_layout(html, css, 80, 24);
    LayoutNode* cb1 = find_by_id(root, "cb1");
    LayoutNode* cb2 = find_by_id(root, "cb2");

    /* cb1: checked */
    CHECK(cb1 != NULL, "cb1 found");
    CHECK_STR(cb1->text_content, "[x] Option A", "cb1 checked text");
    /* CHECK_COLOR(cb1, 120, 220, 120, "cb1 checked color"); */

    /* cb2: unchecked */
    CHECK(cb2 != NULL, "cb2 found");
    CHECK_STR(cb2->text_content, "[ ] Option B", "cb2 unchecked text");
    /* CHECK_COLOR(cb2, 200, 200, 200, "cb2 unchecked color"); */

    /* Simulate toggle cb1: should uncheck */
    { LayoutNode* a = cb1; char buf[128];
    const char* ll = strchr(a->text_content, ']'); if (!ll) ll = strchr(a->text_content, ')'); ll += 2;
    snprintf(buf, sizeof(buf), "[ ] %s", ll);
    node_set_text(a, buf);
    a->color.r = 200; a->color.g = 200; a->color.b = 200;
    CHECK_STR(a->text_content, "[ ] Option A", "cb1 after toggle unchecked"); }

    /* Simulate toggle cb2: should check */
    { LayoutNode* b = cb2; char buf[128];
    const char* ll = strchr(b->text_content, ']'); if (!ll) ll = strchr(b->text_content, ')'); ll += 2;
    snprintf(buf, sizeof(buf), "[x] %s", ll);
    node_set_text(b, buf);
    b->color.r = 120; b->color.g = 220; b->color.b = 120;
    CHECK_STR(b->text_content, "[x] Option B", "cb2 after toggle checked"); }

    /* Double toggle: back to original */
    { LayoutNode* b = cb2; char buf[128];
    const char* ll = strchr(b->text_content, ']'); if (!ll) ll = strchr(b->text_content, ')'); ll += 2;
    snprintf(buf, sizeof(buf), "[ ] %s", ll);
    node_set_text(b, buf);
    CHECK_STR(b->text_content, "[ ] Option B", "cb2 double toggle back to unchecked"); }

    free_layout_tree(root);
}

/* ══════════════════════════════════════════════════════════════════ */
/*  Test 2: radio rendering + group mutual exclusion                  */
/* ══════════════════════════════════════════════════════════════════ */
static void test_radio(void) {
    printf("\n── Test: Radio ──\n");
    const char* html =
        "<html><body>"
        "<input type='radio' id='r1' name='g' value='Yes' checked>"
        "<input type='radio' id='r2' name='g' value='No'>"
        "</body></html>";
    const char* css = "";

    LayoutNode* root = parse_and_layout(html, css, 80, 24);
    LayoutNode* r1 = find_by_id(root, "r1");
    LayoutNode* r2 = find_by_id(root, "r2");

    CHECK(r1 != NULL, "r1 found");
    CHECK_STR(r1->text_content, "(•) Yes", "r1 checked text");

    CHECK(r2 != NULL, "r2 found");
    CHECK_STR(r2->text_content, "( ) No", "r2 unchecked text");

    /* Build focus_list for radio_group_uncheck */
    LayoutNode* focus_list[10];
    focus_list[0] = r1;
    focus_list[1] = r2;
    int focus_count = 2;

    /* Simulate clicking r2: uncheck group then check r2 */
    printf("  ── click r2 (No) ──\n");
    radio_group_uncheck(r2, focus_list, focus_count, NULL);
    CHECK_STR(r1->text_content, "( ) Yes", "r1 uncheck by radio_group_uncheck()");
    {
        LayoutNode* a = r2; char buf[128];
        const char* ll = strchr(a->text_content, ')'); ll += 2;
        snprintf(buf, sizeof(buf), "(•) %s", ll);
        node_set_text(a, buf);
        a->color.r = 120; a->color.g = 220; a->color.b = 120;
    }
    CHECK_STR(r2->text_content, "(•) No", "r2 after toggle checked");

    /* Simulate clicking r1: uncheck group then check r1 */
    printf("  ── click r1 (Yes) ──\n");
    radio_group_uncheck(r1, focus_list, focus_count, NULL);
    CHECK_STR(r2->text_content, "( ) No", "r2 auto-uncheck by group");
    {
        LayoutNode* a = r1; char buf[128];
        const char* ll = strchr(a->text_content, ')'); ll += 2;
        snprintf(buf, sizeof(buf), "(•) %s", ll);
        node_set_text(a, buf);
        a->color.r = 120; a->color.g = 220; a->color.b = 120;
    }
    CHECK_STR(r1->text_content, "(•) Yes", "r1 re-checked");

    /* Click r1 again: already checked → no change */
    printf("  ── click r1 again (already checked) ──\n");
    CHECK((unsigned char)r1->text_content[1] == 0xE2, "r1 was already checked, remains checked");
    CHECK_STR(r1->text_content, "(•) Yes", "r1 still checked (no-op)");

    free_layout_tree(root);
}

/* ══════════════════════════════════════════════════════════════════ */
/*  Test 3: range slider rendering                                     */
/* ══════════════════════════════════════════════════════════════════ */
static void test_range(void) {
    printf("\n── Test: Range ──\n");
    const char* html =
        "<html><body>"
        "<input type='range' id='rg1' value='30'>"
        "<input type='range' id='rg2' value='75'>"
        "</body></html>";
    const char* css = "";

    LayoutNode* root = parse_and_layout(html, css, 80, 24);
    LayoutNode* rg1 = find_by_id(root, "rg1");
    LayoutNode* rg2 = find_by_id(root, "rg2");

    CHECK(rg1 != NULL, "rg1 found");
    CHECK(rg1->text_content != NULL, "rg1 has text");
    {
        const char* t = rg1->text_content;
        CHECK(t[0] == '[', "rg1 starts with [");
        CHECK(strstr(t, "30%") != NULL, "rg1 shows 30%%");
    }

    CHECK(rg2 != NULL, "rg2 found");
    {
        const char* t = rg2->text_content;
        CHECK(strstr(t, "75%") != NULL, "rg2 shows 75%%");
    }

    /* Test range_update_text: drag rg1 to 80% */
    printf("  ── drag rg1 to 80%% ──\n");
    range_update_text(rg1, 80, NULL);
    {
        const char* t = rg1->text_content;
        CHECK(t != NULL, "rg1 after drag has text");
        CHECK(strstr(t, "80%") != NULL, "rg1 shows 80%% after drag");
        CHECK(t[0] == '[', "rg1 still starts with [");
    }

    /* Test range_update_text: clamp > 100 */
    range_update_text(rg1, 150, NULL);
    CHECK(strstr(rg1->text_content, "100%") != NULL, "rg1 clamped to 100%%");

    /* Test range_update_text: clamp < 0 */
    range_update_text(rg2, -10, NULL);
    CHECK(strstr(rg2->text_content, "0%") != NULL, "rg2 clamped to 0%%");

    /* Test range at exactly 100% */
    printf("  ── drag rg1 to 100%% ──\n");
    range_update_text(rg1, 100, NULL);
    {
        const char* t = rg1->text_content;
        CHECK(t != NULL, "rg1 at 100% has text");
        CHECK(strstr(t, "100%") != NULL, "rg1 shows 100%%");
        /* Verify whole structure: [bar] pct% */
        CHECK(t[0] == '[', "rg1 at 100% starts with [");
        const char* rb = strchr(t, ']');
        CHECK(rb != NULL, "rg1 at 100% has closing ]");
        if (rb) {
            CHECK(rb[1] == ' ', "rg1 at 100% has space after ]");
            CHECK(strstr(rb, "100%") != NULL, "rg1 has 100%% after ]");
        }
        int len = (int)strlen(t);
        CHECK(len >= 25, "rg1 at 100% has reasonable length");
        /* Verify width is synced */
        int w = uc_str_width(t);
        CHECK(rg1->width == w, "rg1 width matches uc_str_width after 100%%");
    }

    /* Test range at exactly 0% */
    printf("  ── drag rg2 to 0%% ──\n");
    range_update_text(rg2, 0, NULL);
    {
        const char* t = rg2->text_content;
        CHECK(t != NULL, "rg2 at 0% has text");
        CHECK(strstr(t, "0%") != NULL, "rg2 shows 0%%");
        CHECK(t[0] == '[', "rg2 at 0% starts with [");
        const char* rb = strchr(t, ']');
        CHECK(rb != NULL, "rg2 at 0% has closing ]");
        if (rb) CHECK(strstr(rb, "0%") != NULL, "rg2 has 0%% after ]");
    }

    free_layout_tree(root);
}

/* ══════════════════════════════════════════════════════════════════ */
/*  Test 4: password masking                                           */
/* ══════════════════════════════════════════════════════════════════ */
static void test_password(void) {
    printf("\n── Test: Password ──\n");
    const char* html =
        "<html><body>"
        "<input type='password' id='pw1' value='secret'>"
        "<input type='password' id='pw2' value=''>"
        "</body></html>";
    const char* css = "";

    LayoutNode* root = parse_and_layout(html, css, 80, 24);
    LayoutNode* pw1 = find_by_id(root, "pw1");
    LayoutNode* pw2 = find_by_id(root, "pw2");

    CHECK(pw1 != NULL, "pw1 found");
    {
        const char* t = pw1->text_content;
        CHECK(t != NULL, "pw1 has text");
        int mask_count = 0;
        for (int i = 0; t[i]; i++) if ((unsigned char)t[i] == 0xE2) mask_count++;
        CHECK(mask_count == 6, "pw1 has 6 • chars (secret length)");
    }

    CHECK(pw2 != NULL, "pw2 found");
    CHECK(pw2->text_content != NULL, "pw2 has text");
    CHECK(strlen(pw2->text_content) >= 1, "pw2 empty value has placeholder");

    free_layout_tree(root);
}

/* ══════════════════════════════════════════════════════════════════ */
/*  Test 5: color input                                                */
/* ══════════════════════════════════════════════════════════════════ */
static void test_color(void) {
    printf("\n── Test: Color ──\n");
    const char* html =
        "<html><body>"
        "<input type='color' id='clr1' value='#e94560'>"
        "<input type='color' id='clr2'>"
        "</body></html>";
    const char* css = "";

    LayoutNode* root = parse_and_layout(html, css, 80, 24);
    LayoutNode* clr1 = find_by_id(root, "clr1");
    LayoutNode* clr2 = find_by_id(root, "clr2");

    CHECK(clr1 != NULL, "clr1 found");
    CHECK_STR(clr1->text_content, "[■] #e94560", "clr1 text");
    CHECK(clr1->bg_color.valid, "clr1 bg_color valid");

    CHECK(clr2 != NULL, "clr2 found");
    CHECK_STR(clr2->text_content, "[■] #000000", "clr2 default text");

    free_layout_tree(root);
}

/* ══════════════════════════════════════════════════════════════════ */
/*  Test 6: number validation                                          */
/* ══════════════════════════════════════════════════════════════════ */
static void test_number(void) {
    printf("\n── Test: Number ──\n");
    const char* html =
        "<html><body>"
        "<input type='number' id='num1' value='42'>"
        "<input type='number' id='num2' value='abc'>"
        "</body></html>";
    const char* css = "";

    LayoutNode* root = parse_and_layout(html, css, 80, 24);
    LayoutNode* num1 = find_by_id(root, "num1");
    LayoutNode* num2 = find_by_id(root, "num2");

    CHECK(num1 != NULL, "num1 found");
    CHECK_STR(num1->text_content, "42", "num1 valid number");

    CHECK(num2 != NULL, "num2 found");
    CHECK_STR(num2->text_content, "[invalid]", "num2 invalid number");

    free_layout_tree(root);
}

/* ══════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("═══ INPUT TYPES UNIT TEST ═══\n");

    test_checkbox();
    test_radio();
    test_range();
    test_password();
    test_color();
    test_number();

    printf("\n──────────────────────────────\n");
    printf("  PASS: %d  FAIL: %d\n", g_pass, g_fail);
    printf("──────────────────────────────\n");

    return g_fail > 0 ? 1 : 0;
}

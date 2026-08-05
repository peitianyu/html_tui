#define MINI_BROWSER_IMPLEMENTATION
#include "core/mini_browser.h"

typedef struct { const char* id; const char* msg; } BtnAction;

static const BtnAction btn_actions[] = {
    /* ── 03-inputs: Save/Clear/Reset ── */
    {"btn-clear",   "✓ All inputs cleared"},
    {"btn-reset-f",  "↺ Form reset to defaults"},
    {"btn-save",     "💾 Values saved"},

    /* ── 04-buttons ── */
    {"btn-primary",  "🔵 Primary clicked"},
    {"btn-secondary","🔘 Secondary clicked"},
    {"btn-success",  "🟢 Success clicked"},
    {"btn-warning",  "🟡 Warning clicked"},
    {"btn-danger",   "🔴 Danger clicked"},
    {"btn-hover-1",  "✨ Hover test 1"},
    {"btn-hover-2",  "✨ Hover test 2"},
    {"btn-hover-3",  "✨ Hover test 3"},
    {"btn-small",    "🔹 Small button"},
    {"btn-med",      "🔸 Medium button"},
    {"btn-large",    "🔶 Large button"},
    {"btn-block",    "▬ Block button"},
    {"btn-act-1",    "⚡ Test 1 triggered"},
    {"btn-act-2",    "⚡ Test 2 triggered"},
    {"btn-act-3",    "⚠ Danger action triggered"},

    /* ── 05-flex-direction ── */
    {"btn-nav-1",   "🏠 Home"},
    {"btn-nav-2",   "📊 Dashboard"},
    {"btn-nav-3",   "⚙ Settings"},
    {"btn-nav-4",   "👤 Profile"},
    {"btn-nav-5",   "🚪 Logout"},
    {"btn-cen-1",   "← Left"},
    {"btn-cen-2",   "⇔ Center"},
    {"btn-cen-3",   "→ Right"},
    {"btn-ev-1",    "One"},
    {"btn-ev-2",    "Two"},
    {"btn-ev-3",    "Three"},
    {"btn-ev-4",    "Four"},

    /* ── 09-lists-pseudo ── */
    {"btn-info",     "ℹ Pseudo-classes: :first-child :last-child :nth-child :hover"},
};

#define BTN_ACTIONS_COUNT ((int)(sizeof(btn_actions) / sizeof(btn_actions[0])))

/* 已知输入框 id 列表 (用于 btn-save 收集值) — 对应 03-inputs.html */
static const char* known_input_ids[] = {
    "inp-first", "inp-last", "inp-email", "inp-phone",
    "inp-pass", "inp-focus", "inp-empty",
};
#define KNOWN_INPUT_IDS_COUNT ((int)(sizeof(known_input_ids) / sizeof(known_input_ids[0])))

/* 用于 btn-clear 的输入框 id */
static const char* clear_ids[] = {
    "inp-first", "inp-last", "inp-email", "inp-phone",
    "inp-pass", "inp-focus", "inp-empty",
};
#define CLEAR_IDS_COUNT ((int)(sizeof(clear_ids) / sizeof(clear_ids[0])))

static bool demo_on_click(const char* btn_id, const char* btn_text,
                           MiniBrowser* mb, void* userdata)
{
    (void)userdata;

    if (btn_id && strcmp(btn_id, "btn-save") == 0) {
        char vals[512] = ""; int n = 0;
        for (int i = 0; i < KNOWN_INPUT_IDS_COUNT; i++) {
            const char* v = mb_get_text(mb, known_input_ids[i]);
            if (v && v[0] && v[0] != ' ') {
                char tmp[80];
                snprintf(tmp, sizeof(tmp), "%s%s", n > 0 ? ", " : "", v);
                strncat(vals, tmp, sizeof(vals) - strlen(vals) - 1);
                n++;
            }
        }
        mb_set_status(mb, n > 0 ? vals : "(no inputs)");
        return false;
    }

    /* ── btn-clear: 清空所有输入框 ── */
    if (btn_id && strcmp(btn_id, "btn-clear") == 0) {
        for (int i = 0; i < CLEAR_IDS_COUNT; i++)
            mb_set_text(mb, clear_ids[i], "");
        mb_set_status(mb, "✓ All inputs cleared");
        return true;
    }

    /* ── btn-reset-f: 重置所有输入框为空值 ── */
    if (btn_id && strcmp(btn_id, "btn-reset-f") == 0) {
        for (int i = 0; i < KNOWN_INPUT_IDS_COUNT; i++)
            mb_set_text(mb, known_input_ids[i], "");
    }
    
    /* ── 查表 ── */
    if (btn_id) {
        for (int i = 0; i < BTN_ACTIONS_COUNT; i++) {
            if (strcmp(btn_id, btn_actions[i].id) == 0) {
                mb_set_status(mb, btn_actions[i].msg);
                return false;
            }
        }
    }

    /* ── 默认: 显示按钮文本 ── */
    mb_set_status(mb, btn_text);
    return false;
}

/* 可点击元素回调: <a> 链接跳页, data-action 显示消息 */
static bool demo_on_element_click(const char* action, const char* element_id,
                                  const char* tag, MiniBrowser* mb, void* userdata)
{
    (void)element_id; (void)userdata;
    if (tag && strcmp(tag, "a") == 0 && action) {
        if (strstr(action, ".html")) {
            mb_switch_page(mb, action);
            return true;
        }
    }
    if (action)
        mb_set_status(mb, action);
    else
        mb_set_status(mb, "(click)");
    return false;
}

int main(int argc, char** argv) {
    const char* filepath = argc > 1 ? argv[1] : "pages/00-menu.html";
    /* 解析 --theme 参数 */
    const char* theme_path = NULL;
    for (int i = 1; i < argc - 1; i++)
        if (strcmp(argv[i], "--theme") == 0) theme_path = argv[i + 1];

    MB_Config config = MB_CONFIG_DEFAULT;
    config.on_button_click = demo_on_click;
    config.on_element_click = demo_on_element_click;
    config.show_scrollbars = true;
    config.show_statusbar = false;

    MiniBrowser* mb = mini_browser_open(filepath, &config);
    if (!mb) return 1;
    mb_apply_theme(mb, theme_path);

    mini_browser_run_loop(mb);
    mini_browser_close(mb);
    printf("Goodbye.\n");
    return 0;
}

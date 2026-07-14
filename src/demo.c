#define MINI_BROWSER_IMPLEMENTATION
#include "core/mini_browser.h"

typedef struct { const char* id; const char* msg; } BtnAction;

static const BtnAction btn_actions[] = {
    /* ── Form 页面 ── */
    {"btn-clear",   "✓ All inputs cleared"},
    {"btn-reset-f",  "↺ Form reset to defaults"},
    {"btn-test-1",  "⚡ Test 1 triggered"},
    {"btn-test-2",  "⚡ Test 2 triggered"},
    {"btn-test-3",  "⚠ Danger button pressed"},

    /* ── Flex 页面 ── */
    {"btn-nav-1",   "🏠 Home"},
    {"btn-nav-2",   "📊 Dashboard"},
    {"btn-nav-3",   "⚙ Settings"},
    {"btn-nav-4",   "👤 Profile"},
    {"btn-nav-5",   "🚪 Logout"},
    {"btn-center-act", "🎯 Centered button clicked"},
    {"btn-end-1",   "✏ Edit action"},
    {"btn-end-2",   "🗑 Delete action"},
    {"btn-end-3",   "📤 Export action"},
    {"btn-ev-1",    "One"},
    {"btn-ev-2",    "Two"},
    {"btn-ev-3",    "Three"},
    {"btn-ev-4",    "Four"},
    {"btn-vc-1",    "⬆ Up"},
    {"btn-vc-2",    "⬇ Down"},
    {"btn-card-a",  "✓ Detail clicked"},
    {"btn-card-b",  "✓ Detail clicked"},
    {"btn-card-c",  "✓ Detail clicked"},

    /* ── Selectors 页面 ── */
    {"btn-table-act","📊 Table refreshed"},
    {"btn-list-act", "📝 List updated"},
    {"btn-info",     "ℹ Pseudo-classes: :first-child :last-child :nth-child :hover"},
};

#define BTN_ACTIONS_COUNT ((int)(sizeof(btn_actions) / sizeof(btn_actions[0])))

/* 已知输入框 id 列表 (用于 btn-save 收集值) */
static const char* known_input_ids[] = {
    "input-name", "input-email", "input-pass", "input-age",
    "input-search", "input-tel", "input-url", "input-num",
    "input-date", "input-color",
};
#define KNOWN_INPUT_IDS_COUNT ((int)(sizeof(known_input_ids) / sizeof(known_input_ids[0])))

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
        for (int i = 0; i < KNOWN_INPUT_IDS_COUNT; i++)
            mb_set_text(mb, known_input_ids[i], "");
        for (int i = 0; i < BTN_ACTIONS_COUNT; i++) {
            if (strcmp(btn_actions[i].id, "btn-clear") == 0) {
                mb_set_status(mb, btn_actions[i].msg);
                return true;
            }
        }
        mb_set_status(mb, "✓ Cleared");
        return true;
    }

    /* ── btn-reset-f: 重置所有输入框为空值 ── */
    if (btn_id && strcmp(btn_id, "btn-reset-f") == 0) {
        for (int i = 0; i < KNOWN_INPUT_IDS_COUNT; i++)
            mb_set_text(mb, known_input_ids[i], "");
        for (int i = 0; i < BTN_ACTIONS_COUNT; i++) {
            if (strcmp(btn_actions[i].id, "btn-reset-f") == 0) {
                mb_set_status(mb, btn_actions[i].msg);
                return true;
            }
        }
        mb_set_status(mb, "↺ Reset");
        return true;
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

int main(int argc, char** argv) {
    const char* filepath = argc > 1 ? argv[1] : "pages/00-menu.html";

    MB_Config config = {
        .on_button_click = demo_on_click,
        .show_scrollbars = true,
    };

    MiniBrowser* mb = mini_browser_open(filepath, &config);
    if (!mb) return 1;

    mini_browser_run_loop(mb);
    mini_browser_close(mb);
    printf("Goodbye.\n");
    return 0;
}

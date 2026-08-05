#!/bin/bash
# tools/run_tests.sh — 无头测试套件 (样式 + 布局 + 互动)
# 用法: tools/run_tests.sh [page.html ...]   (默认全部)
# 依赖: gcc + RENDER_HEADLESS 模式; dump 到 $PREFIX/tmp
set -u
cd "$(dirname "$0")/.."

PREFIX_TMP="${PREFIX:-/data/data/com.termux/files/usr}/tmp"
HT="$PREFIX_TMP/headless_test"
IT="$PREFIX_TMP/interact_test"
FAILS=0
TOTAL=0

echo "== 编译测试工具 =="
gcc -DRENDER_HEADLESS -I src -o "$HT" tools/headless_test.c -lm 2>/dev/null || { echo "headless_test 编译失败"; exit 2; }
gcc -DRENDER_HEADLESS -I src -o "$IT" tools/interact_test.c -lm 2>/dev/null || { echo "interact_test 编译失败"; exit 2; }

# 断言辅助: check <工具> <页面> <断言...>
check() {
    local tool="$1" page="$2"; shift 2
    TOTAL=$((TOTAL+1))
    if "$tool" "$page" "$@" >/dev/null 2>&1; then
        echo "PASS  $page :: $*"
    else
        echo "FAIL  $page :: $*"
        FAILS=$((FAILS+1))
    fi
}

echo
echo "== 渲染 + 布局断言 (dump 一一对应) =="
if [ $# -gt 0 ]; then
    PAGES=("$@")
else
    PAGES=(pages/00-menu.html pages/01-text-colors.html pages/02-box-model.html pages/03-inputs.html \
           pages/04-buttons.html pages/05-flex-direction.html pages/06-flex-justify.html pages/07-flex-align-wrap.html \
           pages/08-tables.html pages/09-lists-pseudo.html pages/10-details-textarea.html pages/11-select.html \
           pages/12-css-effects.html pages/13-input-types.html pages/14-text-styles.html pages/15-clickable.html \
           pages/16-theme.html pages/17-edge.html pages/18-media.html \
           pages/19-many.html pages/20-external-css.html pages/21-image.html \
           pages/22-position.html)
fi
for p in "${PAGES[@]}"; do
    case "$p" in
    pages/00-menu.html)
        check "$HT" "$p" --layout \
            --expect='Mini Browser' --expect='Text & Colors' --expect='Box Model' \
            --expect='abs=(0,0) content=80x'
        check "$HT" "$p" --flush-ok ;;
    pages/01-text-colors.html)
        check "$HT" "$p" --layout \
            --expect='Test 01' --expect='color=212,69,69' --expect='bold' --expect='ul=1' \
            --expect='italic' ;;
    pages/02-box-model.html)
        check "$HT" "$p" --layout \
            --expect='Box Model' --expect='pad=2,2,2,2' --expect='border=1,1,1,1' \
            --expect='pad=1,4,2,1' ;;
    pages/03-inputs.html)
        check "$HT" "$p" --layout \
            --expect='inp-first' --expect='<input id="inp-first"' --expect='type="password"' ;;
    pages/04-buttons.html)
        check "$HT" "$p" --layout \
            --expect='btn-primary' --expect='bold' --expect='bg=60,60,80' ;;
    pages/05-flex-direction.html)
        check "$HT" "$p" --layout \
            --expect='flex-dir=1' --expect='justify=' --expect='gap=' ;;
    pages/06-flex-justify.html)
        check "$HT" "$p" --layout \
            --expect='justify=' --expect='align=' --expect='content=80x' ;;
    pages/07-flex-align-wrap.html)
        check "$HT" "$p" --layout \
            --expect='align=' --expect='gap=' --expect='flex-dir=0' ;;
    pages/08-tables.html)
        check "$HT" "$p" --layout \
            --expect='table' --expect='<tr>' --expect='<td>' --expect='<th>' ;;
    pages/09-lists-pseudo.html)
        check "$HT" "$p" --layout \
            --expect='<li>' --expect='ul=1' --expect='list=1' ;;
    pages/10-details-textarea.html)
        check "$HT" "$p" --layout \
            --expect='<summary>' --expect='<textarea id=' ;;
    pages/11-select.html)
        check "$HT" "$p" --layout \
            --expect='<option>' --expect='pad=0,1,0,1' ;;
    pages/12-css-effects.html)
        check "$HT" "$p" --layout \
            --expect='border=1,1,1,1' --expect='z=' --expect='pad=2,2,2,2'
        check "$HT" "$p" --layout --viewport=40x60 \
            --expect-line='narrow-only|color=241,196,15|bold' \
            --expect-line='wide-only|content=0x0'
        check "$HT" "$p" --layout --viewport=120x60 \
            --expect-line='narrow-only|content=0x0' ;;
    pages/13-input-types.html)
        check "$HT" "$p" --layout \
            --expect='type="checkbox"' --expect='type="radio"' --expect='type="range"' ;;
    pages/14-text-styles.html)
        check "$HT" "$p" --layout \
            --expect='ul=3' --expect='bg=' --expect='color=212,69,69' ;;
    pages/15-clickable.html)
        check "$HT" "$p" --layout \
            --expect='clickable action="open-card-1"' \
            --expect='clickable action="toggle-card-2"' \
            --expect='clickable action="card-3"' \
            --expect='disabled' \
            --expect='clickable action="pages/01-text-colors.html"' \
            --expect='color=74,158,255' ;;
    pages/16-theme.html)
        check "$HT" "$p" --layout \
            --expect='16-theme' --expect='placeholder' \
            --expect='border=1,1,1,1' ;;
    pages/17-edge.html)
        check "$HT" "$p" --layout \
            --expect-line='negmargin|margin=2,0,0,-3' \
            --expect-line='autocenter|margin=0,18,0,18|abs=(20,' \
            --expect-line='border-box|content=46x1' \
            --expect-line='content-box|content=50x1' \
            --expect-line='clip|content=20x3' \
            --expect-line='nested|flex-dir=1' \
            --expect-line='rel|abs=(5,50)' \
            --expect-line='narrow-max|content=20x1' \
            --expect-line='trunc|content=30x1' ;;
    pages/18-media.html)
        check "$HT" "$p" --layout --viewport=30x40 \
            --expect-line='narrow-label|color=212,69,69|bold' \
            --expect-line='wide-label|content=0x0'
        check "$HT" "$p" --layout --viewport=80x40 \
            --expect-line='wide-label|color=74,158,255|bold' \
            --expect-line='narrow-label|content=0x0'
        check "$HT" "$p" --layout --viewport=50x40 \
            --expect-line='mid-range|bg=51,47,26' ;;
    pages/19-many.html)
        check "$HT" "$p" --layout --viewport=80x150 \
            --expect='B0' --expect='B299' --expect='btn-back' \
            --expect-count=300:class=\"btn\"
        check "$HT" "$p" --flush-ok ;;
    pages/20-external-css.html)
        check "$HT" "$p" --layout \
            --expect='ext-box' --expect='bg=18,52,86' \
            --expect='border=1,1,1,1' \
            --expect-line='inline-late|bg=0,255,0'
        check "$HT" "$p" --flush-ok ;;
    pages/21-image.html)
        check "$HT" "$p" --layout \
            --expect='[fallback-text]' \
            --expect-cell='1,2=fg:255,0,0;bg:0,255,0' \
            --expect-cell='5,4=fg:0,0,255;bg:255,255,255' \
            --expect-cell='1,7=fg:0,0,0;bg:255,255,255'
        check "$HT" "$p" --flush-ok ;;
    pages/22-position.html)
        check "$HT" "$p" --layout \
            --expect-line='float-1|abs=(5,3)' \
            --expect-line='float-2|abs=(2,5)' \
            --expect-line='float-br|abs=(78,59)'
        check "$HT" "$p" --flush-ok ;;
    esac
done

echo
echo "== 互动断言 (事件注入) =="
check "$IT" pages/15-clickable.html --events 'tab enter' \
    --expect='el[action=open-card-1 id=card-1 tag=div]'
check "$IT" pages/15-clickable.html --events 'tab tab tab enter' \
    --expect='el[action=card-3 id=card-3 tag=div]'
check "$IT" pages/15-clickable.html --events 'click:20,33' \
    --expect='el[action=toggle-card-2 id=card-2 tag=div]'
check "$IT" pages/15-clickable.html --events 'tab tab tab tab enter' \
    --expect='el[action=pages/01-text-colors.html id=- tag=a]'
check "$IT" pages/15-clickable.html --events 'esc' \
    --expect-color='pages/15-clickable.html|74,158,255'
check "$IT" pages/15-clickable.html --events 'tab tab tab tab tab enter' \
    --expect-color='pages/15-clickable.html|155,89,182' \
    --expect='el[action=pages/15-clickable.html id=- tag=a]'
check "$IT" pages/15-clickable.html --events 'motion:10,21' \
    --expect-bg='card|51,68,85'
check "$IT" pages/15-clickable.html --events 'motion:10,21 motion:50,55' \
    --expect-bg='card|34,34,51'
check "$IT" pages/15-clickable.html --events 'btab enter' \
    --expect='el[action=pages/15-clickable.html id=- tag=a]'
check "$IT" pages/15-clickable.html --events 'key:R' \
    --expect='theme'
check "$IT" pages/03-inputs.html --events 'tab key:X key:Y key:Z' \
    --expect-text='inp-first|JohnXYZ'
check "$IT" pages/03-inputs.html --events 'tab tab tab tab tab key:X key:Y' \
    --expect-text='inp-pass|secret123XY'
check "$IT" pages/03-inputs.html --events 'tab key:X keycode:8' \
    --expect-text='inp-first|John'
check "$IT" pages/03-inputs.html --events 'tab key:X key:Y tab tab tab tab tab tab tab enter' \
    --expect='saved: JohnXY' --expect='btn[id=btn-save'
check "$IT" pages/10-details-textarea.html --events 'tab enter' \
    --expect='Expanded'
check "$IT" pages/10-details-textarea.html --events 'tab tab tab tab key:H key:i enter key:!' \
    --expect-text='ta-std|Hi'
check "$IT" pages/11-select.html --events 'tab enter keycode:65516 enter' \
    --expect='Option B' --expect='Selected'
check "$IT" pages/13-input-types.html --events 'tab enter' \
    --expect-text='chk-a|[ ]'
check "$IT" pages/13-input-types.html --events 'tab enter enter' \
    --expect-text='chk-a|[x]'
check "$IT" pages/04-buttons.html --events 'motion:16,10' \
    --expect-bg='btn-primary|232,101,101'
check "$IT" pages/04-buttons.html --events 'motion:16,10 motion:50,50' \
    --expect-bg='btn-primary|212,69,69'
check "$IT" pages/15-clickable.html --events 'tab tab tab tab enter b' \
    --expect='15-clickable'
check "$IT" pages/15-clickable.html --events 'b' \
    --expect='first page'
check "$IT" pages/03-inputs.html --events 'tab b b' \
    --expect-text='inp-first|Johnbb'
check "$IT" pages/19-many.html --events 'tabx:300 enter' \
    --expect='btn[id=btn-299 text=B299]'
check "$IT" pages/19-many.html --events 'tabx:301' \
    --expect='button (#300)'

echo
echo "== 结果: $TOTAL 项, $FAILS 失败 =="
exit $((FAILS > 0))

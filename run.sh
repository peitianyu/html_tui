#!/bin/bash
# Mini Browser - load HTML files from disk
# Usage: ./run.sh [path/to/page.html]
#   Default: pages/00-menu.html
#   Use src/demo_simple.c for the simplest setup (just mini_browser.h)
#   Use src/demo_counter.c for custom callback example
#
# Supports cosmocc (Cosmopolitan), tcc (Tiny C Compiler) and gcc.
PAGE="${1:-pages/00-menu.html}"

if command -v tcc &>/dev/null; then
    tcc -I src -o mini_browser.exe src/demo.c -lm && exec ./mini_browser.exe "$PAGE"
fi

# cosmocc: 生成 Actually Portable Executable (.com)
if command -v cosmocc &>/dev/null; then
    cosmocc -x c -I src -o demo.exe src/demo.c
fi

# Try gcc (MinGW) → .exe
if command -v gcc &>/dev/null; then
    gcc -I src -o mini_browser.exe src/demo.c -lm && exec ./mini_browser.exe "$PAGE"
fi

echo "No compiler found (tried: cosmocc, tcc, gcc)" >&2
exit 1

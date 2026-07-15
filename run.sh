#!/bin/bash
# Mini Browser - load HTML files from disk
# Usage: ./run.sh [path/to/page.html]
#   Default: pages/00-menu.html
#   Use src/demo_simple.c for the simplest setup (just mini_browser.h)
#   Use src/demo_counter.c for custom callback example
#
# Supports both tcc (Tiny C Compiler) and gcc.
PAGE="${1:-pages/00-menu.html}"

if command -v tcc &>/dev/null; then
    # tcc auto-links math library, no -lm needed
    exec tcc -I src -run src/demo.c "$PAGE" -lm
fi

# Fallback to gcc
gcc -I src -o /tmp/mini_browser src/demo.c -lm && exec /tmp/mini_browser "$PAGE"

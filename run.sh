#!/bin/bash
# Mini Browser - load HTML files from disk
# Usage: ./run.sh [path/to/page.html]
#   Default: pages/00-menu.html
#   Use src/demo_simple.c for the simplest setup (just mini_browser.h)
#   Use src/demo_counter.c for custom callback example
PAGE="${1:-pages/00-menu.html}"
tcc -I src -run src/demo.c "$PAGE"

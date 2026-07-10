#!/bin/bash
# Mini Browser - load HTML files from disk
# Usage: ./run.sh [path/to/page.html]
#   Default: pages/00-menu.html
PAGE="${1:-pages/00-menu.html}"
tcc -I src -run src/demo_main.c "$PAGE"

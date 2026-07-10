/*
 * demo_main.c - Interactive TUI browser: HTML+CSS → Layout → Termbox2
 *
 * Usage: tcc src/demo_main.c -I src -run [page.html]
 *   If no page is given, loads pages/00-menu.html
 *
 * Each .html file contains both HTML structure and inline <style> CSS.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define UC_IMPLEMENTATION
#include "core/uc.h"

#define STYLETREE_IMPLEMENTATION
#include "core/styletree.h"

/* Undef katana.h's flex ECHO macro to avoid redefinition warning
   when termbox2.h (via render.h) includes <termios.h> which also defines ECHO. */
#undef ECHO

#define LAYOUT_IMPLEMENTATION
#include "core/layout.h"

#define RENDER_IMPLEMENTATION
#include "core/render.h"

#define INTERACT_IMPLEMENTATION
#include "demo_page.h"

int main(int argc, char** argv) {
    const char* filepath = argc > 1 ? argv[1] : "pages/00-menu.html";
    demo_run(filepath);
    return 0;
}

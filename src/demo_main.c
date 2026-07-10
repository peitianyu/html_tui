/*
 * demo_main.c - Interactive TUI browser: HTML+CSS → Layout → Termbox2
 *
 * This is a thin entry point. Edit demo_page.h to change page content.
 *
 * Build: tcc src/demo_main.c -I src -run
 */

#include <stdio.h>
#include <string.h>

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

int main(void) {
    demo_run();
    return 0;
}

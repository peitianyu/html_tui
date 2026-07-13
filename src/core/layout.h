#ifndef LAYOUT_H
#define LAYOUT_H

#include "styletree.h"
#include "uc.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Data Structures ======================== */

/** Display types */
typedef enum {
    DISPLAY_NONE = 0,
    DISPLAY_BLOCK,
    DISPLAY_INLINE,
    DISPLAY_FLEX,
    DISPLAY_GRID,
    DISPLAY_TABLE,
    DISPLAY_TABLE_ROW
} DisplayType;

/** Flex direction */
typedef enum {
    FLEX_ROW = 0,
    FLEX_COLUMN
} FlexDirection;

/** Justify content */
typedef enum {
    JUSTIFY_START = 0,
    JUSTIFY_CENTER,
    JUSTIFY_END,
    JUSTIFY_SPACE_BETWEEN,
    JUSTIFY_SPACE_AROUND,
    JUSTIFY_SPACE_EVENLY
} JustifyContent;

/** Align items */
typedef enum {
    ALIGN_START = 0,
    ALIGN_CENTER,
    ALIGN_END,
    ALIGN_STRETCH
} AlignItems;

/** A resolved color */
typedef struct {
    int r, g, b;
    bool valid;  /* false if not set */
} ResolvedColor;

/** A layout node: styled node with computed dimensions and positions */
typedef struct LayoutNode {
    /* source */
    StyledNode* styled;

    /* display */
    DisplayType display;

    /* computed position (relative to parent top-left) */
    int x, y;

    /* computed dimensions (content area, excluding padding/border/margin) */
    int width, height;

    /* box model (all in character cells) */
    int padding_top, padding_right, padding_bottom, padding_left;
    int margin_top, margin_right, margin_bottom, margin_left;
    bool margin_left_auto, margin_right_auto; /* for margin: auto centering */
    int border_top, border_right, border_bottom, border_left;

    /* text content extracted from DOM subtree */
    char* text_content;

    /* computed colors */
    ResolvedColor color;
    ResolvedColor bg_color;
    ResolvedColor border_color;

    /* text styling */
    bool font_bold;
    int font_underline; /* 0=none, 1=underline, 2=overline, 3=line-through */

    /* vertical-align: 0=baseline(default), 1=top, 2=middle, 3=bottom */
    int vertical_align;

    /* text-transform: 0=none, 1=uppercase, 2=lowercase, 3=capitalize */
    int text_transform;

    /* line-height in character rows (0 = default = 1) */
    int line_height;

    /* letter-spacing in character cells */
    int letter_spacing;

    /* word-spacing in character cells */
    int word_spacing;

    /* text alignment */
    int text_align; /* 0=left, 1=center, 2=right */

    /* flex layout properties */
    FlexDirection flex_direction;
    JustifyContent justify_content;
    AlignItems align_items;
    int gap;             /* gap between flex items */
    int flex_grow;
    int flex_shrink;
    int flex_basis;

    /* truncate overflow text (single-line, no wrap) */
    bool truncate_overflow;

    /* border style: 0=none, 1=solid, 2=dashed, 3=dotted, 4=double, 5=heavy, 6=rounded */
    int border_style;

    /* whitespace preservation (like <pre>) */
    bool preserve_ws;

    /* list marker type: 0=none, 1=bullet (•), 2=ordered (1.) */
    int list_marker;
    /* list item index (1-based, for ordered lists) */
    int list_index;

    /* overflow hidden */
    bool overflow_hidden;

    /* max-width / min-width / max-height / min-height (0 = not set) */
    int max_width, min_width, max_height, min_height;

    /* box-sizing: 0=content-box, 1=border-box */
    int box_sizing;

    /* outline width (character cells) */
    int outline_width;
    ResolvedColor outline_color;

    /* visibility: hidden — element takes space but is invisible */
    bool visibility_hidden;

    /* details/summary: collapsed state */
    bool collapsed;
    bool is_summary;
    bool details_open;

    /* flex-wrap: 0=nowrap, 1=wrap */
    int flex_wrap;

    /* position: 0=static, 1=relative */
    int position_type;
    int position_top, position_right, position_bottom, position_left;

    /* scroll offset within element's content (e.g. textarea) */
    int content_scroll_y;
    int content_scroll_x;

    /* parent pointer for absolute position computation */
    struct LayoutNode* parent;

    /* children */
    struct LayoutNode** children;
    size_t num_children;
} LayoutNode;

/* ======================== Global Details State API ======================== */
/* Used by interact.h to persist details open/close across layout rebuilds */
void details_set_state(const char* id, bool open);
bool details_get_state(const char* id);

/* ======================== API ======================== */

/**
 * Build a layout tree from a style tree.
 * @param styled_root  root of the style tree (usually the <html> element)
 * @param viewport_w   viewport width in character cells
 * @param viewport_h   viewport height in character cells
 * @return root layout node, or NULL on failure
 */
LayoutNode* build_layout_tree(StyledNode* styled_root, int viewport_w, int viewport_h);

/** Free a layout tree */
void free_layout_tree(LayoutNode* tree);

/** Debug: print the layout tree */
void debug_print_layout(LayoutNode* node, int indent);

#ifdef __cplusplus
}
#endif

/* ==================================================================== */
/* IMPLEMENTATION                                                       */
/* ==================================================================== */
#ifdef LAYOUT_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* ---------- Global details state (toggling across rebuilds) ---------- */
#define MAX_DETAILS_STATES 32
static struct { char id[64]; bool open; } g_det_state[MAX_DETAILS_STATES];
static int g_det_count = 0;

void details_set_state(const char* id, bool open) {
    if (!id) return;
    for (int i = 0; i < g_det_count; i++) {
        if (strcmp(g_det_state[i].id, id) == 0) {
            g_det_state[i].open = open; return;
        }
    }
    if (g_det_count < MAX_DETAILS_STATES) {
        strncpy(g_det_state[g_det_count].id, id, 63);
        g_det_state[g_det_count].open = open;
        g_det_count++;
    }
}

bool details_get_state(const char* id) {
    if (!id) return true;
    for (int i = 0; i < g_det_count; i++) {
        if (strcmp(g_det_state[i].id, id) == 0)
            return g_det_state[i].open;
    }
    return true; /* default open */
}

/* ---------- value parsing helpers ---------- */

/** Parse a CSS dimension string like "100", "100px", "50%", "auto" */
typedef struct {
    float value;
    int unit;  /* 0=px, 1=percent, 2=auto */
    bool valid;
} ParsedDim;

static ParsedDim parse_dimension(const char* str) {
    ParsedDim d = {0, 0, false};
    if (!str || !*str) return d;

    /* trim leading spaces */
    while (*str == ' ' || *str == '\t') str++;

    if (strcmp(str, "auto") == 0) {
        d.value = 0;
        d.unit = 2;
        d.valid = true;
        return d;
    }

    /* try to parse number */
    char* end = NULL;
    float val = strtof(str, &end);
    if (end == str) return d; /* not a number */

    d.value = val;

    /* skip spaces after number */
    while (*end == ' ' || *end == '\t') end++;

    if (*end == '%') {
        d.unit = 1; /* percent */
        d.valid = true;
    } else if (strncmp(end, "px", 2) == 0) {
        d.unit = 0; /* px */
        d.valid = true;
    } else if (*end == '\0' || isalpha(*end)) {
        /* just a number (no unit) or unknown unit - treat as px */
        d.unit = 0; /* px */
        d.valid = true;
    }

    return d;
}

/** Parse a shorthand value like "10" or "10 20" or "10 20 30 40" into 4 values */
static void parse_box_shorthand(const char* str, int* top, int* right, int* bottom, int* left) {
    *top = *right = *bottom = *left = 0;
    if (!str || !*str) return;

    /* tokenize by space */
    const char* p = str;
    float vals[4];
    int count = 0;

    while (*p && count < 4) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        char* end;
        float v = strtof(p, &end);
        vals[count++] = v;
        p = end;
    }

    int r[4];
    switch (count) {
        case 1: r[0] = r[1] = r[2] = r[3] = (int)vals[0]; break;
        case 2: r[0] = r[2] = (int)vals[0]; r[1] = r[3] = (int)vals[1]; break;
        case 3: r[0] = (int)vals[0]; r[1] = r[3] = (int)vals[1]; r[2] = (int)vals[2]; break;
        case 4: r[0] = (int)vals[0]; r[1] = (int)vals[1]; r[2] = (int)vals[2]; r[3] = (int)vals[3]; break;
        default: return;
    }

    *top = r[0];
    *right = r[1];
    *bottom = r[2];
    *left = r[3];
}

/** Resolve a dimension given a parent size and parsed dim */
static int resolve_dim(ParsedDim dim, int parent_size, int default_px) {
    if (!dim.valid) return default_px;
    if (dim.unit == 1) return (int)(parent_size * dim.value / 100.0f); /* percent */
    return (int)dim.value; /* px or auto (auto uses the value 0, caller handles) */
}

/** Parse a hex color string like "#ff6600" or "#f60" or named color */
static ResolvedColor parse_color(const char* str) {
    ResolvedColor c = {0, 0, 0, false};
    if (!str || !*str) return c;

    while (*str == ' ' || *str == '\t') str++;

    /* named colors */
    if (strcmp(str, "red") == 0)      { c.r = 255; c.g = 0;   c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "green") == 0)    { c.r = 0;   c.g = 128; c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "blue") == 0)     { c.r = 0;   c.g = 0;   c.b = 255; c.valid = true; return c; }
    if (strcmp(str, "white") == 0)    { c.r = 255; c.g = 255; c.b = 255; c.valid = true; return c; }
    if (strcmp(str, "black") == 0)    { c.r = 0;   c.g = 0;   c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "gray") == 0 || strcmp(str, "grey") == 0)
                                      { c.r = 128; c.g = 128; c.b = 128; c.valid = true; return c; }
    if (strcmp(str, "yellow") == 0)   { c.r = 255; c.g = 255; c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "cyan") == 0)     { c.r = 0;   c.g = 255; c.b = 255; c.valid = true; return c; }
    if (strcmp(str, "magenta") == 0)  { c.r = 255; c.g = 0;   c.b = 255; c.valid = true; return c; }
    if (strcmp(str, "silver") == 0)   { c.r = 192; c.g = 192; c.b = 192; c.valid = true; return c; }
    if (strcmp(str, "maroon") == 0)   { c.r = 128; c.g = 0;   c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "olive") == 0)    { c.r = 128; c.g = 128; c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "navy") == 0)     { c.r = 0;   c.g = 0;   c.b = 128; c.valid = true; return c; }
    if (strcmp(str, "purple") == 0)   { c.r = 128; c.g = 0;   c.b = 128; c.valid = true; return c; }
    if (strcmp(str, "teal") == 0)     { c.r = 0;   c.g = 128; c.b = 128; c.valid = true; return c; }
    if (strcmp(str, "orange") == 0)   { c.r = 255; c.g = 165; c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "pink") == 0)     { c.r = 255; c.g = 192; c.b = 203; c.valid = true; return c; }
    if (strcmp(str, "lime") == 0)     { c.r = 0;   c.g = 255; c.b = 0;   c.valid = true; return c; }
    if (strcmp(str, "transparent") == 0) { c.valid = false; return c; }
    if (strcmp(str, "currentColor") == 0) { c.valid = false; return c; /* resolved at apply_styles */ }

    /* rgb(r,g,b) or rgba(r,g,b,a) */
    if (strncmp(str, "rgb", 3) == 0) {
        const char* p = strchr(str, '(');
        if (p) {
            p++;
            int vals[4] = {0,0,0,255};
            int cnt = 0;
            while (*p && cnt < 4) {
                while (*p == ' ' || *p == '\t' || *p == ',' || *p == '(') p++;
                if (!*p || *p == ')') break;
                char* end = NULL;
                long v = strtol(p, &end, 10);
                if (end != p) { vals[cnt++] = (int)v; p = end; }
                else break;
                if (*p == '%') { vals[cnt-1] = vals[cnt-1] * 255 / 100; p++; }
            }
            if (cnt >= 3) {
                c.r = vals[0]; c.g = vals[1]; c.b = vals[2];
                c.valid = true;
                return c;
            }
        }
    }

    /* hsl(h,s,l) or hsla(h,s,l,a) — convert HSL to RGB */
    if (strncmp(str, "hsl", 3) == 0) {
        const char* p = strchr(str, '(');
        if (p) {
            p++;
            double vals[4] = {0,0,0,1.0};
            int cnt = 0;
            while (*p && cnt < 4) {
                while (*p == ' ' || *p == '\t' || *p == ',' || *p == '(') p++;
                if (!*p || *p == ')') break;
                char* end = NULL;
                double v = strtod(p, &end);
                if (end != p) {
                    if (cnt == 0) { /* H — could be angle */
                        if (*end == 'd' && end[1] == 'e' && end[2] == 'g') { p = end + 3; }
                        else p = end;
                    } else { /* S, L — check % */
                        p = end;
                        if (*p == '%') { v /= 100.0; p++; }
                    }
                    vals[cnt++] = v;
                } else break;
            }
            if (cnt >= 3) {
                double h = vals[0], s = vals[1], l = vals[2];
                /* Normalize H to [0,360) */
                h = h - 360.0 * floor(h / 360.0);
                /* HSL → RGB conversion */
                double C = (1.0 - fabs(2.0 * l - 1.0)) * s;
                double X = C * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
                double m = l - C / 2.0;
                double r1, g1, b1;
                if (h < 60)       { r1 = C; g1 = X; b1 = 0; }
                else if (h < 120) { r1 = X; g1 = C; b1 = 0; }
                else if (h < 180) { r1 = 0; g1 = C; b1 = X; }
                else if (h < 240) { r1 = 0; g1 = X; b1 = C; }
                else if (h < 300) { r1 = X; g1 = 0; b1 = C; }
                else              { r1 = C; g1 = 0; b1 = X; }
                c.r = (int)((r1 + m) * 255.0 + 0.5);
                c.g = (int)((g1 + m) * 255.0 + 0.5);
                c.b = (int)((b1 + m) * 255.0 + 0.5);
                c.valid = true;
                return c;
            }
        }
    }

    /* #RRGGBB or #RGB */
    if (str[0] == '#') {
        unsigned int r = 0, g = 0, b = 0;
        int len = (int)strlen(str + 1);
        const char* hex = str + 1;
        if (len == 6) {
            sscanf(hex, "%02x%02x%02x", &r, &g, &b);
            c.r = (int)r; c.g = (int)g; c.b = (int)b;
            c.valid = true;
        } else if (len == 3) {
            sscanf(hex, "%1x%1x%1x", &r, &g, &b);
            c.r = (int)(r * 17); c.g = (int)(g * 17); c.b = (int)(b * 17);
            c.valid = true;
        }
    } else {
        /* bare hex like "ff0000" or "888" (Katana strips #) */
        int len = (int)strlen(str);
        if (len == 6 || len == 3) {
            bool all_hex = true;
            for (int i = 0; i < len; i++) {
                if (!isxdigit((unsigned char)str[i])) { all_hex = false; break; }
            }
            if (all_hex) {
                unsigned int r = 0, g = 0, b = 0;
                if (len == 6) {
                    sscanf(str, "%02x%02x%02x", &r, &g, &b);
                    c.r = (int)r; c.g = (int)g; c.b = (int)b;
                } else {
                    sscanf(str, "%1x%1x%1x", &r, &g, &b);
                    c.r = (int)(r * 17); c.g = (int)(g * 17); c.b = (int)(b * 17);
                }
                c.valid = true;
            }
        }
    }

    return c;
}

/* ---------- display type helpers ---------- */

/** Get default display type from tag name */
static DisplayType get_default_display(const char* tagname) {
    if (!tagname) return DISPLAY_BLOCK;

    /* block elements */
    if (strcmp(tagname, "div") == 0)    return DISPLAY_BLOCK;
    if (strcmp(tagname, "p") == 0)      return DISPLAY_BLOCK;
    if (strcmp(tagname, "h1") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "h2") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "h3") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "h4") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "h5") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "h6") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "li") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "ul") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "ol") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "header") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "footer") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "section") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "article") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "nav") == 0)    return DISPLAY_BLOCK;
    if (strcmp(tagname, "body") == 0)   return DISPLAY_BLOCK;
    if (strcmp(tagname, "html") == 0)   return DISPLAY_BLOCK;
    if (strcmp(tagname, "main") == 0)   return DISPLAY_BLOCK;
    if (strcmp(tagname, "form") == 0)   return DISPLAY_BLOCK;
    if (strcmp(tagname, "hr") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "pre") == 0)    return DISPLAY_BLOCK;
    if (strcmp(tagname, "table") == 0)  return DISPLAY_TABLE;
    if (strcmp(tagname, "tr") == 0)     return DISPLAY_TABLE_ROW;
    if (strcmp(tagname, "td") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "th") == 0)     return DISPLAY_BLOCK;
    if (strcmp(tagname, "details") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "summary") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "textarea") == 0) return DISPLAY_BLOCK;
    if (strcmp(tagname, "select") == 0)  return DISPLAY_BLOCK;
    if (strcmp(tagname, "option") == 0)  return DISPLAY_NONE;

    /* inline elements */
    if (strcmp(tagname, "span") == 0)   return DISPLAY_INLINE;
    if (strcmp(tagname, "a") == 0)      return DISPLAY_INLINE;
    if (strcmp(tagname, "em") == 0)     return DISPLAY_INLINE;
    if (strcmp(tagname, "strong") == 0) return DISPLAY_INLINE;
    if (strcmp(tagname, "b") == 0)      return DISPLAY_INLINE;
    if (strcmp(tagname, "i") == 0)      return DISPLAY_INLINE;
    if (strcmp(tagname, "u") == 0)      return DISPLAY_INLINE;
    if (strcmp(tagname, "code") == 0)   return DISPLAY_INLINE;
    if (strcmp(tagname, "small") == 0)  return DISPLAY_INLINE;
    if (strcmp(tagname, "br") == 0)     return DISPLAY_INLINE;
    if (strcmp(tagname, "img") == 0)    return DISPLAY_INLINE;
    if (strcmp(tagname, "input") == 0)  return DISPLAY_INLINE;
    if (strcmp(tagname, "button") == 0) return DISPLAY_INLINE;

    return DISPLAY_BLOCK; /* default */
}

/** Parse display string to enum */
static DisplayType parse_display(const char* str) {
    if (!str) return DISPLAY_BLOCK;
    if (strcmp(str, "none") == 0)      return DISPLAY_NONE;
    if (strcmp(str, "block") == 0)     return DISPLAY_BLOCK;
    if (strcmp(str, "inline") == 0)    return DISPLAY_INLINE;
    if (strcmp(str, "inline-block") == 0) return DISPLAY_BLOCK; /* treat as block */
    if (strcmp(str, "flex") == 0)      return DISPLAY_FLEX;
    if (strcmp(str, "grid") == 0)      return DISPLAY_GRID;
    if (strcmp(str, "table") == 0)    return DISPLAY_TABLE;
    if (strcmp(str, "table-row") == 0) return DISPLAY_TABLE_ROW;
    return DISPLAY_BLOCK;
}

/* ---------- text extraction ---------- */

/** Recursively extract text content from a DOM node.
 *  Only collects direct TEXT nodes and <br> elements.
 *  Text from child ELEMENTs belongs to those elements, not the parent. */
static char* extract_text(GumboNode* node) {
    if (!node) return NULL;

    if (node->type == GUMBO_NODE_TEXT) {
        return strdup(node->v.text.text);
    }

    if (node->type == GUMBO_NODE_ELEMENT) {
        /* skip script/style elements */
        GumboTag tag = node->v.element.tag;
        if (tag == GUMBO_TAG_SCRIPT || tag == GUMBO_TAG_STYLE || tag == GUMBO_TAG_HEAD ||
            tag == GUMBO_TAG_TITLE || tag == GUMBO_TAG_META || tag == GUMBO_TAG_LINK) {
            return NULL;
        }

        /* <br> becomes newline */
        if (tag == GUMBO_TAG_BR) {
            return strdup("\n");
        }

        /* Only collect direct TEXT nodes and <br>; skip ELEMENT children */
        GumboVector* children = &node->v.element.children;
        size_t total_len = 0;

        /* first pass: calculate total length from text/br/inline children */
        char** pieces = (char**)calloc(children->length, sizeof(char*));
        size_t piece_count = 0;
        for (unsigned int i = 0; i < children->length; i++) {
            GumboNode* child = (GumboNode*)children->data[i];
            if (child->type == GUMBO_NODE_TEXT) {
                char* text = strdup(child->v.text.text);
                pieces[piece_count++] = text;
                total_len += strlen(text);
            } else if (child->type == GUMBO_NODE_ELEMENT &&
                       child->v.element.tag == GUMBO_TAG_BR) {
                char* text = strdup("\n");
                pieces[piece_count++] = text;
                total_len += 1;
            }
            /* skip other ELEMENT nodes */
        }

        if (total_len == 0) {
            free(pieces);
            return NULL;
        }

        /* second pass: combine */
        char* result = (char*)malloc(total_len + 1);
        result[0] = '\0';
        for (size_t i = 0; i < piece_count; i++) {
            strcat(result, pieces[i]);
            free(pieces[i]);
        }
        free(pieces);
        return result;
    }

    return NULL;
}

/* ---------- UTF-8 helpers (using uc.h) ---------- */

/* forwards */
static void compute_child_layouts(LayoutNode* parent, int content_w);
static LayoutNode* build_layout_tree_recursive(StyledNode* snode, LayoutNode* parent,
                                               int viewport_w, int viewport_h);

/* ---------- style application ---------- */

/** Apply computed styles from StyledNode to LayoutNode */
static bool is_css_keyword(const char* val) {
    return val && (strcmp(val, "inherit") == 0 || strcmp(val, "initial") == 0 || strcmp(val, "unset") == 0);
}

static void apply_styles(LayoutNode* ln) {
    if (!ln || !ln->styled) return;
    StyledNode* sn = ln->styled;

    /* display */
    const char* disp = get_style(sn, "display");
    ln->display = disp ? parse_display(disp) : DISPLAY_BLOCK;

    /* padding */
    const char* pad = get_style(sn, "padding");
    if (pad) parse_box_shorthand(pad, &ln->padding_top, &ln->padding_right,
                                  &ln->padding_bottom, &ln->padding_left);

    /* margin */
    const char* mar = get_style(sn, "margin");
    if (mar) parse_box_shorthand(mar, &ln->margin_top, &ln->margin_right,
                                  &ln->margin_bottom, &ln->margin_left);
    /* Detect auto margins */
    ln->margin_left_auto = ln->margin_right_auto = false;
    if (mar) {
        const char* mp = mar; while (*mp == ' '||*mp=='\t') mp++;
        if (strcmp(mp, "auto") == 0) { ln->margin_left_auto = ln->margin_right_auto = true; }
        else {
            /* Try tokenizing and checking each value */
            int cnt = 0; while (*mp) {
                while (*mp == ' '||*mp=='\t') mp++;
                if (!*mp) break;
                if (mp[0] == 'a' && mp[1] == 'u' && mp[2] == 't' && mp[3] == 'o' && (mp[4]==' '||mp[4]=='\t'||!mp[4])) {
                    if (cnt == 0 || cnt == 1) ln->margin_left_auto = true;
                    if (cnt == 1 || cnt == 3) ln->margin_right_auto = true;
                    while (*mp && *mp != ' ' && *mp != '\t') mp++;
                } else {
                    while (*mp && *mp != ' ' && *mp != '\t') mp++;
                }
                cnt++;
            }
        }
    }

    /* overflow */
    const char* ov = get_style(sn, "overflow");
    if (ov) {
        if (strcmp(ov, "hidden") == 0) {
            ln->overflow_hidden = true;
            ln->truncate_overflow = true;
        } else if (strcmp(ov, "auto") == 0 || strcmp(ov, "scroll") == 0) {
            ln->overflow_hidden = true;
            ln->truncate_overflow = true;
        }
    }

    /* whitespace */
    const char* ws = get_style(sn, "white-space");
    if (ws && (strcmp(ws, "pre") == 0 || strcmp(ws, "pre-wrap") == 0 ||
               strcmp(ws, "pre-line") == 0))
        ln->preserve_ws = true;

    /* visibility */
    const char* vis = get_style(sn, "visibility");
    if (vis && strcmp(vis, "hidden") == 0)
        ln->visibility_hidden = true;

    /* outline */
    ln->outline_width = 0;
    const char* olw = get_style(sn, "outline-width");
    if (olw) ln->outline_width = (int)strtol(olw, NULL, 10);
    const char* olc = get_style(sn, "outline-color");
    if (olc) ln->outline_color = parse_color(olc);
    if (ln->outline_width == 0) {
        const char* ol = get_style(sn, "outline");
        if (ol) {
            /* Try to parse "width style color" */
            const char* cp = ol;
            while (*cp) {
                while (*cp == ' '||*cp=='\t') cp++;
                if (!*cp) break;
                if ((*cp >= '0' && *cp <= '9') || *cp == '-') {
                    ln->outline_width = (int)strtol(cp, (char**)&cp, 10);
                } else if (strncmp(cp, "currentColor", 12) == 0) {
                    /* resolved later */
                    cp += 12;
                } else if (strncmp(cp, "transparent", 11) == 0) {
                    cp += 11;
                } else if (strncmp(cp, "solid", 5) == 0 || strncmp(cp, "dashed", 6) == 0) {
                    cp += (*cp == 's') ? 5 : 6;
                } else {
                    /* Assume it's a color */
                    const char* end = cp;
                    while (*end && *end != ' ' && *end != '\t') end++;
                    char tmp[64]; int tlen = (int)(end-cp);
                    if (tlen > 63) tlen = 63;
                    strncpy(tmp, cp, tlen); tmp[tlen] = '\0';
                    ln->outline_color = parse_color(tmp);
                    cp = end;
                }
            }
        }
    }

    /* border */
    const char* bord = get_style(sn, "border");
    if (bord) parse_box_shorthand(bord, &ln->border_top, &ln->border_right,
                                   &ln->border_bottom, &ln->border_left);

    const char* bw = get_style(sn, "border-width");
    if (bw) parse_box_shorthand(bw, &ln->border_top, &ln->border_right,
                                 &ln->border_bottom, &ln->border_left);

    /* border style */
    const char* bs = get_style(sn, "border-style");
    if (bs) {
        if (strcmp(bs, "solid") == 0) ln->border_style = 1;
        else if (strcmp(bs, "dashed") == 0) ln->border_style = 2;
        else if (strcmp(bs, "dotted") == 0) ln->border_style = 3;
        else if (strcmp(bs, "double") == 0) ln->border_style = 4;
        else if (strcmp(bs, "heavy") == 0 || strcmp(bs, "bold") == 0) ln->border_style = 5;
        else if (strcmp(bs, "rounded") == 0) ln->border_style = 6;
        else if (strcmp(bs, "groove") == 0 || strcmp(bs, "ridge") == 0 ||
                 strcmp(bs, "inset") == 0 || strcmp(bs, "outset") == 0) ln->border_style = 1;
    }

    /* border color */
    const char* bc = get_style(sn, "border-color");
    if (bc && strcmp(bc, "currentColor") != 0) ln->border_color = parse_color(bc);

    /* colors */
    const char* col = get_style(sn, "color");
    if (col) ln->color = parse_color(col);

    const char* bg = get_style(sn, "background-color");
    if (bg) {
        if (strcmp(bg, "currentColor") == 0) {
            ln->bg_color = ln->color;
        } else {
            ln->bg_color = parse_color(bg);
        }
    }
    /* also check 'background' shorthand */
    if (!ln->bg_color.valid) {
        bg = get_style(sn, "background");
        if (bg) {
            if (strcmp(bg, "currentColor") == 0) {
                ln->bg_color = ln->color;
            } else {
                ln->bg_color = parse_color(bg);
            }
        }
    }

    /* text styling */
    ln->font_bold = false;
    const char* fw = get_style(sn, "font-weight");
    if (fw && (strcmp(fw, "bold") == 0 || strcmp(fw, "700") == 0 || strcmp(fw, "bolder") == 0))
        ln->font_bold = true;

    ln->font_underline = 0;
    const char* td = get_style(sn, "text-decoration");
    if (td) {
        if (strcmp(td, "underline") == 0) ln->font_underline = 1;
        else if (strcmp(td, "overline") == 0) ln->font_underline = 2;
        else if (strcmp(td, "line-through") == 0) ln->font_underline = 3;
    }

    /* text-transform */
    ln->text_transform = 0;
    const char* tt = get_style(sn, "text-transform");
    if (tt) {
        if (strcmp(tt, "uppercase") == 0) ln->text_transform = 1;
        else if (strcmp(tt, "lowercase") == 0) ln->text_transform = 2;
        else if (strcmp(tt, "capitalize") == 0) ln->text_transform = 3;
    }

    /* line-height */
    ln->line_height = 0;
    const char* lh = get_style(sn, "line-height");
    if (lh) {
        int v = (int)strtol(lh, NULL, 10);
        if (v > 1) ln->line_height = v;
    }

    /* letter-spacing */
    ln->letter_spacing = 0;
    const char* ls = get_style(sn, "letter-spacing");
    if (ls) ln->letter_spacing = (int)strtol(ls, NULL, 10);

    /* word-spacing */
    ln->word_spacing = 0;
    const char* wos = get_style(sn, "word-spacing");
    if (wos) ln->word_spacing = (int)strtol(wos, NULL, 10);

    /* vertical-align */
    ln->vertical_align = 0;
    const char* va = get_style(sn, "vertical-align");
    if (va) {
        if (strcmp(va, "top") == 0) ln->vertical_align = 1;
        else if (strcmp(va, "middle") == 0) ln->vertical_align = 2;
        else if (strcmp(va, "bottom") == 0) ln->vertical_align = 3;
        else if (strcmp(va, "text-top") == 0) ln->vertical_align = 1;
        else if (strcmp(va, "text-bottom") == 0) ln->vertical_align = 3;
    }

    /* max-width / min-width / max-height / min-height */
    ln->min_width = 0; ln->max_width = 0;
    ln->min_height = 0; ln->max_height = 0;
    const char* mwstr = get_style(sn, "min-width");
    if (mwstr) { ParsedDim pd = parse_dimension(mwstr); if (pd.valid && pd.unit==0) ln->min_width = (int)pd.value; }
    mwstr = get_style(sn, "max-width");
    if (mwstr) { ParsedDim pd = parse_dimension(mwstr); if (pd.valid && pd.unit==0) ln->max_width = (int)pd.value; }
    mwstr = get_style(sn, "min-height");
    if (mwstr) { ParsedDim pd = parse_dimension(mwstr); if (pd.valid && pd.unit==0) ln->min_height = (int)pd.value; }
    mwstr = get_style(sn, "max-height");
    if (mwstr) { ParsedDim pd = parse_dimension(mwstr); if (pd.valid && pd.unit==0) ln->max_height = (int)pd.value; }

    /* box-sizing */
    ln->box_sizing = 0;
    const char* bs_str = get_style(sn, "box-sizing");
    if (bs_str && strcmp(bs_str, "border-box") == 0) ln->box_sizing = 1;

    /* text-align */
    ln->text_align = 0; /* left */
    const char* ta = get_style(sn, "text-align");
    if (ta) {
        if (strcmp(ta, "center") == 0) ln->text_align = 1;
        else if (strcmp(ta, "right") == 0) ln->text_align = 2;
    }

    /* flex properties */
    ln->flex_direction = FLEX_ROW;
    const char* fd = get_style(sn, "flex-direction");
    if (fd && strcmp(fd, "column") == 0) ln->flex_direction = FLEX_COLUMN;

    ln->justify_content = JUSTIFY_START;
    const char* jc = get_style(sn, "justify-content");
    if (jc) {
        if (strcmp(jc, "center") == 0) ln->justify_content = JUSTIFY_CENTER;
        else if (strcmp(jc, "end") == 0) ln->justify_content = JUSTIFY_END;
        else if (strcmp(jc, "space-between") == 0) ln->justify_content = JUSTIFY_SPACE_BETWEEN;
        else if (strcmp(jc, "space-around") == 0) ln->justify_content = JUSTIFY_SPACE_AROUND;
        else if (strcmp(jc, "space-evenly") == 0) ln->justify_content = JUSTIFY_SPACE_EVENLY;
    }

    ln->align_items = ALIGN_STRETCH;
    const char* ai = get_style(sn, "align-items");
    if (ai) {
        if (strcmp(ai, "start") == 0 || strcmp(ai, "flex-start") == 0) ln->align_items = ALIGN_START;
        else if (strcmp(ai, "center") == 0) ln->align_items = ALIGN_CENTER;
        else if (strcmp(ai, "end") == 0 || strcmp(ai, "flex-end") == 0) ln->align_items = ALIGN_END;
    }

    /* gap */
    ln->gap = 0;
    const char* gp = get_style(sn, "gap");
    if (gp) ln->gap = (int)strtol(gp, NULL, 10);

    /* flex-wrap */
    ln->flex_wrap = 0;
    const char* fwrap = get_style(sn, "flex-wrap");
    if (fwrap && strcmp(fwrap, "wrap") == 0) ln->flex_wrap = 1;

    /* position: relative */
    ln->position_type = 0;
    ln->position_top = ln->position_right = ln->position_bottom = ln->position_left = 0;
    const char* ppos = get_style(sn, "position");
    if (ppos && strcmp(ppos, "relative") == 0) ln->position_type = 1;
    const char* ptop = get_style(sn, "top"); if (ptop) ln->position_top = (int)strtol(ptop, NULL, 10);
    const char* pleft = get_style(sn, "left"); if (pleft) ln->position_left = (int)strtol(pleft, NULL, 10);
    const char* pright = get_style(sn, "right"); if (pright) ln->position_right = (int)strtol(pright, NULL, 10);
    const char* pbottom = get_style(sn, "bottom"); if (pbottom) ln->position_bottom = (int)strtol(pbottom, NULL, 10);

    /* flex child properties */
    ln->flex_grow = 0;
    const char* fg = get_style(sn, "flex-grow");
    if (fg) ln->flex_grow = (int)strtol(fg, NULL, 10);

    ln->flex_shrink = 1;
    const char* fs = get_style(sn, "flex-shrink");
    if (fs) ln->flex_shrink = (int)strtol(fs, NULL, 10);

    ln->flex_basis = 0;
    const char* fb = get_style(sn, "flex-basis");
    if (fb) {
        ParsedDim dim = parse_dimension(fb);
        if (dim.valid && dim.unit == 0) ln->flex_basis = (int)dim.value;
    }
}

/* ---------- dimension resolution ---------- */

/** Get the total width including padding and border */
static inline int total_width(LayoutNode* n) {
    return n->width + n->padding_left + n->padding_right +
           n->border_left + n->border_right;
}

/** Get the total height including padding and border */
static inline int total_height(LayoutNode* n) {
    return n->height + n->padding_top + n->padding_bottom +
           n->border_top + n->border_bottom;
}

/* ---------- layout algorithms ---------- */

/** Layout block children: stack vertically, full width */
static void layout_block_children(LayoutNode* parent, int content_w) {
    int y_cursor = 0;
    size_t visible_count = 0;

    /* first pass: count visible children and compute widths */
    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;
        visible_count++;

        /* resolve child width */
        const char* w_str = get_style(child->styled, "width");
        ParsedDim wdim = parse_dimension(w_str);
        int box_extra = (child->box_sizing == 1) ? child->padding_left + child->padding_right + child->border_left + child->border_right : 0;
        if (wdim.valid && wdim.unit != 2) {
            int set_w = resolve_dim(wdim, content_w, content_w);
            if (child->box_sizing == 1) {
                child->width = set_w - child->padding_left - child->padding_right - child->border_left - child->border_right;
            } else {
                child->width = set_w;
            }
            /* Clamp to available space */
            int avail = content_w - child->margin_left - child->margin_right - box_extra;
            if (child->width > avail) child->width = avail;
        } else {
            /* default: fill parent */
            child->width = content_w - child->margin_left - child->margin_right -
                           child->padding_left - child->padding_right -
                           child->border_left - child->border_right;
        }
        /* Apply min-width / max-width constraints */
        if (child->min_width > 0 && child->width < child->min_width) child->width = child->min_width;
        if (child->max_width > 0 && child->width > child->max_width) child->width = child->max_width;
        if (child->width < 0) child->width = 0;
    }

    /* second pass: position children and compute their heights */
    int inline_cursor = 0;
    int prev_mb = 0;
    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;

        if (child->display == DISPLAY_INLINE) {
            /* Inline children flow horizontally on the same line */
            child->width = content_w - child->margin_left - child->margin_right -
                           child->padding_left - child->padding_right -
                           child->border_left - child->border_right;
            if (child->width < 0) child->width = 0;
            /* Width from measured text (unless CSS width is set) */
            if (child->text_content) {
                int tw = uc_str_width(child->text_content);
                if (tw > 0) child->width = tw;
            }
            {
                const char* css_w = get_style(child->styled, "width");
                if (css_w) { ParsedDim pd = parse_dimension(css_w);
                    if (pd.valid && pd.unit == 0 && (int)pd.value > 0) child->width = (int)pd.value; }
            }
            /* Apply min-width / max-width to inline children too */
            if (child->min_width > 0 && child->width < child->min_width) child->width = child->min_width;
            if (child->max_width > 0 && child->width > child->max_width) child->width = child->max_width;

            child->x = inline_cursor + child->margin_left + child->border_left + child->padding_left;
            child->y = child->margin_top + child->border_top + child->padding_top;
            /* vertical-align adjustment for inline children */
            if (child->vertical_align == 2) { /* middle */
                child->y = (1 - child->height) / 2 + child->margin_top + child->border_top + child->padding_top;
            } else if (child->vertical_align == 3) { /* bottom */
                child->y = 1 - child->height + child->margin_top + child->border_top + child->padding_top;
            }

            compute_child_layouts(child, child->width);

            const char* h_str = get_style(child->styled, "height");
            if (h_str) {
                ParsedDim hdim = parse_dimension(h_str);
                if (hdim.valid && hdim.unit == 0) child->height = (int)hdim.value;
            }
            if (child->height < 1) child->height = 1;
            /* Apply min-height / max-height */
            if (child->min_height > 0 && child->height < child->min_height) child->height = child->min_height;
            if (child->max_height > 0 && child->height > child->max_height) child->height = child->max_height;

            inline_cursor += child->width + child->padding_left + child->padding_right +
                             child->border_left + child->border_right +
                             child->margin_left + child->margin_right;
            /* position: relative offset for inline children */
            if (child->position_type == 1) {
                child->x += child->position_left - child->position_right;
                child->y += child->position_top - child->position_bottom;
            }
        } else {
            /* Block children: stack vertically, full width */

            /* margin: auto — center block horizontally */
            if (child->margin_left_auto && child->margin_right_auto) {
                int total_box = total_width(child) + child->margin_left + child->margin_right;
                if (total_box < content_w) {
                    child->margin_left = (content_w - total_box) / 2;
                    child->margin_right = content_w - total_box - child->margin_left;
                }
            } else if (child->margin_left_auto) {
                int total_box = total_width(child) + child->margin_right;
                if (total_box < content_w)
                    child->margin_left = content_w - total_box;
            } else if (child->margin_right_auto) {
                int total_box = total_width(child) + child->margin_left;
                if (total_box < content_w)
                    child->margin_right = content_w - total_box;
            }

            child->x = child->margin_left + child->border_left + child->padding_left;
            child->y = y_cursor + child->margin_top + child->border_top + child->padding_top;
            /* position: relative offset */
            if (child->position_type == 1) {
                child->x += child->position_left - child->position_right;
                child->y += child->position_top - child->position_bottom;
            }

            compute_child_layouts(child, child->width);

            const char* h_str = get_style(child->styled, "height");
            if (h_str) {
                ParsedDim hdim = parse_dimension(h_str);
                if (hdim.valid && hdim.unit == 0) child->height = (int)hdim.value;
            }
            if (child->min_height > 0 && child->height < child->min_height) child->height = child->min_height;
            if (child->max_height > 0 && child->height > child->max_height) child->height = child->max_height;

            /* Margin collapse: adjacent vertical margins collapse to max */
            if (child->margin_top > 0 || prev_mb > 0) {
                int collapsed_mt = (prev_mb > child->margin_top) ? prev_mb : child->margin_top;
                y_cursor += collapsed_mt + total_height(child);
            } else {
                y_cursor += total_height(child) + child->margin_bottom;
            }
            prev_mb = child->margin_bottom;
        }
    }

    /* set parent height based on children */
    if (visible_count > 0) {
        int max_bottom = 0;
        for (size_t i = 0; i < parent->num_children; i++) {
            LayoutNode* ch = parent->children[i];
            if (ch->display == DISPLAY_NONE) continue;
            int btm = ch->y + ch->height + ch->padding_bottom +
                      ch->border_bottom + ch->margin_bottom;
            if (btm > max_bottom) max_bottom = btm;
        }
        if (y_cursor < max_bottom) y_cursor = max_bottom;
        parent->height = y_cursor;
    }
}

/** Layout flex children */
static void layout_flex_children(LayoutNode* parent, int content_w, int content_h) {
    bool is_row = (parent->flex_direction == FLEX_ROW);
    int main_size = is_row ? content_w : content_h;
    int cross_size = is_row ? content_h : content_w;

    /* first pass: collect visible children and compute their natural sizes */
    size_t visible = 0;
    int* natural_mains = (int*)calloc(parent->num_children, sizeof(int));
    int total_grow = 0;

    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;

        /* natural main size */
        int natural;
        if (child->flex_basis > 0) {
            natural = child->flex_basis;
        } else {
            const char* w_str = is_row ? get_style(child->styled, "width") : get_style(child->styled, "height");
            ParsedDim dim = parse_dimension(w_str);
            if (dim.valid && dim.unit == 0) {
                natural = (int)dim.value;
            } else {
                /* fit content: estimate from text */
                natural = 0;
                if (child->text_content) {
                    int len = uc_str_width(child->text_content);
                    if (is_row) natural = len; else natural = 1;
                }
            }
        }

        /* add box model */
        if (is_row) {
            natural += child->padding_left + child->padding_right +
                       child->border_left + child->border_right +
                       child->margin_left + child->margin_right;
        } else {
            natural += child->padding_top + child->padding_bottom +
                       child->border_top + child->border_bottom +
                       child->margin_top + child->margin_bottom;
        }

        natural_mains[i] = natural;
        total_grow += child->flex_grow;
        visible++;
    }

    /* calculate gaps */
    int total_gap = parent->gap * (int)(visible > 0 ? visible - 1 : 0);
    int used_main = total_gap;

    for (size_t i = 0; i < parent->num_children; i++) {
        if (parent->children[i]->display == DISPLAY_NONE) continue;
        used_main += natural_mains[i];
    }

    int remaining = main_size - used_main;

    /* flex-shrink: only shrink when container has definite size (not auto) */
    /* NOTE: if flex-wrap is enabled, skip shrink — items should overflow to wrap */
    if (remaining < 0 && main_size > 0 && visible > 0 && !parent->flex_wrap) {
        int total_shrink_weight = 0;
        for (size_t i = 0; i < parent->num_children; i++) {
            LayoutNode* child = parent->children[i];
            if (child->display == DISPLAY_NONE) continue;
            /* Use natural_main as weight (larger items shrink more) */
            int weight = child->flex_shrink > 0 ? child->flex_shrink * natural_mains[i] : natural_mains[i];
            if (weight < 0) weight = 1;
            total_shrink_weight += weight;
        }
        if (total_shrink_weight > 0) {
            int shrink_total = -remaining;
            int remaining_shrink = shrink_total;
            for (size_t i = 0; i < parent->num_children; i++) {
                LayoutNode* child = parent->children[i];
                if (child->display == DISPLAY_NONE) continue;
                int weight = child->flex_shrink > 0 ? child->flex_shrink * natural_mains[i] : natural_mains[i];
                if (weight < 0) weight = 1;
                int shrink = (shrink_total * weight) / total_shrink_weight;
                if (shrink > natural_mains[i]) shrink = natural_mains[i];
                if (shrink < 0) shrink = 0;
                if (i == parent->num_children - 1)
                    shrink = remaining_shrink; /* last one takes remainder */
                remaining_shrink -= shrink;
                natural_mains[i] -= shrink;
            }
            remaining = 0;
        }
    }

    /* second pass: distribute remaining space */
    int main_cursor = 0;

    /* justify-content spacing */
    int justify_offset = 0;
    int justify_gap = 0;
    if (visible >= 1 && remaining > 0) {
        switch (parent->justify_content) {
            case JUSTIFY_CENTER:
                justify_offset = remaining / 2;
                remaining = 0;
                break;
            case JUSTIFY_END:
                justify_offset = remaining;
                remaining = 0;
                break;
            case JUSTIFY_SPACE_BETWEEN:
                if (visible > 1) {
                    justify_gap = remaining / (int)(visible - 1);
                    remaining = 0;
                }
                break;
            case JUSTIFY_SPACE_AROUND: {
                int space = remaining / (int)visible;
                justify_offset = space / 2;
                justify_gap = space;
                remaining = 0;
                break;
            }
            case JUSTIFY_SPACE_EVENLY: {
                int space = remaining / (int)(visible + 1);
                justify_offset = space;
                justify_gap = space;
                remaining = 0;
                break;
            }
            default: break;
        }
    }

    main_cursor = justify_offset;

    /* flex-wrap tracking */
    int cross_cursor = 0;
    int line_max_cross = 0;
    int* line_starts = (int*)calloc(visible + 1, sizeof(int));
    int line_count = 0;
    line_starts[line_count++] = 0;

    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;

        int child_main = natural_mains[i];

        /* flex-wrap: if item doesn't fit on current line, start new line */
        if (parent->flex_wrap && main_cursor + child_main > main_size && main_cursor > 0) {
            cross_cursor += line_max_cross + parent->gap;
            main_cursor = justify_offset;
            line_max_cross = 0;
            line_starts[line_count++] = i;
        }

        /* flex-grow: distribute remaining */
        if (total_grow > 0 && remaining > 0) {
            int extra = (remaining * child->flex_grow) / total_grow;
            child_main += extra;
        }

        /* Set position and size */
        if (is_row) {
            child->x = main_cursor + child->margin_left + child->border_left + child->padding_left;
            child->y = cross_cursor + child->margin_top + child->border_top + child->padding_top;
            child->width = child_main - child->margin_left - child->margin_right -
                           child->padding_left - child->padding_right -
                           child->border_left - child->border_right;

            /* cross axis: align-items */
            int available_cross = cross_size - cross_cursor - child->margin_top - child->margin_bottom -
                                  child->padding_top - child->padding_bottom -
                                  child->border_top - child->border_bottom;

            const char* h_str = get_style(child->styled, "height");
            ParsedDim hdim = parse_dimension(h_str);
            if (hdim.valid && hdim.unit == 0) {
                child->height = (int)hdim.value;
            } else if (parent->align_items == ALIGN_STRETCH && available_cross > 0) {
                child->height = available_cross;
            } else {
                /* compute from children or use default */
                compute_child_layouts(child, child->width);
                if (child->height < 1) child->height = 1;
            }

            /* align-items positioning */
            if (parent->align_items == ALIGN_CENTER && cross_size > 0) {
                child->y = (cross_size - total_height(child) - child->margin_top - child->margin_bottom) / 2
                           + child->margin_top + child->border_top + child->padding_top;
            } else if (parent->align_items == ALIGN_END && cross_size > 0) {
                child->y = cross_size - total_height(child) - child->margin_bottom
                           + child->margin_top + child->border_top + child->padding_top;
            }

            main_cursor += child_main + parent->gap;
            if (justify_gap > 0) main_cursor += justify_gap;
        } else {
            /* column mode */
            child->x = child->margin_left + child->border_left + child->padding_left;
            child->y = main_cursor + child->margin_top + child->border_top + child->padding_top;
            child->height = child_main - child->margin_top - child->margin_bottom -
                            child->padding_top - child->padding_bottom -
                            child->border_top - child->border_bottom;

            /* cross axis: width */
            int available_cross = cross_size - child->margin_left - child->margin_right -
                                  child->padding_left - child->padding_right -
                                  child->border_left - child->border_right;

            if (parent->align_items == ALIGN_STRETCH && available_cross > 0) {
                child->width = available_cross;
            } else {
                const char* w_str = get_style(child->styled, "width");
                ParsedDim wdim = parse_dimension(w_str);
                if (wdim.valid && wdim.unit == 0) {
                    child->width = (int)wdim.value;
                }
                if (child->width < 1) child->width = 1;
            }

            /* align-items positioning (for column, controls horizontal) */
            switch (parent->align_items) {
                case ALIGN_CENTER:
                    child->x = (cross_size - total_width(child) - child->margin_left - child->margin_right) / 2
                               + child->margin_left + child->border_left + child->padding_left;
                    break;
                case ALIGN_END:
                    child->x = cross_size - total_width(child) - child->margin_right
                               + child->margin_left + child->border_left + child->padding_left;
                    break;
                default: break;
            }

            main_cursor += child_main + parent->gap;
            if (justify_gap > 0) main_cursor += justify_gap;
        }

        /* compute child's children after dimensions are set */
        compute_child_layouts(child, child->width);

        /* Track max cross size for flex-wrap */
        {
            int ch = child->height + child->padding_top + child->padding_bottom +
                     child->border_top + child->border_bottom +
                     child->margin_top + child->margin_bottom;
            if (ch > line_max_cross) line_max_cross = ch;
        }

        /* position: relative offset */
        if (child->position_type == 1) {
            child->x += child->position_left - child->position_right;
            child->y += child->position_top - child->position_bottom;
        }
    }

    free(natural_mains);
    free(line_starts);

    /* set parent height (for row) or width+height (for column) */
    if (visible > 0) {
        int max_bottom = 0;
        int max_right = 0;
        for (size_t i = 0; i < parent->num_children; i++) {
            LayoutNode* child = parent->children[i];
            if (child->display == DISPLAY_NONE) continue;
            int bottom = child->y + child->height + child->padding_bottom +
                         child->border_bottom + child->margin_bottom;
            int right = child->x + child->width + child->padding_right +
                        child->border_right + child->margin_right;
            if (bottom > max_bottom) max_bottom = bottom;
            if (right > max_right) max_right = right;
        }
        if (is_row) {
            parent->height = max_bottom;
        } else {
            parent->width = max_right;
            parent->height = max_bottom;
        }
    }
}

/** Compute child layouts based on parent's display type */
/** Layout table: rows stack vertically, cells share width equally */
static void layout_table_children(LayoutNode* parent, int content_w) {
    if (content_w < 1) content_w = 1;

    /* Respect CSS width/height on <table> element if set */
    bool has_css_width = false;
    int table_w = content_w;
    const char* tw_str = parent->styled ? get_style(parent->styled, "width") : NULL;
    if (tw_str && strcmp(tw_str, "auto") != 0) {
        ParsedDim tw_dim = parse_dimension(tw_str);
        if (tw_dim.valid) {
            int tw = resolve_dim(tw_dim, content_w, content_w);
            if (tw > 0) { table_w = tw; has_css_width = true; }
        }
    }
    int table_h = 0; /* 0 means auto */
    const char* th_str = parent->styled ? get_style(parent->styled, "height") : NULL;
    if (th_str) {
        ParsedDim th_dim = parse_dimension(th_str);
        if (th_dim.valid) {
            int th = resolve_dim(th_dim, content_w, content_w);
            if (th > 0) table_h = th;
        }
    }

    /* First pass: collect all rows by walking through table children.
       Gumbo wraps <tr> in <tbody>, so we need to handle both.
       A "row" is any child with display=TABLE_ROW.
       A "row group" (like tbody) has rows as children. */
    LayoutNode** rows = (LayoutNode**)malloc(16 * sizeof(LayoutNode*));
    size_t num_rows = 0, cap_rows = 16;

    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;
        if (child->display == DISPLAY_TABLE_ROW) {
            /* Direct row child */
            if (num_rows >= cap_rows) { cap_rows *= 2; rows = realloc(rows, cap_rows * sizeof(LayoutNode*)); }
            rows[num_rows++] = child;
        } else if (child->num_children > 0) {
            /* Could be a row group (tbody/thead/tfoot). Collect its row children. */
            for (size_t j = 0; j < child->num_children; j++) {
                LayoutNode* rc = child->children[j];
                if (rc->display != DISPLAY_NONE && rc->display == DISPLAY_TABLE_ROW) {
                    if (num_rows >= cap_rows) { cap_rows *= 2; rows = realloc(rows, cap_rows * sizeof(LayoutNode*)); }
                    rows[num_rows++] = rc;
                }
            }
        }
    }

    if (num_rows == 0) { free(rows); return; }

    /* Count max columns across all rows (considering colspan) */
    size_t max_cols = 0;
    for (size_t i = 0; i < num_rows; i++) {
        size_t n = 0;
        for (size_t j = 0; j < rows[i]->num_children; j++) {
            if (rows[i]->children[j]->display == DISPLAY_NONE) continue;
            int colspan = 1;
            if (rows[i]->children[j]->styled && rows[i]->children[j]->styled->node &&
                rows[i]->children[j]->styled->node->type == GUMBO_NODE_ELEMENT) {
                GumboAttribute* ca = gumbo_get_attribute(
                    &rows[i]->children[j]->styled->node->v.element.attributes, "colspan");
                if (ca && ca->value) { int cv = atoi(ca->value); if (cv > 1) colspan = cv; }
            }
            n += colspan;
        }
        if (n > max_cols) max_cols = n;
    }
    if (max_cols == 0) { free(rows); return; }

    /* Measure natural cell width per column */
    int* col_w = (int*)calloc(max_cols, sizeof(int));
    for (size_t i = 0; i < num_rows; i++) {
        LayoutNode* row = rows[i];
        size_t c = 0;
        for (size_t j = 0; j < row->num_children; j++) {
            LayoutNode* cell = row->children[j];
            if (cell->display == DISPLAY_NONE) continue;
            int colspan = 1;
            if (cell->styled && cell->styled->node && cell->styled->node->type == GUMBO_NODE_ELEMENT) {
                GumboAttribute* ca = gumbo_get_attribute(
                    &cell->styled->node->v.element.attributes, "colspan");
                if (ca && ca->value) { int cv = atoi(ca->value); if (cv > 1) colspan = cv; }
            }
            int nat = cell->padding_left + cell->padding_right +
                      cell->border_left + cell->border_right;
            if (cell->text_content) nat += uc_str_width(cell->text_content);
            int per_col = nat / colspan;
            for (int k = 0; k < colspan && c + k < max_cols; k++) {
                if (per_col > (int)col_w[c + k]) col_w[c + k] = per_col;
            }
            c += colspan;
        }
    }

    int total_nat = 0;
    for (size_t c = 0; c < max_cols; c++) total_nat += col_w[c];
    if (total_nat < 1) total_nat = 1;

    /* Use CSS width if set, otherwise natural width (capped at parent) */
    bool has_width = (tw_str && strcmp(tw_str, "auto") != 0);
    int avail = has_width ? table_w : (total_nat < content_w ? total_nat : content_w);

    int y_cursor = 0;
    for (size_t i = 0; i < num_rows; i++) {
        LayoutNode* row = rows[i];

        row->x = 0; row->y = y_cursor; row->width = avail;

        int cx = 0, max_h = 0;
        size_t c = 0;
        for (size_t j = 0; j < row->num_children; j++) {
            LayoutNode* cell = row->children[j];
            if (cell->display == DISPLAY_NONE) continue;

            int colspan = 1;
            if (cell->styled && cell->styled->node && cell->styled->node->type == GUMBO_NODE_ELEMENT) {
                GumboAttribute* ca = gumbo_get_attribute(
                    &cell->styled->node->v.element.attributes, "colspan");
                if (ca && ca->value) { int cv = atoi(ca->value); if (cv > 1) colspan = cv; }
            }

            /* Sum column widths for colspan */
            int cw = 0;
            for (int k = 0; k < colspan && c + k < max_cols; k++) {
                cw += col_w[c + k] * avail / total_nat;
            }
            if (cw < 1) cw = 1;

            cell->x = cx + cell->border_left + cell->padding_left;
            /* Cell content starts 1 cell below row top (for the grid line) */
            cell->y = cell->padding_top + 1;
            cell->width = cw - cell->padding_left - cell->padding_right -
                          cell->border_left - cell->border_right;
            if (cell->width < 0) cell->width = 0;

            compute_child_layouts(cell, cell->width);

            /* Row height = content only (borders handled by table grid);
               +1 for the grid line at row top */
            int ch = cell->height + cell->padding_top + cell->padding_bottom;
            if (ch > max_h) max_h = ch;
            cx += cw;
            c += colspan;
        }

        for (size_t j = 0; j < row->num_children; j++) {
            LayoutNode* cell = row->children[j];
            if (cell->display == DISPLAY_NONE) continue;
            int target = max_h - cell->padding_top - cell->padding_bottom;
            if (target > cell->height) cell->height = target;
            /* Zero out cell vertical borders — table grid handles them */
            cell->border_top = cell->border_bottom = 0;
        }
        row->height = max_h + 1;  /* +1 for the grid separator line */
        y_cursor += max_h + 1;
    }

    parent->height = y_cursor;

    /* If CSS height is set, adjust row heights to fill */
    if (table_h > 0 && y_cursor > 0 && y_cursor < table_h && num_rows > 0) {
        int extra = table_h - y_cursor;
        int extra_per = extra / (int)num_rows;
        int extra_rem = extra % (int)num_rows;
        int y_fix = 0;
        for (size_t i = 0; i < num_rows; i++) {
            int add = extra_per + (i < (size_t)extra_rem ? 1 : 0);
            LayoutNode* row = rows[i];
            row->height += add;
            row->y = y_fix;
            /* Extend cell heights in this row */
            for (size_t j = 0; j < row->num_children; j++) {
                LayoutNode* cell = row->children[j];
                if (cell->display == DISPLAY_NONE) continue;
                cell->height += add;
            }
            y_fix += row->height;
        }
        parent->height = table_h;
    }

    parent->width = avail;
    free(col_w);
    free(rows);
}

/** Compute child layouts based on parent's display type */
static void compute_child_layouts(LayoutNode* parent, int content_w) {
    if (!parent || parent->num_children == 0) return;

    int content_h = parent->height;

    switch (parent->display) {
        case DISPLAY_BLOCK:
        case DISPLAY_INLINE:
            layout_block_children(parent, content_w);
            break;
        case DISPLAY_TABLE:
            layout_table_children(parent, content_w);
            break;
        case DISPLAY_TABLE_ROW:
            /* Handled entirely by parent layout_table_children */
            break;
        case DISPLAY_FLEX:
            layout_flex_children(parent, content_w, content_h);
            break;
        case DISPLAY_GRID:
            /* Fall back to block for now */
            layout_block_children(parent, content_w);
            break;
        default:
            break;
    }
}

/* ---------- tree builder ---------- */

static LayoutNode* build_layout_tree_recursive(StyledNode* snode, LayoutNode* parent,
                                               int viewport_w, int viewport_h) {
    if (!snode || !snode->node) return NULL;

    LayoutNode* ln = (LayoutNode*)calloc(1, sizeof(LayoutNode));
    ln->styled = snode;
    ln->parent = parent;

    /* apply styles */
    apply_styles(ln);

    if (ln->display == DISPLAY_NONE) {
        return ln; /* return but won't be rendered */
    }

    /* if we don't have a specific display from CSS, use default from tag */
    if (!get_style(snode, "display") && snode->node->type == GUMBO_NODE_ELEMENT) {
        ln->display = get_default_display(gumbo_normalized_tagname(snode->node->v.element.tag));
    }

    /* Set default text color to white BEFORE building children (inline fragments inherit it) */
    if (!ln->color.valid) {
        ln->color.r = 255; ln->color.g = 255; ln->color.b = 255;
        ln->color.valid = true;
    }

    /* extract text content (skip for table/tr — they render via children) */
    const char* tag_name = NULL;
    if (snode->node->type == GUMBO_NODE_ELEMENT)
        tag_name = gumbo_normalized_tagname(snode->node->v.element.tag);
    
    if (tag_name && (strcmp(tag_name, "table") == 0 || strcmp(tag_name, "tr") == 0)) {
        ln->text_content = NULL;
    } else if (tag_name && strcmp(tag_name, "img") == 0) {
        /* Try to read alt attribute */
        if (snode->node->type == GUMBO_NODE_ELEMENT) {
            GumboAttribute* alt_attr = gumbo_get_attribute(
                &snode->node->v.element.attributes, "alt");
            if (alt_attr && alt_attr->value && alt_attr->value[0]) {
                char buf[128];
                snprintf(buf, sizeof(buf), "[%s]", alt_attr->value);
                ln->text_content = strdup(buf);
            } else {
                ln->text_content = strdup("[img]");
            }
        } else {
            ln->text_content = strdup("[img]");
        }
    } else if (tag_name && snode->num_children > 0) {
        /* Node has element children — don't extract text, let children render it */
        ln->text_content = NULL;
    } else {
        ln->text_content = extract_text(snode->node);
    }

    /* Apply text-transform to text_content (do this once, not at render time) */
    if (ln->text_content && snode->node->type == GUMBO_NODE_ELEMENT) {
        const char* tt = get_style(snode, "text-transform");
        if (tt) {
            char* buf = strdup(ln->text_content);
            if (buf) {
                char* q = buf;
                bool cap_next = true; /* for capitalize */
                if (strcmp(tt, "uppercase") == 0) {
                    while (*q) { if (*q >= 'a' && *q <= 'z') *q = *q - 32; q++; }
                } else if (strcmp(tt, "lowercase") == 0) {
                    while (*q) { if (*q >= 'A' && *q <= 'Z') *q = *q + 32; q++; }
                } else if (strcmp(tt, "capitalize") == 0) {
                    while (*q) {
                        if (cap_next && *q >= 'a' && *q <= 'z') { *q = *q - 32; cap_next = false; }
                        else if (*q >= 'A' && *q <= 'Z') { if (!cap_next) *q = *q + 32; cap_next = false; }
                        else if (*q == ' ' || *q == '\t' || *q == '\n' || *q == '\r' || *q == '-' || *q == '.') cap_next = true;
                        else cap_next = false;
                        q++;
                    }
                }
                free(ln->text_content);
                ln->text_content = buf;
            }
        }
    }

    /* ::before / ::after pseudo-element content */
    if (snode->node->type == GUMBO_NODE_ELEMENT) {
        const char* content_val = get_style(snode, "content");
        if (content_val && content_val[0] && content_val[0] != 'n' &&
            strcmp(content_val, "normal") != 0) {
            /* Strip quotes if present */
            char content_buf[128];
            const char* cp = content_val;
            while (*cp == ' ' || *cp == '\t') cp++;
            if (*cp == '"' || *cp == '\'') {
                size_t clen = strlen(cp);
                if (clen > 2) {
                    int blen = 0;
                    cp++; /* skip opening quote */
                    while (*cp && *cp != '"' && *cp != '\'' && blen < 126) {
                        content_buf[blen++] = *cp;
                        cp++;
                    }
                    content_buf[blen] = '\0';
                    /* Prepend or append to text_content with a space */
                    char* old = ln->text_content;
                    char* combined = (char*)malloc(strlen(content_buf) + (old ? strlen(old) : 0) + 3);
                    combined[0] = '\0';
                    if (old) {
                        strcat(combined, old);
                        strcat(combined, " ");
                    }
                    strcat(combined, content_buf);
                    if (old) free(old);
                    ln->text_content = combined;
                }
            }
        }
    }

    /* build children — interleave styled element children with inline text fragments
       in the original DOM order from Gumbo children */
    if (snode->num_children > 0) {
        GumboVector* gchildren = (snode->node && snode->node->type == GUMBO_NODE_ELEMENT)
            ? &snode->node->v.element.children : NULL;

        /* Count Gumbo non-whitespace text children for allocation */
        size_t gumbo_text_count = 0;
        if (gchildren) {
            for (unsigned int gi = 0; gi < gchildren->length; gi++) {
                GumboNode* gc = (GumboNode*)gchildren->data[gi];
                if (gc->type == GUMBO_NODE_TEXT || gc->type == GUMBO_NODE_WHITESPACE) {
                    bool only_ws = true;
                    for (const char* cp = gc->v.text.text; *cp; cp++)
                        if (*cp != ' ' && *cp != '\t' && *cp != '\n' && *cp != '\r') { only_ws = false; break; }
                    if (!only_ws) gumbo_text_count++;
                }
            }
        }

        size_t total = snode->num_children + gumbo_text_count;
        ln->children = (LayoutNode**)calloc(total, sizeof(LayoutNode*));
        size_t idx = 0, elem_i = 0;

        /* <details> collapse: when closed, only allow summary child and skip others */
        bool details_closed = false;
        if (snode->node->type == GUMBO_NODE_ELEMENT &&
            snode->node->v.element.tag == GUMBO_TAG_DETAILS) {
            details_closed = (ln->details_open == false);
        }

        /* Walk Gumbo children in DOM order; interleave text fragments and element children */
        if (gchildren) {
            for (unsigned int gi = 0; gi < gchildren->length; gi++) {
                GumboNode* gc = (GumboNode*)gchildren->data[gi];
                if (gc->type == GUMBO_NODE_TEXT || gc->type == GUMBO_NODE_WHITESPACE) {
                    /* Text/whitespace node → create inline fragment */
                    if (!gc->v.text.text || !*gc->v.text.text) continue;
                    bool only_ws = true;
                    for (const char* cp = gc->v.text.text; *cp; cp++)
                        if (*cp != ' ' && *cp != '\t' && *cp != '\n' && *cp != '\r') { only_ws = false; break; }
                    if (only_ws) continue;
                    if (details_closed) continue; /* skip text when collapsed */
                    LayoutNode* tn = (LayoutNode*)calloc(1, sizeof(LayoutNode));
                    /* Strip leading/trailing newlines and trim spaces */
                    const char* src = gc->v.text.text;
                    while (*src == '\n' || *src == '\r' || *src == ' ' || *src == '\t') src++;
                    if (!*src) { free(tn); continue; }
                    const char* end = src + strlen(src);
                    while (end > src && (*(end-1) == '\n' || *(end-1) == '\r' || *(end-1) == ' ' || *(end-1) == '\t')) end--;
                    if (end <= src) { free(tn); continue; }
                    tn->text_content = (char*)malloc((size_t)(end - src) + 1);
                    strncpy(tn->text_content, src, (size_t)(end - src));
                    tn->text_content[end - src] = '\0';
                    tn->display = DISPLAY_INLINE;
                    tn->height = 1;
                    tn->color = ln->color;
                    tn->color.valid = ln->color.valid;
                    tn->width = uc_str_width(tn->text_content);
                    ln->children[idx++] = tn;
                } else if (gc->type == GUMBO_NODE_ELEMENT && elem_i < snode->num_children) {
                    /* Element child → use styled tree child */
                    LayoutNode* child = build_layout_tree_recursive(
                        snode->children[elem_i], ln, viewport_w, viewport_h);
                    elem_i++;
                    if (!child) continue;
                    /* <details> collapse: when closed, only keep the first <summary> child */
                    if (details_closed) {
                        if (child->is_summary) {
                            ln->children[idx++] = child;
                            /* Keep collapsed; subsequent children after summary are hidden */
                        } else {
                            free_layout_tree(child);
                            continue;
                        }
                    } else {
                        ln->children[idx++] = child;
                    }
                }
            }
        } else {
            /* No Gumbo children (document root), use styled children directly */
            for (size_t i = 0; i < snode->num_children; i++) {
                LayoutNode* child = build_layout_tree_recursive(
                    snode->children[i], ln, viewport_w, viewport_h);
                if (child) ln->children[idx++] = child;
            }
        }
        ln->num_children = idx;
    }

    /* Compute natural text height if no explicit height */
    if (ln->text_content) {
        int mw = viewport_w - ln->padding_left - ln->padding_right -
                 ln->border_left - ln->border_right;
        if (mw < 1) mw = 1;

        if (ln->preserve_ws) {
            /* <pre>: line count = newlines + 1, no word-wrap */
            int lines = 1;
            for (const char* p = ln->text_content; *p; p++) {
                if (*p == '\n') lines++;
            }
            ln->height = lines;
        } else {
            /* estimate word-wrapped height using display widths */
            int lines = 0, lpos = 0;
            const char* p = ln->text_content;
            while (*p) {
                while (*p == ' ' || *p == '\t') p++;
                if (!*p) break;
                if (*p == '\n') { lines++; lpos = 0; p++; continue; }
                const char* we = p;
                int ww = 0;
                while (*we && *we != ' ' && *we != '\t' && *we != '\n') {
                    uint32_t cp = uc_dec(&we);
                    if (cp == 0) break;
                    ww += uc_wid((int)cp);
                }
                if (lpos + ww > mw && lpos > 0) { lines++; lpos = 0; }
                lpos += ww + 1;
                if (lpos >= mw) { lines++; lpos = 0; }
                p = we;
                if (*p == ' ' || *p == '\t') p++;
                while (*p == '\n') { lines++; lpos = 0; p++; }
            }
            if (lines == 0) lines = 1;
            ln->height = lines;
        }
    }

    /* Check whitespace preservation (<pre> tag) */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_PRE) {
        ln->preserve_ws = true;
    } else if (parent && parent->preserve_ws) {
        ln->preserve_ws = true;
    }

    /* <hr>: set height to 1 for the horizontal line */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_HR && ln->height < 1) {
        ln->height = 1;
    }

    /* List markers for <ul>/<ol>/<li> */
    if (snode->node->type == GUMBO_NODE_ELEMENT) {
        if (snode->node->v.element.tag == GUMBO_TAG_LI) {
            if (parent && parent->styled && parent->styled->node &&
                parent->styled->node->type == GUMBO_NODE_ELEMENT) {
                GumboTag pt = parent->styled->node->v.element.tag;
                if (pt == GUMBO_TAG_UL) ln->list_marker = 1;
                else if (pt == GUMBO_TAG_OL) ln->list_marker = 2;
            }
        }
    }

    /* <a>: default blue + underline */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_A) {
        ln->font_underline = true;
        if (!ln->color.valid ||
            (ln->color.r == 255 && ln->color.g == 255 && ln->color.b == 255)) {
            ln->color.r = 83; ln->color.g = 168; ln->color.b = 182;
            ln->color.valid = true;
        }
    }

    /* <input>: render as value or placeholder */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_INPUT) {
        /* Detect type=password — mask with dots */
        const char* type_val = NULL;
        GumboAttribute* type_attr = gumbo_get_attribute(&snode->node->v.element.attributes, "type");
        if (type_attr && type_attr->value) type_val = type_attr->value;
        bool is_password = (type_val && strcmp(type_val, "password") == 0);

        const char* val = NULL;
        GumboAttribute* attr = gumbo_get_attribute(&snode->node->v.element.attributes, "value");
        if (attr && attr->value) val = attr->value;
        char buf[128];
        if (val && *val) {
            if (is_password) {
                int vlen = (int)strlen(val);
                for (int k = 0; k < vlen && k < 126; k++) buf[k] = 0x2022; /* • */
                buf[vlen > 126 ? 126 : vlen] = '\0';
            } else {
                snprintf(buf, sizeof(buf), "%s", val);
            }
        } else {
            /* Try placeholder */
            GumboAttribute* ph = gumbo_get_attribute(&snode->node->v.element.attributes, "placeholder");
            if (ph && ph->value && ph->value[0]) {
                snprintf(buf, sizeof(buf), "%s", ph->value);
                ln->color.r = 120; ln->color.g = 120; ln->color.b = 120;
            } else {
                strcpy(buf, " ");
                ln->color.r = 180; ln->color.g = 180; ln->color.b = 180;
            }
        }
        if (ln->text_content) free(ln->text_content);
        ln->text_content = strdup(buf);
        ln->color.valid = true;
        ln->truncate_overflow = true;
        if (ln->height < 1) ln->height = 1;
    }

    /* <details>: check "open" attribute first, then override with saved toggle state */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_DETAILS) {
        GumboAttribute* open_attr = gumbo_get_attribute(
            &snode->node->v.element.attributes, "open");
        ln->details_open = (open_attr != NULL);
        /* Override with saved global toggle state if user has toggled it */
        GumboAttribute* id_attr = gumbo_get_attribute(
            &snode->node->v.element.attributes, "id");
        if (id_attr && id_attr->value) {
            for (int i = 0; i < g_det_count; i++) {
                if (strcmp(g_det_state[i].id, id_attr->value) == 0) {
                    ln->details_open = g_det_state[i].open;
                    break;
                }
            }
        }
    }

    /* <summary>: mark as summary */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_SUMMARY) {
        ln->is_summary = true;
        /* Summary is focusable (like a button) */
        if (!ln->bg_color.valid) {
            ln->bg_color.r = 50; ln->bg_color.g = 50; ln->bg_color.b = 70;
            ln->bg_color.valid = true;
        }
        ln->font_bold = true;
    }

    /* <textarea>: editable text area — handle BEFORE text extraction override */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_TEXTAREA) {
        ln->preserve_ws = true;
        if (ln->height < 2) ln->height = 3;
    }

    /* <select>: show placeholder */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_SELECT) {
        /* Show first option or placeholder */
        const char* val = NULL;
        GumboAttribute* attr = gumbo_get_attribute(&snode->node->v.element.attributes, "value");
        if (attr && attr->value) val = attr->value;
        if (val && val[0]) {
            if (ln->text_content) free(ln->text_content);
            ln->text_content = strdup(val);
        } else {
            bool found = false;
            if (snode->node->type == GUMBO_NODE_ELEMENT) {
                GumboVector* ch = &snode->node->v.element.children;
                for (unsigned int ci = 0; ci < ch->length; ci++) {
                    GumboNode* gc = (GumboNode*)ch->data[ci];
                    if (gc->type == GUMBO_NODE_ELEMENT &&
                        gc->v.element.tag == GUMBO_TAG_OPTION) {
                        /* Use value attribute first, or text content */
                        GumboAttribute* oa = gumbo_get_attribute(&gc->v.element.attributes, "value");
                        if (oa && oa->value) {
                            if (ln->text_content) free(ln->text_content);
                            ln->text_content = strdup(oa->value);
                            found = true;
                            break;
                        }
                        /* Try option's text content */
                        char* opt_text = extract_text(gc);
                        if (opt_text && opt_text[0]) {
                            if (ln->text_content) free(ln->text_content);
                            ln->text_content = opt_text;
                            found = true;
                            break;
                        }
                        free(opt_text);
                    }
                }
            }
            if (!found) {
                if (!ln->text_content) ln->text_content = strdup("[Select]");
            }
        }
        ln->color.valid = true;
        ln->truncate_overflow = true;
        if (ln->height < 1) ln->height = 1;
    }

    /* <button>: background + bold text */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_BUTTON) {
        if (!ln->bg_color.valid) {
            ln->bg_color.r = 60; ln->bg_color.g = 60; ln->bg_color.b = 80;
            ln->bg_color.valid = true;
        }
        ln->font_bold = true;
    }

    /* background defaults to transparent (no fill) - CSS behavior */

    return ln;
}

/* ---------- list numbering (post-process) ---------- */

static void number_list_items(LayoutNode* node) {
    if (!node) return;
    /* Number ordered list children */
    if (node->num_children > 0) {
        /* Check if this node is an <ol> — then number its li children */
        bool is_ol = false;
        for (size_t i = 0; i < node->num_children && !is_ol; i++) {
            if (node->children[i]->list_marker == 2) is_ol = true;
        }
        if (is_ol) {
            int idx = 1;
            for (size_t i = 0; i < node->num_children; i++) {
                if (node->children[i]->list_marker == 2) {
                    node->children[i]->list_index = idx++;
                }
            }
        }
        /* Recurse */
        for (size_t i = 0; i < node->num_children; i++) {
            number_list_items(node->children[i]);
        }
    }
}

/* ---------- public API ---------- */

LayoutNode* build_layout_tree(StyledNode* styled_root, int viewport_w, int viewport_h) {
    if (!styled_root) return NULL;

    /* Update media query viewport width */
    g_media_viewport_w = viewport_w;

    LayoutNode* root = build_layout_tree_recursive(styled_root, NULL, viewport_w, viewport_h);
    if (!root) return NULL;

    /* Post-process: number list items */
    number_list_items(root);

    /* Root fills viewport */
    root->width = viewport_w;
    root->height = viewport_h;
    root->x = 0; root->y = 0;

    /* Recursively compute layouts for all children */
    compute_child_layouts(root, root->width);

    /* Root height is at least viewport height (scrollable overflow) */
    if (root->height < viewport_h) root->height = viewport_h;

    /* Root background: default to black so viewport is always filled */
    if (!root->bg_color.valid) {
        root->bg_color.r = 0; root->bg_color.g = 0; root->bg_color.b = 0;
        root->bg_color.valid = true;
    }

    return root;
}

void free_layout_tree(LayoutNode* tree) {
    if (!tree) return;
    for (size_t i = 0; i < tree->num_children; i++) {
        free_layout_tree(tree->children[i]);
    }
    free(tree->children);
    free(tree->text_content);
    free(tree);
}

void debug_print_layout(LayoutNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  ");

    const char* tag = "?";
    const char* disp = "?";
    if (node->styled && node->styled->node &&
        node->styled->node->type == GUMBO_NODE_ELEMENT) {
        tag = gumbo_normalized_tagname(node->styled->node->v.element.tag);
    }
    switch (node->display) {
        case DISPLAY_NONE: disp = "none"; break;
        case DISPLAY_BLOCK: disp = "block"; break;
        case DISPLAY_INLINE: disp = "inline"; break;
        case DISPLAY_FLEX: disp = "flex"; break;
        case DISPLAY_GRID: disp = "grid"; break;
    }

    printf("<%s display=%s xy=(%d,%d) wh=(%d,%d) pad=(%d,%d,%d,%d) mar=(%d,%d,%d,%d)",
           tag, disp, node->x, node->y, node->width, node->height,
           node->padding_top, node->padding_right, node->padding_bottom, node->padding_left,
           node->margin_top, node->margin_right, node->margin_bottom, node->margin_left);

    if (node->text_content) {
        /* print first 30 chars of text */
        printf(" text=\"");
        for (int i = 0; i < 30 && node->text_content[i]; i++) {
            char c = node->text_content[i];
            if (c == '\n') printf("\\n");
            else putchar(c);
        }
        if (strlen(node->text_content) > 30) printf("...");
        printf("\"");
    }

    printf("\n");

    for (size_t i = 0; i < node->num_children; i++) {
        debug_print_layout(node->children[i], indent + 1);
    }
}

#endif /* LAYOUT_IMPLEMENTATION */

#endif /* LAYOUT_H */

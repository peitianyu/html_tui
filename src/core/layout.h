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
    int border_top, border_right, border_bottom, border_left;

    /* text content extracted from DOM subtree */
    char* text_content;

    /* computed colors */
    ResolvedColor color;
    ResolvedColor bg_color;
    ResolvedColor border_color;

    /* text styling */
    bool font_bold;
    bool font_underline;

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

    /* border style: 0=none, 1=solid, 2=dashed */
    int border_style;

    /* whitespace preservation (like <pre>) */
    bool preserve_ws;

    /* list marker type: 0=none, 1=bullet (•), 2=ordered (1.) */
    int list_marker;
    /* list item index (1-based, for ordered lists) */
    int list_index;

    /* overflow hidden */
    bool overflow_hidden;

    /* children */
    struct LayoutNode** children;
    size_t num_children;
} LayoutNode;

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

    /* overflow */
    const char* ov = get_style(sn, "overflow");
    if (ov && strcmp(ov, "hidden") == 0) ln->overflow_hidden = true;

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
    }

    /* border color */
    const char* bc = get_style(sn, "border-color");
    if (bc) ln->border_color = parse_color(bc);

    /* colors */
    const char* col = get_style(sn, "color");
    if (col) ln->color = parse_color(col);

    const char* bg = get_style(sn, "background-color");
    if (bg) ln->bg_color = parse_color(bg);
    /* also check 'background' shorthand */
    if (!ln->bg_color.valid) {
        bg = get_style(sn, "background");
        if (bg) ln->bg_color = parse_color(bg);
    }

    /* text styling */
    ln->font_bold = false;
    const char* fw = get_style(sn, "font-weight");
    if (fw && (strcmp(fw, "bold") == 0 || strcmp(fw, "700") == 0 || strcmp(fw, "bolder") == 0))
        ln->font_bold = true;

    ln->font_underline = false;
    const char* td = get_style(sn, "text-decoration");
    if (td && strcmp(td, "underline") == 0)
        ln->font_underline = true;

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
        if (wdim.valid && wdim.unit != 2) {
            child->width = resolve_dim(wdim, content_w,
                          content_w - child->margin_left - child->margin_right -
                          child->padding_left - child->padding_right -
                          child->border_left - child->border_right);
        } else {
            /* default: fill parent */
            child->width = content_w - child->margin_left - child->margin_right -
                           child->padding_left - child->padding_right -
                           child->border_left - child->border_right;
        }
        if (child->width < 0) child->width = 0;
    }

    /* second pass: position children and compute their heights */
    int inline_cursor = 0;
    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;

        if (child->display == DISPLAY_INLINE) {
            /* Inline children flow horizontally on the same line */
            child->width = content_w - child->margin_left - child->margin_right -
                           child->padding_left - child->padding_right -
                           child->border_left - child->border_right;
            if (child->width < 0) child->width = 0;
            /* Width from measured text */
            if (child->text_content) {
                int tw = uc_str_width(child->text_content);
                if (tw > 0) child->width = tw;
            }

            child->x = inline_cursor + child->margin_left + child->border_left + child->padding_left;
            child->y = child->margin_top + child->border_top + child->padding_top;

            compute_child_layouts(child, child->width);

            const char* h_str = get_style(child->styled, "height");
            if (h_str) {
                ParsedDim hdim = parse_dimension(h_str);
                if (hdim.valid && hdim.unit == 0) child->height = (int)hdim.value;
            }
            if (child->height < 1) child->height = 1;

            inline_cursor += child->width + child->padding_left + child->padding_right +
                             child->border_left + child->border_right +
                             child->margin_left + child->margin_right;
        } else {
            /* Block children: stack vertically, full width */
            child->x = child->margin_left + child->border_left + child->padding_left;
            child->y = y_cursor + child->margin_top + child->border_top + child->padding_top;

            compute_child_layouts(child, child->width);

            const char* h_str = get_style(child->styled, "height");
            if (h_str) {
                ParsedDim hdim = parse_dimension(h_str);
                if (hdim.valid && hdim.unit == 0) child->height = (int)hdim.value;
            }

            y_cursor += total_height(child) + child->margin_top + child->margin_bottom;
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

    for (size_t i = 0; i < parent->num_children; i++) {
        LayoutNode* child = parent->children[i];
        if (child->display == DISPLAY_NONE) continue;

        int child_main = natural_mains[i];

        /* flex-grow: distribute remaining */
        if (total_grow > 0 && remaining > 0) {
            int extra = (remaining * child->flex_grow) / total_grow;
            child_main += extra;
        }

        /* Set position and size */
        if (is_row) {
            child->x = main_cursor + child->margin_left + child->border_left + child->padding_left;
            child->y = child->margin_top + child->border_top + child->padding_top;
            child->width = child_main - child->margin_left - child->margin_right -
                           child->padding_left - child->padding_right -
                           child->border_left - child->border_right;

            /* cross axis: align-items */
            int available_cross = cross_size - child->margin_top - child->margin_bottom -
                                  child->padding_top - child->padding_bottom -
                                  child->border_top - child->border_bottom;

            const char* h_str = get_style(child->styled, "height");
            ParsedDim hdim = parse_dimension(h_str);
            if (hdim.valid && hdim.unit == 0) {
                child->height = (int)hdim.value;
            } else if (parent->align_items == ALIGN_STRETCH) {
                child->height = available_cross;
            } else {
                /* compute from children */
                compute_child_layouts(child, child->width);
                /* height is auto */
            }

            /* align-items positioning */
            switch (parent->align_items) {
                case ALIGN_CENTER:
                    child->y = (cross_size - total_height(child) - child->margin_top - child->margin_bottom) / 2
                               + child->margin_top + child->border_top + child->padding_top;
                    break;
                case ALIGN_END:
                    child->y = cross_size - total_height(child) - child->margin_bottom
                               + child->margin_top + child->border_top + child->padding_top;
                    break;
                default: break;
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

            if (parent->align_items == ALIGN_STRETCH) {
                child->width = available_cross;
            } else {
                const char* w_str = get_style(child->styled, "width");
                ParsedDim wdim = parse_dimension(w_str);
                if (wdim.valid && wdim.unit == 0) {
                    child->width = (int)wdim.value;
                }
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
    }

    free(natural_mains);

    /* set parent height (for row) or width (for column) */
    if (visible > 0) {
        int max_cross = 0;
        for (size_t i = 0; i < parent->num_children; i++) {
            LayoutNode* child = parent->children[i];
            if (child->display == DISPLAY_NONE) continue;
            if (is_row) {
                int bottom = child->y + child->height + child->padding_bottom +
                             child->border_bottom + child->margin_bottom;
                if (bottom > max_cross) max_cross = bottom;
            } else {
                int right = child->x + child->width + child->padding_right +
                            child->border_right + child->margin_right;
                if (right > max_cross) max_cross = right;
            }
        }
        if (is_row) {
            parent->height = max_cross;
        }
    }
}

/** Compute child layouts based on parent's display type */
/** Layout table: rows stack vertically, cells share width equally */
static void layout_table_children(LayoutNode* parent, int content_w) {
    if (content_w < 1) content_w = 1;

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

    /* Count max columns across all rows */
    size_t max_cols = 0;
    for (size_t i = 0; i < num_rows; i++) {
        size_t n = 0;
        for (size_t j = 0; j < rows[i]->num_children; j++)
            if (rows[i]->children[j]->display != DISPLAY_NONE) n++;
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
            int nat = cell->padding_left + cell->padding_right +
                      cell->border_left + cell->border_right;
            if (cell->text_content) nat += uc_str_width(cell->text_content);
            if (c < max_cols && nat > (int)col_w[c]) col_w[c] = nat;
            c++;
        }
    }

    int total_nat = 0;
    for (size_t c = 0; c < max_cols; c++) total_nat += col_w[c];
    if (total_nat < 1) total_nat = 1;

    int y_cursor = 0;
    for (size_t i = 0; i < num_rows; i++) {
        LayoutNode* row = rows[i];

        row->x = 0; row->y = y_cursor; row->width = content_w;

        int cx = 0, max_h = 0;
        size_t c = 0;
        for (size_t j = 0; j < row->num_children; j++) {
            LayoutNode* cell = row->children[j];
            if (cell->display == DISPLAY_NONE) continue;

            int cw = col_w[c] * content_w / total_nat;
            if (cw < 1) cw = 1;

            cell->x = cx + cell->border_left + cell->padding_left;
            cell->y = cell->border_top + cell->padding_top;
            cell->width = cw - cell->padding_left - cell->padding_right -
                          cell->border_left - cell->border_right;
            if (cell->width < 0) cell->width = 0;

            compute_child_layouts(cell, cell->width);

            int ch = cell->height + cell->padding_top + cell->padding_bottom +
                     cell->border_top + cell->border_bottom;
            if (ch > max_h) max_h = ch;
            cx += cw;
            c++;
        }

        for (size_t j = 0; j < row->num_children; j++) {
            LayoutNode* cell = row->children[j];
            if (cell->display == DISPLAY_NONE) continue;
            int target = max_h - cell->padding_top - cell->padding_bottom -
                         cell->border_top - cell->border_bottom;
            if (target > cell->height) cell->height = target;
        }
        row->height = max_h;
        y_cursor += max_h;
    }

    parent->height = y_cursor;
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

    /* apply styles */
    apply_styles(ln);

    if (ln->display == DISPLAY_NONE) {
        return ln; /* return but won't be rendered */
    }

    /* if we don't have a specific display from CSS, use default from tag */
    if (!get_style(snode, "display") && snode->node->type == GUMBO_NODE_ELEMENT) {
        ln->display = get_default_display(gumbo_normalized_tagname(snode->node->v.element.tag));
    }

    /* extract text content (skip for table/tr — they render via children) */
    const char* tag_name = NULL;
    if (snode->node->type == GUMBO_NODE_ELEMENT)
        tag_name = gumbo_normalized_tagname(snode->node->v.element.tag);
    
    if (tag_name && (strcmp(tag_name, "table") == 0 || strcmp(tag_name, "tr") == 0)) {
        ln->text_content = NULL;
    } else if (tag_name && strcmp(tag_name, "img") == 0) {
        ln->text_content = strdup("[img]");
    } else if (tag_name && snode->num_children > 0) {
        /* Node has element children — don't extract text, let children render it */
        ln->text_content = NULL;
    } else {
        ln->text_content = extract_text(snode->node);
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
                if (gc->type == GUMBO_NODE_TEXT && gc->v.text.text && *gc->v.text.text) {
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

        /* Walk Gumbo children in DOM order; interleave text fragments and element children */
        if (gchildren) {
            for (unsigned int gi = 0; gi < gchildren->length; gi++) {
                GumboNode* gc = (GumboNode*)gchildren->data[gi];
                if (gc->type == GUMBO_NODE_TEXT) {
                    /* Text node → create inline fragment */
                    if (!gc->v.text.text || !*gc->v.text.text) continue;
                    bool only_ws = true;
                    for (const char* cp = gc->v.text.text; *cp; cp++)
                        if (*cp != ' ' && *cp != '\t' && *cp != '\n' && *cp != '\r') { only_ws = false; break; }
                    if (only_ws) continue;
                    LayoutNode* tn = (LayoutNode*)calloc(1, sizeof(LayoutNode));
                    tn->text_content = strdup(gc->v.text.text);
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
                    if (child) ln->children[idx++] = child;
                    elem_i++;
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

    /* set default text color to white */

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

    /* <input>: render as [value] box */
    if (snode->node->type == GUMBO_NODE_ELEMENT &&
        snode->node->v.element.tag == GUMBO_TAG_INPUT) {
        const char* val = NULL;
        GumboAttribute* attr = gumbo_get_attribute(&snode->node->v.element.attributes, "value");
        if (attr && attr->value) val = attr->value;
        char buf[128];
        if (val && *val) {
            snprintf(buf, sizeof(buf), "[%s]", val);
        } else {
            strcpy(buf, "[  ]");
        }
        if (ln->text_content) free(ln->text_content);
        ln->text_content = strdup(buf);
        ln->color.r = 180; ln->color.g = 180; ln->color.b = 180;
        ln->color.valid = true;
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

    /* set default text color to white */
    if (!ln->color.valid) {
        ln->color.r = 255; ln->color.g = 255; ln->color.b = 255;
        ln->color.valid = true;
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

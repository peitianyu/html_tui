#ifndef STYLETREE_H
#define STYLETREE_H

#define GUMBO_IMPLEMENTATION
#include "gumbo.h"
#define KATANA_IMPLEMENTATION
#include "katana.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Data Structures ======================== */

/** A single style property key-value pair */
typedef struct {
    char* key;
    char* value;
} StyleProperty;

/** A dynamic map of style properties */
typedef struct {
    StyleProperty* data;
    size_t length;
    size_t capacity;
} StyleMap;

/** Forward declaration */
typedef struct StyledNode StyledNode;

/** A styled node: wraps a DOM node with its computed styles */
struct StyledNode {
    GumboNode* node;               /* the source DOM node */
    StyleMap styles;               /* computed style properties */
    StyledNode** children;         /* styled children */
    size_t num_children;           /* number of children */
};

/* ======================== StyleMap API ======================== */

/** Initialize a style map */
void style_map_init(StyleMap* map);

/** Free the contents of a style map */
void style_map_free(StyleMap* map);

/** Set a property in the map (overwrites existing) */
void style_map_set(StyleMap* map, const char* key, const char* value);

/** Get a property value (returns NULL if not found) */
const char* style_map_get(StyleMap* map, const char* key);

/* ======================== Style Tree API ======================== */

/* Set these before selector matching to enable :hover/:focus/:active */
static GumboNode* g_interact_hover = NULL;
static GumboNode* g_interact_focus = NULL;
static GumboNode* g_interact_active = NULL;

/**
 * Build the full style tree from a parsed DOM tree and a CSS stylesheet.
 * Returns the root StyledNode (the <html> element).
 */
StyledNode* build_style_tree(GumboNode* dom, KatanaStylesheet* stylesheet);

/** Free an entire style tree */
void free_style_tree(StyledNode* tree);

/** Get a computed style from a styled node (convenience wrapper) */
const char* get_style(StyledNode* node, const char* key);

/* ======================== Selector Matching API ======================== */

/** Check whether a CSS selector matches a DOM element node */
bool selector_matches_node(KatanaSelector* selector, GumboNode* node);

#ifdef __cplusplus
}
#endif

/* ==================================================================== */
/* IMPLEMENTATION                                                       */
/* ==================================================================== */
#ifdef STYLETREE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- StyleMap implementation ---------- */

#define INITIAL_MAP_CAPACITY 32

void style_map_init(StyleMap* map) {
    map->data = NULL;
    map->length = 0;
    map->capacity = 0;
}

void style_map_free(StyleMap* map) {
    for (size_t i = 0; i < map->length; i++) {
        free(map->data[i].key);
        free(map->data[i].value);
    }
    free(map->data);
    map->data = NULL;
    map->length = 0;
    map->capacity = 0;
}

void style_map_set(StyleMap* map, const char* key, const char* value) {
    if (!key || !value) return;

    /* overwrite existing entry */
    for (size_t i = 0; i < map->length; i++) {
        if (strcmp(map->data[i].key, key) == 0) {
            free(map->data[i].value);
            map->data[i].value = strdup(value);
            return;
        }
    }

    /* grow if needed */
    if (map->length >= map->capacity) {
        size_t new_cap = map->capacity ? map->capacity * 2 : INITIAL_MAP_CAPACITY;
        StyleProperty* new_data = (StyleProperty*)realloc(map->data, new_cap * sizeof(StyleProperty));
        if (!new_data) return;
        map->data = new_data;
        map->capacity = new_cap;
    }

    /* append new entry */
    map->data[map->length].key   = strdup(key);
    map->data[map->length].value = strdup(value);
    map->length++;
}

const char* style_map_get(StyleMap* map, const char* key) {
    if (!key) return NULL;
    for (size_t i = 0; i < map->length; i++) {
        if (strcmp(map->data[i].key, key) == 0)
            return map->data[i].value;
    }
    return NULL;
}

/* ---------- Helpers ---------- */

/** Check if a DOM node is an element */
static inline bool is_element_node(GumboNode* node) {
    return node && node->type == GUMBO_NODE_ELEMENT;
}

/** Get the class attribute value from a node, or NULL */
static const char* get_node_class(GumboNode* node) {
    if (!is_element_node(node)) return NULL;
    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "class");
    return attr ? attr->value : NULL;
}

/** Get the id attribute value from a node, or NULL */
static const char* get_node_id(GumboNode* node) {
    if (!is_element_node(node)) return NULL;
    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
    return attr ? attr->value : NULL;
}

/** Check if a class attribute contains a specific class name (space-separated) */
static bool has_class(const char* class_attr, const char* class_name) {
    if (!class_attr || !class_name) return false;
    size_t nlen = strlen(class_name);
    const char* p = class_attr;
    while (*p) {
        /* skip whitespace */
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (!*p) break;
        /* compare at word boundary */
        if (strncmp(p, class_name, nlen) == 0) {
            char c = p[nlen];
            if (c == '\0' || c == ' ' || c == '\t' || c == '\n' || c == '\r')
                return true;
        }
        /* advance to next space */
        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') p++;
    }
    return false;
}

/* ---------- Single-selector matching ---------- */

/**
 * Match one simple selector (one link in the chain) against a single element.
 * Returns true if this part of the selector matches the element.
 */
static bool match_simple_selector(KatanaSelector* sel, GumboNode* node) {
    if (!is_element_node(node)) return false;

    /* Tag check: if selector has a tag, it must match */
    if (sel->tag && sel->tag->local) {
        const char* sel_tag = sel->tag->local;
        if (strcmp(sel_tag, "*") != 0) {
            const char* node_tag = gumbo_normalized_tagname(node->v.element.tag);
            if (strcmp(sel_tag, node_tag) != 0)
                return false;
        }
    }

    switch (sel->match) {
        case KatanaSelectorMatchTag:
            break;

        case KatanaSelectorMatchClass: {
            if (!sel->data || !sel->data->value) return false;
            const char* cls = get_node_class(node);
            if (!cls) return false;
            if (!has_class(cls, sel->data->value)) return false;
            break;
        }

        case KatanaSelectorMatchId: {
            if (!sel->data || !sel->data->value) return false;
            const char* id = get_node_id(node);
            if (!id) return false;
            if (strcmp(id, sel->data->value) != 0) return false;
            break;
        }

        case KatanaSelectorMatchAttributeSet: {
            if (!sel->data || !sel->data->attribute) return false;
            const char* attr_name = sel->data->attribute->local;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attr_name);
            if (!attr) return false;
            break;
        }

        case KatanaSelectorMatchAttributeExact: {
            if (!sel->data || !sel->data->attribute) return false;
            const char* attr_name = sel->data->attribute->local;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, attr_name);
            if (!attr) return false;
            if (sel->data->value && strcmp(attr->value, sel->data->value) != 0) return false;
            break;
        }

        case KatanaSelectorMatchAttributeContain: {
            /* E[foo*="bar"]: value contains substring */
            if (!sel->data || !sel->data->attribute || !sel->data->value) return false;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, sel->data->attribute->local);
            if (!attr) return false;
            if (!strstr(attr->value, sel->data->value)) return false;
            break;
        }

        case KatanaSelectorMatchAttributeBegin: {
            /* E[foo^="bar"]: value starts with substring */
            if (!sel->data || !sel->data->attribute || !sel->data->value) return false;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, sel->data->attribute->local);
            if (!attr) return false;
            size_t vlen = strlen(sel->data->value);
            if (strncmp(attr->value, sel->data->value, vlen) != 0) return false;
            break;
        }

        case KatanaSelectorMatchAttributeEnd: {
            /* E[foo$="bar"]: value ends with substring */
            if (!sel->data || !sel->data->attribute || !sel->data->value) return false;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, sel->data->attribute->local);
            if (!attr) return false;
            size_t vlen = strlen(sel->data->value);
            size_t alen = strlen(attr->value);
            if (vlen > alen) return false;
            if (strcmp(attr->value + alen - vlen, sel->data->value) != 0) return false;
            break;
        }

        case KatanaSelectorMatchAttributeList: {
            /* E[foo~="bar"]: space-separated list contains value */
            if (!sel->data || !sel->data->attribute || !sel->data->value) return false;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, sel->data->attribute->local);
            if (!attr) return false;
            if (!has_class(attr->value, sel->data->value)) return false;
            break;
        }

        case KatanaSelectorMatchAttributeHyphen: {
            /* E[foo|="bar"]: value starts with "bar" or "bar-" */
            if (!sel->data || !sel->data->attribute || !sel->data->value) return false;
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, sel->data->attribute->local);
            if (!attr) return false;
            size_t vlen = strlen(sel->data->value);
            if (strncmp(attr->value, sel->data->value, vlen) != 0) return false;
            if (attr->value[vlen] != '\0' && attr->value[vlen] != '-') return false;
            break;
        }

        case KatanaSelectorMatchPseudoClass: {
            /* Structural and other pseudo-classes */
            if (!sel->data) break;
            GumboNode* parent = node->parent;
            GumboVector* sibs = parent ? &parent->v.element.children : NULL;

            /* Helper: count element siblings and get node index (1-based) */
            int elem_idx = 0, elem_total = 0;
            if (sibs) {
                for (unsigned int i = 0; i < sibs->length; i++) {
                    GumboNode* s = (GumboNode*)sibs->data[i];
                    if (s->type == GUMBO_NODE_ELEMENT) {
                        elem_total++;
                        if (s == node) elem_idx = elem_total;
                    }
                }
            }
            /* Helper: count same-tag siblings */
            int type_idx = 0, type_total = 0;
            if (sibs && node->type == GUMBO_NODE_ELEMENT) {
                GumboTag tag = node->v.element.tag;
                for (unsigned int i = 0; i < sibs->length; i++) {
                    GumboNode* s = (GumboNode*)sibs->data[i];
                    if (s->type == GUMBO_NODE_ELEMENT && s->v.element.tag == tag) {
                        type_total++;
                        if (s == node) type_idx = type_total;
                    }
                }
            }

            switch (sel->pseudo) {
                case KatanaPseudoFirstChild:
                    if (elem_idx != 1) return false;
                    break;
                case KatanaPseudoLastChild:
                    if (elem_idx != elem_total) return false;
                    break;
                case KatanaPseudoOnlyChild:
                    if (elem_total != 1) return false;
                    break;
                case KatanaPseudoFirstOfType:
                    if (type_idx != 1) return false;
                    break;
                case KatanaPseudoLastOfType:
                    if (type_idx != type_total) return false;
                    break;
                case KatanaPseudoOnlyOfType:
                    if (type_total != 1) return false;
                    break;
                case KatanaPseudoEmpty: {
                    /* No children (element or text) */
                    if (node->type != GUMBO_NODE_ELEMENT) break;
                    GumboVector* ch = &node->v.element.children;
                    bool empty = true;
                    for (unsigned int i = 0; i < ch->length; i++) {
                        GumboNode* c = (GumboNode*)ch->data[i];
                        if (c->type == GUMBO_NODE_ELEMENT ||
                            (c->type == GUMBO_NODE_TEXT && c->v.text.text && *(c->v.text.text))) {
                            empty = false; break;
                        }
                    }
                    if (!empty) return false;
                    break;
                }
                case KatanaPseudoNthChild:
                case KatanaPseudoNthLastChild: {
                    /* Parse "an+b" from argument */
                    const char* arg = sel->data->argument;
                    if (!arg) return false;
                    int a = 0, b = 0;
                    bool valid = false;
                    if (strcmp(arg, "odd") == 0) { a = 2; b = 1; valid = true; }
                    else if (strcmp(arg, "even") == 0) { a = 2; b = 0; valid = true; }
                    else {
                        /* Parse "an+b" format */
                        const char* p = arg;
                        if (*p == '-' || (*p >= '0' && *p <= '9') || *p == '+') {
                            char* end = NULL;
                            long n = strtol(p, &end, 10);
                            if (end && (*end == 'n' || *end == 'N')) {
                                a = (int)n;
                                p = end + 1;
                                if (*p == '+' || *p == '-') {
                                    b = (int)strtol(p, &end, 10);
                                }
                                valid = true;
                            } else if (end && *end == '\0') {
                                /* Just a number: nth-child(3) */
                                b = (int)n; a = 0; valid = true;
                            }
                        }
                    }
                    if (!valid) break;
                    int pos = (sel->pseudo == KatanaPseudoNthChild) ? elem_idx : elem_total - elem_idx + 1;
                    if (pos < 1) return false;
                    if (a == 0) { if (pos != b) return false; }
                    else { int diff = pos - b; if (diff < 0 || diff % a != 0) return false; }
                    break;
                }
                case KatanaPseudoLink:
                case KatanaPseudoAnyLink:
                    /* Match <a> elements with href attribute */
                    if (node->type != GUMBO_NODE_ELEMENT) return false;
                    if (node->v.element.tag != GUMBO_TAG_A) return false;
                    if (!gumbo_get_attribute(&node->v.element.attributes, "href")) return false;
                    break;
                case KatanaPseudoHover:
                    if (node != g_interact_hover) return false;
                    break;
                case KatanaPseudoFocus:
                    if (node != g_interact_focus) return false;
                    break;
                case KatanaPseudoActive:
                    if (node != g_interact_active) return false;
                    break;
                default:
                    /* Skip unsupported pseudo-classes */
                    break;
            }
            break;
        }

        default:
            break;
    }

    return true;
}

/**
 * Recursively check if a selector chain matches a node and its ancestors,
 * walking up the DOM tree for descendant combinator.
 */
static bool match_selector_chain_to_ancestors(KatanaSelector* cur, GumboNode* cur_node) {
    if (!cur || !cur_node) return false;
    if (!is_element_node(cur_node)) return false;

    /* Check current node against the "rightmost" (current) part */
    if (!match_simple_selector(cur, cur_node))
        return false;

    KatanaSelector* next = cur->tagHistory;
    if (!next) return true; /* full chain matched */

    /* There's more to match — follow the combinator */
    switch (cur->relation) {
        case KatanaSelectorRelationSubSelector:
            /* Both parts apply to same element */
            return match_selector_chain_to_ancestors(next, cur_node);

        case KatanaSelectorRelationChild:
            /* Next part must match the parent */
            if (!cur_node->parent || !is_element_node(cur_node->parent))
                return false;
            return match_selector_chain_to_ancestors(next, cur_node->parent);

        case KatanaSelectorRelationDescendant:
            /* Next part must match some ancestor */
            for (GumboNode* a = cur_node->parent; a; a = a->parent) {
                if (is_element_node(a)) {
                    if (match_selector_chain_to_ancestors(next, a))
                        return true;
                }
            }
            return false;

        case KatanaSelectorRelationDirectAdjacent: {
            /* Next part must match the immediately preceding element sibling */
            GumboNode* parent = cur_node->parent;
            if (!parent) return false;
            GumboVector* sibs = &parent->v.element.children;
            int idx = -1;
            for (unsigned int i = 0; i < sibs->length; i++)
                if ((GumboNode*)sibs->data[i] == cur_node) { idx = (int)i; break; }
            if (idx <= 0) return false;
            GumboNode* prev = NULL;
            for (int i = idx - 1; i >= 0; i--) {
                GumboNode* s = (GumboNode*)sibs->data[i];
                if (s->type == GUMBO_NODE_ELEMENT) { prev = s; break; }
            }
            if (!prev) return false;
            return match_selector_chain_to_ancestors(next, prev);
        }

        case KatanaSelectorRelationIndirectAdjacent: {
            /* Next part must match any preceding element sibling */
            GumboNode* parent = cur_node->parent;
            if (!parent) return false;
            GumboVector* sibs = &parent->v.element.children;
            int idx = -1;
            for (unsigned int i = 0; i < sibs->length; i++)
                if ((GumboNode*)sibs->data[i] == cur_node) { idx = (int)i; break; }
            if (idx <= 0) return false;
            for (int i = idx - 1; i >= 0; i--) {
                GumboNode* s = (GumboNode*)sibs->data[i];
                if (s->type == GUMBO_NODE_ELEMENT) {
                    if (match_selector_chain_to_ancestors(next, s))
                        return true;
                }
            }
            return false;
        }

        default:
            return false;
    }
}

/**
 * Match a full compound/complex selector against a node.
 */
bool selector_matches_node(KatanaSelector* selector, GumboNode* node) {
    if (!selector || !node) return false;
    return match_selector_chain_to_ancestors(selector, node);
}

/* ---------- Specificity ---------- */

/** Compute specificity of a full selector chain */
static size_t compute_specificity(KatanaSelector* selector) {
    size_t a = 0, b = 0, c = 0; /* inline, id, class/attr/pseudo, tag/pseudo-el */
    KatanaSelector* cur = selector;
    while (cur) {
        switch (cur->match) {
            case KatanaSelectorMatchId:
                a++;
                break;
            case KatanaSelectorMatchClass:
            case KatanaSelectorMatchPseudoClass:
            case KatanaSelectorMatchAttributeExact:
            case KatanaSelectorMatchAttributeSet:
            case KatanaSelectorMatchAttributeList:
            case KatanaSelectorMatchAttributeHyphen:
            case KatanaSelectorMatchAttributeContain:
            case KatanaSelectorMatchAttributeBegin:
            case KatanaSelectorMatchAttributeEnd:
                b++;
                break;
            case KatanaSelectorMatchTag:
            case KatanaSelectorMatchPseudoElement:
                c++;
                break;
            default:
                c++;
                break;
        }
        cur = cur->tagHistory;
    }
    /* Pack into a single comparable value */
    return (a << 24) | (b << 12) | c;
}

/* ---------- Collect matching rules ---------- */

typedef struct {
    KatanaStyleRule* rule;
    size_t specificity;
    unsigned int source_order;
} MatchedRule;

static int compare_matched_rules(const void* a, const void* b) {
    const MatchedRule* ra = (const MatchedRule*)a;
    const MatchedRule* rb = (const MatchedRule*)b;
    if (ra->specificity != rb->specificity)
        return ra->specificity < rb->specificity ? -1 : 1;
    /* same specificity: source order (later wins) */
    if (ra->source_order != rb->source_order)
        return ra->source_order < rb->source_order ? -1 : 1;
    return 0;
}

/** Collect all matching style rules for a node */
static void collect_matching_rules(GumboNode* node, KatanaStylesheet* ss,
                                   MatchedRule** out, size_t* out_len) {
    size_t cap = 64;
    size_t len = 0;
    MatchedRule* arr = (MatchedRule*)malloc(cap * sizeof(MatchedRule));

    for (unsigned int i = 0; i < ss->rules.length; i++) {
        KatanaRule* base = (KatanaRule*)ss->rules.data[i];
        if (base->type != KatanaRuleStyle) continue;

        KatanaStyleRule* sr = (KatanaStyleRule*)base;
        if (!sr->selectors) continue;

        for (unsigned int j = 0; j < sr->selectors->length; j++) {
            KatanaSelector* sel = (KatanaSelector*)sr->selectors->data[j];
            if (selector_matches_node(sel, node)) {
                if (len >= cap) {
                    cap *= 2;
                    arr = (MatchedRule*)realloc(arr, cap * sizeof(MatchedRule));
                }
                arr[len].rule = sr;
                arr[len].specificity = compute_specificity(sel);
                arr[len].source_order = i;
                len++;
                /* Only count once per rule (first matching selector) */
                break;
            }
        }
    }

    *out = arr;
    *out_len = len;
}

/* ---------- Inherit styles from parent ---------- */
/* Properties that inherit by default in CSS */
#define IS_INHERITED_PROP(k) ( \
    strcmp((k), "color") == 0 || \
    strcmp((k), "font-family") == 0 || \
    strcmp((k), "font-size") == 0 || \
    strcmp((k), "font-style") == 0 || \
    strcmp((k), "font-weight") == 0 || \
    strcmp((k), "line-height") == 0 || \
    strcmp((k), "text-align") == 0 || \
    strcmp((k), "text-indent") == 0 || \
    strcmp((k), "visibility") == 0 || \
    strcmp((k), "word-spacing") == 0 || \
    strcmp((k), "letter-spacing") == 0 || \
    strcmp((k), "white-space") == 0 || \
    strcmp((k), "list-style") == 0 || \
    strcmp((k), "cursor") == 0 \
)

/** Inherit inheritable properties from parent map */
static void inherit_styles(StyleMap* dest, StyledNode* parent) {
    if (!parent) return;
    for (size_t i = 0; i < parent->styles.length; i++) {
        const char* key = parent->styles.data[i].key;
        if (IS_INHERITED_PROP(key)) {
            /* Only set if not already explicitly set */
            if (!style_map_get(dest, key)) {
                style_map_set(dest, key, parent->styles.data[i].value);
            }
        }
    }
}

/* Extract value string from a KatanaValue */
static const char* value_get_text(KatanaValue* val) {
    /* For identifier-like values, use the string field */
    switch (val->unit) {
        case KATANA_VALUE_IDENT:
        case KATANA_VALUE_STRING:
        case KATANA_VALUE_URI:
        case KATANA_VALUE_ATTR:
        case KATANA_VALUE_COUNTER:
        case KATANA_VALUE_PARSER_HEXCOLOR:
        case KATANA_VALUE_PARSER_FUNCTION:
            if (val->string) return val->string;
            break;
        case KATANA_VALUE_PX:
        case KATANA_VALUE_EMS:
        case KATANA_VALUE_EXS:
        case KATANA_VALUE_REMS:
        case KATANA_VALUE_CHS:
        case KATANA_VALUE_VW:
        case KATANA_VALUE_VH:
        case KATANA_VALUE_VMIN:
        case KATANA_VALUE_VMAX:
        case KATANA_VALUE_PERCENTAGE:
        case KATANA_VALUE_CM:
        case KATANA_VALUE_MM:
        case KATANA_VALUE_IN:
        case KATANA_VALUE_PT:
        case KATANA_VALUE_PC:
        case KATANA_VALUE_DEG:
        case KATANA_VALUE_RAD:
        case KATANA_VALUE_GRAD:
        case KATANA_VALUE_MS:
        case KATANA_VALUE_S:
        case KATANA_VALUE_HZ:
        case KATANA_VALUE_KHZ:
        case KATANA_VALUE_DIMENSION:
        case KATANA_VALUE_NUMBER:
        case KATANA_VALUE_VARIABLE_NAME:
            /* For numeric values, raw usually has the text representation */
            if (val->raw) return val->raw;
            break;
        default:
            if (val->raw) return val->raw;
            break;
    }
    return NULL;
}

/* ---------- Compute styles for one node ---------- */

/** Concatenate KatanaValue array into a space-separated string. Caller must free. */
static char* concat_values_to_str(KatanaArray* values) {
    if (!values || values->length == 0) return NULL;
    size_t total = 0;
    for (unsigned int v = 0; v < values->length; v++) {
        const char* txt = value_get_text((KatanaValue*)values->data[v]);
        if (txt) total += strlen(txt) + 1;
    }
    if (total == 0) return NULL;
    char* val_str = (char*)malloc(total + 1);
    val_str[0] = '\0';
    for (unsigned int v = 0; v < values->length; v++) {
        KatanaValue* val = (KatanaValue*)values->data[v];
        const char* txt = value_get_text(val);
        if (txt) {
            if (val_str[0]) strcat(val_str, " ");
            strcat(val_str, txt);
        }
    }
    return val_str;
}

static void compute_styles_for_node(GumboNode* node, KatanaStylesheet* ss,
                                    StyledNode* parent_snode, StyleMap* out) {
    style_map_init(out);

    /* 1. Inherit from parent first */
    inherit_styles(out, parent_snode);

    /* 2. Collect and sort matching rules */
    MatchedRule* rules = NULL;
    size_t num_rules = 0;
    collect_matching_rules(node, ss, &rules, &num_rules);

    if (num_rules > 0) {
        qsort(rules, num_rules, sizeof(MatchedRule), compare_matched_rules);

        /* 3. Apply declarations */
        for (size_t r = 0; r < num_rules; r++) {
            KatanaStyleRule* sr = rules[r].rule;
            if (!sr->declarations) continue;
            for (unsigned int d = 0; d < sr->declarations->length; d++) {
                KatanaDeclaration* decl = (KatanaDeclaration*)sr->declarations->data[d];
                if (!decl->property) continue;
                if (decl->values && decl->values->length > 0) {
                    char* val_str = concat_values_to_str(decl->values);
                    if (val_str) {
                        style_map_set(out, decl->property, val_str);
                        free(val_str);
                    }
                }
            }
        }
        free(rules);
    }

    /* 4. Apply inline styles (style attribute) - highest priority */
    if (is_element_node(node)) {
        GumboAttribute* style_attr = gumbo_get_attribute(&node->v.element.attributes, "style");
        if (style_attr && style_attr->value) {
            KatanaOutput* inline_out = katana_parse(
                style_attr->value, strlen(style_attr->value),
                KatanaParserModeDeclarationList);
            if (inline_out && inline_out->declarations) {
                for (unsigned int i = 0; i < inline_out->declarations->length; i++) {
                    KatanaDeclaration* decl = (KatanaDeclaration*)inline_out->declarations->data[i];
                    if (!decl->property) continue;
                    if (decl->values && decl->values->length > 0) {
                        char* val_str = concat_values_to_str(decl->values);
                        if (val_str) {
                            style_map_set(out, decl->property, val_str);
                            free(val_str);
                        }
                    }
                }
            }
            if (inline_out) katana_destroy_output(inline_out);
        }
    }
}

/* ---------- Recursive style tree builder ---------- */

static StyledNode* build_style_tree_recursive(GumboNode* dom, KatanaStylesheet* ss,
                                               StyledNode* parent_snode) {
    if (!dom) return NULL;

    StyledNode* snode = (StyledNode*)calloc(1, sizeof(StyledNode));
    snode->node = dom;

    /* Compute styles for this node */
    compute_styles_for_node(dom, ss, parent_snode, &snode->styles);

    /* Recurse into children (only for element/document nodes) */
    GumboVector* children = NULL;
    if (dom->type == GUMBO_NODE_ELEMENT) {
        children = &dom->v.element.children;
    } else if (dom->type == GUMBO_NODE_DOCUMENT) {
        children = &dom->v.document.children;
    }

    if (children && children->length > 0) {
        /* First count element children */
        size_t elem_count = 0;
        for (unsigned int i = 0; i < children->length; i++) {
            GumboNode* child = (GumboNode*)children->data[i];
            if (child->type == GUMBO_NODE_ELEMENT)
                elem_count++;
        }

        if (elem_count > 0) {
            snode->children = (StyledNode**)calloc(elem_count, sizeof(StyledNode*));
            size_t idx = 0;
            for (unsigned int i = 0; i < children->length; i++) {
                GumboNode* child = (GumboNode*)children->data[i];
                if (child->type == GUMBO_NODE_ELEMENT) {
                    snode->children[idx] = build_style_tree_recursive(child, ss, snode);
                    idx++;
                }
            }
            snode->num_children = idx;
        }
    }

    return snode;
}

/* ---------- Public API ---------- */

StyledNode* build_style_tree(GumboNode* dom, KatanaStylesheet* stylesheet) {
    if (!dom) return NULL;
    return build_style_tree_recursive(dom, stylesheet, NULL);
}

void free_style_tree(StyledNode* tree) {
    if (!tree) return;
    for (size_t i = 0; i < tree->num_children; i++) {
        free_style_tree(tree->children[i]);
    }
    free(tree->children);
    style_map_free(&tree->styles);
    free(tree);
}

/**
 * Recompute styles for a single styled node (and its descendants).
 * Used when hover/focus/active state changes.
 * Pass the global KatanaStylesheet that was used in build_style_tree.
 */
void recompute_style_subtree(StyledNode* node, KatanaStylesheet* ss, StyledNode* parent) {
    if (!node) return;
    /* Recompute this node's styles */
    style_map_free(&node->styles);
    compute_styles_for_node(node->node, ss, parent ? parent : NULL, &node->styles);

    /* Recurse into children */
    for (size_t i = 0; i < node->num_children; i++) {
        recompute_style_subtree(node->children[i], ss, node);
    }
}

const char* get_style(StyledNode* node, const char* key) {
    if (!node || !key) return NULL;
    return style_map_get(&node->styles, key);
}

/* ---------- Internal forward declarations used above ---------- */
/* (already defined above) */

#endif /* STYLETREE_IMPLEMENTATION */

#endif /* STYLETREE_H */

/* Debug: check what Gumbo children <p> has */
#include <stdio.h>
#include <string.h>

#define GUMBO_IMPLEMENTATION
#include "core/gumbo.h"

int main() {
    GumboOutput* dom = gumbo_parse(
        "<html><body><p>Search: <input value='hi'> <button>Go</button></p></body></html>"
    );
    GumboNode* body = NULL;
    for (unsigned i = 0; i < dom->root->v.element.children.length; i++) {
        GumboNode* n = dom->root->v.element.children.data[i];
        if (n->type == GUMBO_NODE_ELEMENT && n->v.element.tag == GUMBO_TAG_BODY)
            body = n;
    }
    if (body) {
        for (unsigned i = 0; i < body->v.element.children.length; i++) {
            GumboNode* n = body->v.element.children.data[i];
            printf("body child[%u]: type=%d ", i, n->type);
            if (n->type == GUMBO_NODE_ELEMENT)
                printf("tag=%s", gumbo_normalized_tagname(n->v.element.tag));
            else if (n->type == GUMBO_NODE_TEXT)
                printf("text=[%s]", n->v.text.text);
            printf("\n");
        }
        /* Find <p> */
        for (unsigned i = 0; i < body->v.element.children.length; i++) {
            GumboNode* n = body->v.element.children.data[i];
            if (n->type == GUMBO_NODE_ELEMENT && n->v.element.tag == GUMBO_TAG_P) {
                printf("\n<p> has %u children:\n", n->v.element.children.length);
                for (unsigned j = 0; j < n->v.element.children.length; j++) {
                    GumboNode* c = n->v.element.children.data[j];
                    printf("  child[%u]: type=%d ", j, c->type);
                    if (c->type == GUMBO_NODE_ELEMENT)
                        printf("tag=%s", gumbo_normalized_tagname(c->v.element.tag));
                    else if (c->type == GUMBO_NODE_TEXT)
                        printf("text=[%s]", c->v.text.text);
                    printf("\n");
                }
            }
        }
    }
    return 0;
}

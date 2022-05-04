#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ustr.h"

struct Node
{
    struct Node *next;
    struct UStr ustr[];
};

static struct Node *node;

struct UStr *
makeUStr_(const char *s, bool *added)
{
    size_t len = strlen(s);
    if (added) {
	*added = true;
    }
    for (struct Node *n = node; n; n = n->next) {
	if (len == n->ustr->len && !strcmp(s, n->ustr->cstr)) {
	    if (added) {
		*added = false;
	    }
	    return n->ustr;
	}
    }

    struct Node *n = malloc(len + 1 + sizeof(size_t) + sizeof(struct Node *));
    if (!n) {
	fprintf(stderr, "makeUStr: out of memory\n");
	abort();
    }

    n->next = node;
    n->ustr->len = len;
    strcpy(n->ustr->cstr, s);
    node = n;
    return node->ustr;
}

struct UStr *
makeUStr(const char *s)
{
    return makeUStr_(s, 0);
}

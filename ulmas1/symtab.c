#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "symtab.h"
#include <utils/loc.h>

struct SymtabNode
{
    const struct UStr *ident;
    const struct Expr *val;
    bool global;
    struct SymtabNode *next;
} * symtab, *symtabLast;

static struct SymtabNode *
findSym(const struct UStr *ident)
{
    for (struct SymtabNode *n = symtab; n; n = n->next) {
	if (n->ident == ident) {
	    return n;
	}
    }
    return 0;
}

static void
setSym(struct SymtabNode *n, const struct Expr *val)
{
    assert(n);
    n->val = val;
}

static struct SymtabNode *
addSym(const struct UStr *ident, const struct Expr *val)
{
    struct SymtabNode *node = malloc(sizeof(*node));
    if (!node) {
	fprintf(stderr, "'addSym': out of memory\n");
	exit(1);
    }

    node->ident = ident;
    node->val = val;
    node->global = false;
    node->next = 0;

    if (symtab) {
	symtabLast = symtabLast->next = node;
    } else {
	symtab = symtabLast = node;
    }
    return node;
}

void
symtabSet(const struct UStr *ident, const struct Expr *val)
{
    struct SymtabNode *n = findSym(ident);

    if (n) {
	setSym(n, val);
    } else {
	addSym(ident, val);
    }
}

void
symtabGlobal(const struct UStr *ident)
{
    struct SymtabNode *n = findSym(ident);

    if (!n) {
	n = addSym(ident, makeValExpr(nullLoc, UNKNOWN, 0));
    }
    n->global = true;
}

void
symtabGet(const struct UStr *ident, const struct UStr *recGuard,
	  enum ExprType *type, uint64_t *val)
{
    struct SymtabNode *n = findSym(ident);

    if (!n) {
	n = addSym(ident, makeValExpr(nullLoc, UNKNOWN, 0));
    }
    *type = typeExpr(n->val);

    if (ident == recGuard) {
	error("symbol '%s' recursively defined\n", ident->cstr);
    } else {
	*val = evalExprWithRecGuard(n->val, recGuard ? recGuard : ident);
    }
}

size_t
symtabNumUnresolvable()
{
    size_t numUnresolvable = 0;
    for (struct SymtabNode *n = symtab; n; n = n->next) {
	evalExpr(n->val);
	if (typeExpr(n->val) == UNKNOWN) {
	    ++numUnresolvable;
	}
    }
    return numUnresolvable;
}

void
symtabResolve()
{
    size_t run1 = symtabNumUnresolvable();
    size_t run2 = symtabNumUnresolvable();
    while (run1 != run2 && run2 != 0) {
	run1 = run2;
	run2 = symtabNumUnresolvable();
    }
}

void
symtabPrint(FILE *out)
{
    symtabResolve();
    fprintf(out, "#SYMTAB\n");
    for (struct SymtabNode *n = symtab; n; n = n->next) {
	uint64_t val = evalExpr(n->val);
	int type = typeExpr(n->val);

	fprintf(out, "%c ", (char)(n->global ? toupper(type) : type));
	fprintf(out, "%-20s ", n->ident->cstr);
	fprintf(out, "0x%016" PRIX64 "\n", val);
    }
}

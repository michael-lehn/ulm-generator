#ifndef ULMAS1_SYMTAB_H
#define ULMAS1_SYMTAB_H

#include <stdint.h>
#include <stdio.h>

#include "expr.h"
#include <utils/ustr.h>

struct SymtabEntry;

void symtabSet(const struct UStr *ident, const struct Expr *val);
void symtabGlobal(const struct UStr *ident);
void symtabGet(const struct UStr *ident, const struct UStr *recGuard,
	       enum ExprType *type, uint64_t *val);
void symtabPrint(FILE *out);

#endif // ULMAS1_SYMTAB_H

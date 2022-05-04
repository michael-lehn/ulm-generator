#ifndef ULMAS1_EXPR_H
#define ULMAS1_EXPR_H

#include <stdbool.h>
#include <stdint.h>

#include <utils/loc.h>
#include <utils/ustr.h>

struct Expr;

enum ExprType
{
    ABS = 'a',
    REL_TEXT = 't',
    REL_DATA = 'd',
    REL_BSS = 'b',
    UNKNOWN = 'U',
};

enum ExprBinaryOperand
{
    EO_PLUS = '+',
    EO_MINUS = '-',
    EO_ASTERISK = '*',
    EO_SLASH = '/',
    EO_PERCENT = '%',
};

struct Expr *makeValExpr(struct Loc loc, enum ExprType type, uint64_t val);
struct Expr *makeSymExpr(struct Loc loc, const struct UStr *sym);
struct Expr *makeUnaryMinusExpr(struct Loc loc, const struct Expr *x);
struct Expr *makeUnaryWordExpr(struct Loc loc, size_t word,
			       const struct Expr *x);
struct Expr *makeBinaryExpr(struct Loc loc, enum ExprBinaryOperand exprOperand,
			    const struct Expr *x, const struct Expr *y);

void freeExpr(struct Expr *x);

uint64_t evalExpr(const struct Expr *x);
uint64_t evalExprWithRecGuard(const struct Expr *x, const struct UStr *guard);
enum ExprType typeExpr(const struct Expr *x);
struct Loc locExpr(const struct Expr *x);

bool symInExpr(const struct UStr *ident, const struct Expr *x);
void printTreeExpr(const struct Expr *x);
const char *strExpr(const struct Expr *x);
const char *relocStrExpr(const struct Expr *x);
void printExpr(const struct Expr *x);

#endif // ULMAS1_EXPR_H

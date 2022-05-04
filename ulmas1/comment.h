#ifndef ULMAS1_COMMENT_H
#define ULMAS1_COMMENT_H

#include "expr.h"
#include "lexer.h"

void commentClear(void);
void commentAddExpr(const struct Expr *expr);
void commentAddCStr(const char *str);
void commentAddLiteral(uint64_t val);
void commentAddToken(enum TokenKind);
void commentAddCurrentToken(void);
const char *commentGetCStr(void);

#endif // ULMAS1_COMMENT_H


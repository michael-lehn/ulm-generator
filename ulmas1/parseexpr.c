#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "error.h"
#include "expected.h"
#include "lexer.h"
#include "parseexpr.h"

static struct Expr *parseExpr();
static struct Expr *parseBinaryExpr(size_t priority);
static struct Expr *parseUnaryExpr();
static struct Expr *parsePrimaryExpr();

static bool
wordToken(size_t *word)
{
    assert(word);
    switch (token.kind) {
	case W0:
	    *word = 0;
	    return true;
	case W1:
	    *word = 1;
	    return true;
	case W2:
	    *word = 2;
	    return true;
	case W3:
	    *word = 3;
	    return true;
	default:
	    return false;
    }
}

struct Expr *
parseExpression()
{
    size_t word = 0;
    if (wordToken(&word)) {
	struct Token op = token;
	getToken();
	expected(LPAREN);
	getToken();
	struct Expr *expr = parseExpr();
	if (!expr) {
	    error("Expression expected\n");
	}
	expected(RPAREN);
	getToken();
	return makeUnaryWordExpr(op.loc, word, expr);
    }

    return parseBinaryExpr(1);
}

static struct Expr *
parseExpr()
{
    return parseBinaryExpr(1);
}

static size_t
binaryOpPriority(enum TokenKind t)
{
    switch (t) {
	case ASTERISK:
	case SLASH:
	case PERCENT:
	    return 2;
	case PLUS:
	case MINUS:
	    return 1;
	default:
	    return 0;
    }
}

static enum ExprBinaryOperand
binaryOp(enum TokenKind t)
{
    switch (t) {
	case PLUS:
	    return EO_PLUS;
	case MINUS:
	    return EO_MINUS;
	case ASTERISK:
	    return EO_ASTERISK;
	case SLASH:
	    return EO_SLASH;
	case PERCENT:
	    return EO_PERCENT;
	default:
	    fprintf(stderr, "internal error in 'binaryOp'\n");
	    fprintf(stderr, "unexpected token %s\n", strTokenKind(t));
	    abort();
	    return 0;
    }
}

static struct Expr *
parseBinaryExpr(size_t priority)
{
    struct Expr *expr = parseUnaryExpr();
    if (!expr) {
	return 0;
    }

    for (size_t p = binaryOpPriority(token.kind); p >= priority; --p) {
	while (binaryOpPriority(token.kind) == p) {
	    struct Token op = token;
	    getToken();
	    struct Expr *right = parseBinaryExpr(p + 1);
	    expr = makeBinaryExpr(op.loc, binaryOp(op.kind), expr, right);
	}
    }
    return expr;
}

static struct Expr *
parseUnaryExpr()
{
    while (token.kind == PLUS) {
	getToken();
    }
    if (token.kind == MINUS) {
	struct Token op = token;
	getToken();
	return makeUnaryMinusExpr(op.loc, parseUnaryExpr());
    }
    return parsePrimaryExpr();
}

static uint64_t
hexToUint(const char *s)
{
    s += 2;
    uint64_t val = 0, digit;
    while ((digit = *s++)) {
	if (digit >= '0' && digit <= '9') {
	    digit -= '0';
	} else if (digit >= 'a' && digit <= 'f') {
	    digit -= 'a' + 10;
	} else if (digit >= 'A' && digit <= 'F') {
	    digit -= 'A' + 10;
	}
	val = val * 16 + digit;
    }
    return val;
}

static uint64_t
octToUint(const char *s)
{
    ++s;
    uint64_t val = 0, digit;
    while ((digit = *s++)) {
	digit -= '0';
	val = val * 8 + digit;
    }
    return val;
}

static uint64_t
decToUint(const char *s)
{
    uint64_t val = 0, digit;
    while ((digit = *s++)) {
	digit -= '0';
	val = val * 10 + digit;
    }
    return val;
}


static struct Expr *
parsePrimaryExpr()
{
    struct Expr *expr = 0;

    switch (token.kind) {
	case CHARACTER_LITERAL: {
	    uint64_t val = token.processedVal.cstr[0];
	    expr = makeValExpr(token.loc, ABS, val);
	    getToken();
	} break;
	case HEXADECIMAL_LITERAL: {
	    uint64_t val = hexToUint(token.val.cstr);
	    expr = makeValExpr(token.loc, ABS, val);
	    getToken();
	} break;
	case OCTAL_LITERAL: {
	    uint64_t val = octToUint(token.val.cstr);
	    expr = makeValExpr(token.loc, ABS, val);
	    getToken();
	} break;
	case DECIMAL_LITERAL: {
	    uint64_t val = decToUint(token.val.cstr);
	    expr = makeValExpr(token.loc, ABS, val);
	    getToken();
	} break;
	case IDENT:
	    expr = makeSymExpr(token.loc, makeUStr(token.val.cstr));
	    getToken();
	    break;
	case LPAREN:
	    getToken();
	    expr = parseExpr();
	    expected(RPAREN);
	    getToken();
	default:;
    }

    return expr;
}

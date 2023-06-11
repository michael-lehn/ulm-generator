#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "cg.h"
#include "expr.h"
#include "symtab.h"
#include <utils/str.h>

enum ExprKind
{
    VAL = 'v',
    SYM = 's',
    NEG = 'n',
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/',
    MOD = '%',
    W0 = '0',
    W1 = '1',
    W2 = '2',
    W3 = '3',
};

struct Expr
{
    struct Loc loc;
    enum ExprKind kind;
    union
    {
	uint64_t val;		  // if kind == VAL
	const struct UStr *sym;	  // if kind == SYM
	const struct Expr *unary; // if kind == NEG
	struct			  // if kind represents a binary op (e.g. ADD)
	{
	    const struct Expr *left;
	    const struct Expr *right;
	} binary;
    } u;
    struct Mutable // fields are mutable
    {
	uint64_t val; // set if type != UNKNOWN
	enum ExprType type;
    } m;
};

static struct Expr *
allocExpr()
{
    struct Expr *x = malloc(sizeof(*x));
    if (!x) {
	fprintf(stderr, "'allocExpr': out of memory\n");
	exit(1);
    }
    return x;
}

static enum ExprType
typeUnary(const struct Expr *x)
{
    assert(x->kind == NEG || (x->kind >= W0 && x->kind <= W3));
    if (x->kind == NEG && x->u.unary->m.type == ABS) {
	return ABS;
    } else if (x->kind >= W0 && x->kind <= W3) {
	return x->u.unary->m.type;
    }
    return UNKNOWN;
}

static enum ExprType
typeBinary(const struct Expr *x)
{
    enum ExprType leftType = x->u.binary.left->m.type;
    enum ExprType rightType = x->u.binary.right->m.type;

    if (leftType == UNKNOWN || rightType == UNKNOWN) {
	return UNKNOWN;
    } else if (leftType == ABS && rightType == ABS) {
	return ABS;
    } else if (x->kind == ADD) {
	if (leftType == ABS || rightType == ABS) {
	    return leftType == ABS ? rightType : leftType;
	}
    } else if (x->kind == SUB) {
	if (rightType == ABS) {
	    return leftType;
	} else if (leftType == rightType) {
	    return ABS;
	}
    }
    // printTreeExpr(x);
    errorAt(x->loc, "illegal expression\n");
    return UNKNOWN;
}

struct Expr *
makeValExpr(struct Loc loc, enum ExprType type, uint64_t val)
{
    struct Expr *x = allocExpr();
    x->loc = loc;
    x->kind = VAL;
    x->u.val = val;
    x->m.type = type;
    x->m.val = x->u.val;
    return x;
}

struct Expr *
makeSymExpr(struct Loc loc, const struct UStr *sym)
{
    struct Expr *x = allocExpr();
    x->loc = loc;
    x->kind = SYM;
    x->u.sym = sym;
    x->m.type = UNKNOWN;
    return x;
}

struct Expr *
makeUnaryMinusExpr(struct Loc loc, const struct Expr *x)
{
    if (!x) {
	return 0;
    }
    struct Expr *e = allocExpr();
    e->loc = loc;
    e->kind = NEG;
    e->u.unary = x;
    e->m.type = UNKNOWN;
    return e;
}

struct Expr *
makeUnaryWordExpr(struct Loc loc, size_t word, const struct Expr *x)
{
    if (!x) {
	return 0;
    }
    struct Expr *e = allocExpr();
    e->loc = loc;
    e->kind = word + '0';
    e->u.unary = x;
    e->m.type = UNKNOWN;
    return e;
}

struct Expr *
makeBinaryExpr(struct Loc loc, enum ExprBinaryOperand exprOperand,
	       const struct Expr *x, const struct Expr *y)
{
    if (!x || !y) {
	return 0;
    }
    struct Expr *e = allocExpr();
    e->loc = loc;
    e->kind = (enum ExprKind)exprOperand;
    e->u.binary.left = x;
    e->u.binary.right = y;
    e->m.type = UNKNOWN;
    return e;
}

void
freeExpr(struct Expr *x)
{
    if (!x) {
	return;
    }
    switch (x->kind) {
	case NEG:
	    freeExpr((struct Expr *)x->u.unary);
	    break;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case MOD:
	    freeExpr((struct Expr *)x->u.binary.left);
	    freeExpr((struct Expr *)x->u.binary.right);
	default:;
    }
    free(x);
}

static void
eval(const struct Expr *x, const struct UStr *guard)
{
    // access the mutable field 'r' of expression
    struct Mutable *m = (struct Mutable *)&x->m;

    if (x->m.type == ABS) {
	return;
    }
    switch (x->kind) {
	case VAL:
	    m->val = x->u.val;
	    break;
	case SYM: {
	    symtabGet(x->u.sym, guard, &m->type, &m->val);
	    if (m->type == REL_TEXT) {
		m->val += cgSegStartAddr(CGSEG_TEXT);
	    } else if (m->type == REL_DATA) {
		m->val += cgSegStartAddr(CGSEG_DATA);
	    } else if (m->type == REL_BSS) {
		m->val += cgSegStartAddr(CGSEG_BSS);
	    }
	} break;
	case NEG:
	    eval(x->u.unary, guard);
	    m->type = typeUnary(x);
	    m->val = -x->u.unary->m.val;
	    break;
	case W0:
	case W1:
	case W2:
	case W3: {
	    eval(x->u.unary, guard);
	    m->type = typeUnary(x);
	    size_t w = x->kind - '0';
	    m->val = x->u.unary->m.val >> 16 * w & 0xFFFF;
	} break;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case MOD: {
	    eval(x->u.binary.left, guard);
	    eval(x->u.binary.right, guard);
	    m->type = typeBinary(x);
	    uint64_t valLeft = x->u.binary.left->m.val;
	    uint64_t valRight = x->u.binary.right->m.val;
	    switch (x->kind) {
		case ADD:
		    m->val = valLeft + valRight;
		    break;
		case SUB:
		    m->val = valLeft - valRight;
		    break;
		case MUL:
		    m->val = valLeft * valRight;
		    break;
		case DIV:
		    m->val = valLeft / valRight;
		    break;
		case MOD:
		    m->val = valLeft % valRight;
		    break;
		default:
		    assert(0);
	    }
	} break;
	default:
	    fprintf(stderr,
		    "internal error in 'eval': x->kind = %d ('%c'), type = %d "
		    "('%c')\n",
		    x->kind, x->kind, x->m.type, x->m.type);
	    assert(0);
    }
}

uint64_t
evalExpr(const struct Expr *x)
{
    if (!x) {
	return 0;
    }
    eval(x, 0);
    return x->m.val;
}

uint64_t
evalExprWithRecGuard(const struct Expr *x, const struct UStr *guard)
{
    if (!x) {
	return 0;
    }
    eval(x, guard);
    return x->m.val;
}

enum ExprType
typeExpr(const struct Expr *x)
{
    return x->m.type;
}

struct Loc
locExpr(const struct Expr *x)
{
    return x->loc;
}

bool
symInExpr(const struct UStr *sym, const struct Expr *x)
{
    if (x->kind == SYM) {
	return sym == x->u.sym;
    } else if (x->kind == NEG) {
	return symInExpr(sym, x->u.unary);
    } else if (x->kind > NEG) {
	return symInExpr(sym, x->u.binary.left) ||
	       symInExpr(sym, x->u.binary.right);
    }
    return false;
}

static void
printTreeExpr_(size_t level, const struct Expr *x)
{
    for (size_t i = 0; i < level; ++i) {
	printf("    ");
    }
    switch (x->kind) {
	case VAL:
	    printf("%3" PRId64, x->u.val);
	    break;
	case SYM:
	    printf("'%s'", x->u.sym->cstr);
	    break;
	case NEG:
	    printf("- (unary)");
	    break;
	case ADD:
	    printf("+");
	    break;
	case SUB:
	    printf("-");
	    break;
	case MUL:
	    printf("*");
	    break;
	case DIV:
	    printf("/");
	    break;
	case MOD:
	    printf("%%");
	    break;
	case W0:
	    printf("@w0");
	    break;
	case W1:
	    printf("@w1");
	    break;
	case W2:
	    printf("@w2");
	    break;
	case W3:
	    printf("@w3");
	    break;
	    break;
	default:
	    fprintf(stderr, "in 'printTreeExpr_': internal error");
	    abort();
    }
    printf(" [%c]\n", x->m.type);

    if (x->kind == NEG || (x->kind >= W0 && x->kind <= W3)) {
	printTreeExpr_(level + 1, x->u.unary);
    } else {
	switch (x->kind) {
	    case ADD:
	    case SUB:
	    case MUL:
	    case DIV:
	    case MOD:
		printTreeExpr_(level + 1, x->u.binary.left);
		printTreeExpr_(level + 1, x->u.binary.right);
		break;
	    default:;
	}
    }
}

void
printTreeExpr(const struct Expr *x)
{
    if (!x) {
	printf("<null>");
    } else {
	printTreeExpr_(1, x);
	printf("val: '%" PRId64 "', type: '%c'\n", x->m.val, x->m.type);
    }
}

static const char *
strExprKind(enum ExprKind kind)
{
    switch (kind) {
	case VAL:
	    return "VAL";
	case SYM:
	    return "SYM";
	case NEG:
	case SUB:
	    return "-";
	case ADD:
	    return "+";
	case MUL:
	    return "*";
	case DIV:
	    return "/";
	case MOD:
	    return "%";
	case W0:
	    return "@w0";
	case W1:
	    return "@w1";
	case W2:
	    return "@w2";
	case W3:
	    return "@w3";
    }
    fprintf(stderr, "internal error in 'strExprKind'\n");
    abort();
    return 0;
}

static void
strExpr_(const struct Expr *x, bool isFactor, struct Str *str)
{
    if (x->kind == VAL) {
	char s[30];
	snprintf(s, 30, "%" PRIu64, x->m.val);
	appendCStrToStr(str, s);
    } else if (x->kind == SYM) {
	appendCStrToStr(str, x->u.sym->cstr);
    } else if (x->kind == NEG) {
	appendCharToStr(str, '-');
	strExpr_(x->u.unary, true, str);
    } else if (x->kind >= W0 && x->kind <= W3) {
	appendCStrToStr(str, strExprKind(x->kind));
	appendCharToStr(str, '(');
	strExpr_(x->u.unary, false, str);
	appendCharToStr(str, ')');
    } else {
	switch (x->kind) {
	    case ADD:
	    case SUB:
		if (isFactor) {
		    appendCStrToStr(str, "(");
		}
		strExpr_(x->u.binary.left, false, str);
		appendCStrToStr(str, strExprKind(x->kind));
		strExpr_(x->u.binary.right, false, str);
		if (isFactor) {
		    appendCStrToStr(str, ")");
		}
		break;
	    default:
		strExpr_(x->u.binary.left, false, str);
		appendCStrToStr(str, strExprKind(x->kind));
		strExpr_(x->u.binary.right, false, str);
	}
    }
}

const char *
strExpr(const struct Expr *x)
{
    static struct Str str;

    clearStr(&str);
    strExpr_(x, false, &str);
    return str.cstr;
}

const char *
relocStr_(const struct Expr *x, bool clear)
{
    static struct Str str;

    if (clear) {
	clearStr(&str);
    }
    evalExpr(x);

    if (x->kind >= W0 && x->kind <= W3) {
	appendCStrToStr(&str, strExprKind(x->kind));
	appendCharToStr(&str, '(');
	relocStr_(x->u.unary, false);
	appendCharToStr(&str, ')');
	return str.cstr;
    }

    if (x->m.type != UNKNOWN) {
	uint64_t val = x->m.val;
	switch (x->m.type) {
	    case REL_TEXT:
		val -= cgSegStartAddr(CGSEG_TEXT);
		appendCStrToStr(&str, "[text]");
		break;
	    case REL_DATA:
		val -= cgSegStartAddr(CGSEG_DATA);
		appendCStrToStr(&str, "[data]");
		break;
	    case REL_BSS:
		val -= cgSegStartAddr(CGSEG_BSS);
		appendCStrToStr(&str, "[bss]");
		break;
	    default:;
	}
	char s[30];
	if (x->m.type == ABS) {
	    snprintf(s, 30, "%" PRId64, val);
	} else {
	    snprintf(s, 30, "%+" PRId64, val);
	}
	appendCStrToStr(&str, s);
	return str.cstr;
    } else if (x->kind == SYM) {
	appendCStrToStr(&str, x->u.sym->cstr);
	return str.cstr;
    } else if (x->kind == ADD || x->kind == SUB) {
	const struct Expr *left = x->u.binary.left;
	const struct Expr *right = x->u.binary.right;
	if (left->m.type == UNKNOWN && right->m.type == UNKNOWN) {
	    return 0;
	}
	if (left->m.type == UNKNOWN && left->kind != SYM) {
	    return 0;
	}
	if (right->m.type == UNKNOWN && (right->kind != SYM || x->kind == SUB))
	{
	    return 0;
	}
	if (right->m.type == UNKNOWN) {
	    const struct Expr *tmp = left;
	    left = right;
	    right = tmp;
	}
	relocStr_(left, false);
	appendCStrToStr(&str, x->kind == ADD ? " + " : " - ");
	relocStr_(right, false);
	return str.cstr;
    }

    return 0;
}

const char *
relocStrExpr(const struct Expr *x)
{
    return relocStr_(x, true);
}

static void
printExpr_(const struct Expr *x, bool isFactor)
{
    if (x->kind == VAL) {
	printf("%" PRId64, x->m.val);
    } else if (x->kind == SYM) {
	printf("%s", x->u.sym->cstr);
    } else if (x->kind == NEG) {
	printf("%s", strExprKind(x->kind));
	printExpr_(x->u.unary, true);
    } else if (x->kind >= W0 && x->kind <= W3) {
	printf("%s(", strExprKind(x->kind));
	printExpr_(x->u.unary, false);
	printf(")");
    } else {
	switch (x->kind) {
	    case ADD:
	    case SUB:
		if (isFactor) {
		    printf("(");
		}
		printExpr_(x->u.binary.left, false);
		printf(" %s ", strExprKind(x->kind));
		printExpr_(x->u.binary.right, false);
		if (isFactor) {
		    printf(")");
		}
		break;
	    default:
		printExpr_(x->u.binary.left, true);
		printf(" %s ", strExprKind(x->kind));
		printExpr_(x->u.binary.right, true);
	}
    }
}

void
printExpr(const struct Expr *x)
{
    if (!x) {
	printf("<null>");
    } else {
	printExpr_(x, false);
    }
}

#include <assert.h>

#include "cg.h"
#include "comment.h"
#include "error.h"
#include "expected.h"
#include "parsedirective.h"
#include "parseexpr.h"
#include "symtab.h"
#include "warning.h"

bool
parseDirective(void)
{
    if (token.kind < DIRECTIVE_FIRST || token.kind >= DIRECTIVE_END) {
	return false;
    }
    commentClear();
    commentAddCurrentToken();
    enum TokenKind directive = token.kind;
    getToken();
    switch (directive) {
	case DOT_TEXT:
	    commentClear();
	    cgSetActiveSegment(CGSEG_TEXT);
	    break;
	case DOT_DATA:
	    commentClear();
	    cgSetActiveSegment(CGSEG_DATA);
	    break;
	case DOT_BSS:
	    commentClear();
	    cgSetActiveSegment(CGSEG_BSS);
	    break;
	case DOT_STRING:
	    expected(STRING_LITERAL);
	    cgAppendString(token.processedVal.cstr);
	    commentAddCStr("\"");
	    commentAddCStr(token.val.cstr);
	    commentAddCStr("\"");
	    getToken();
	    break;
	case DOT_BYTE:
	case DOT_WORD:
	case DOT_LONG:
	case DOT_QUAD: {
	    size_t numBytes = 0;
	    switch (directive) {
		case DOT_BYTE:
		    numBytes = 1;
		    break;
		case DOT_WORD:
		    numBytes = 2;
		    break;
		case DOT_LONG:
		    numBytes = 4;
		    break;
		case DOT_QUAD:
		    numBytes = 8;
		    break;
		default:
		    assert(0);
	    }
	    struct Expr *expr = parseExpression();
	    /*
	    if (!expr) {
		error("expression expected. Got '%s'\n",
		      strTokenKind(token.kind));
	    }
	    */
	    cgAppendInteger(numBytes, expr);
	    commentAddExpr(expr);
	} break;
	case DOT_ALIGN: {
	    commentClear();
	    struct Expr *expr = parseExpression();
	    if (expr) {
		uint64_t val = evalExpr(expr);
		if (typeExpr(expr) == UNKNOWN) {
		    errorAt(locExpr(expr), "can not resolve expr");
		}
		cgAlign(val);
	    } else {
		warning("alignment directive with no operand is ignored\n");
	    }
	} break;
	case DOT_SPACE: {
	    struct Expr *expr = parseExpression();
	    if (!expr) {
		error("expression expected. Got '%s'\n",
		      strTokenKind(token.kind));
	    }
	    uint64_t val = evalExpr(expr);
	    if (typeExpr(expr) == UNKNOWN) {
		errorAt(locExpr(expr), "can not resolve expr");
	    }
	    cgAppendSpace(val);
	} break;
	case DOT_GLOBL:
	case DOT_GLOBAL: {
	    commentClear();
	    expected(IDENT);
	    const struct UStr *ident = makeUStr(token.val.cstr);
	    getToken();
	    symtabGlobal(ident);
	} break;
	case DOT_EQU: {
	    commentClear();
	    expected(IDENT);
	    const struct UStr *ident = makeUStr(token.val.cstr);
	    getToken();
	    expected(COMMA);
	    getToken();
	    struct Expr *expr = parseExpression();
	    if (!expr) {
		error("expression expected. Got '%s'\n",
		      strTokenKind(token.kind));
	    }
	    symtabSet(ident, expr);
	} break;
	default:
	    assert(0);
	    return false; // never reached
    }
    cgAppendComment(commentGetCStr());
    expected(EOL);
    getToken();
    return true;
}

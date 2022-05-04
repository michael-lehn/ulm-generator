#include <inttypes.h>

#include "comment.h"
#include <utils/str.h>

static struct Str comment;

void
commentClear(void)
{
    clearStr(&comment);
}

void
commentAddExpr(const struct Expr *expr)
{
    if (expr) {
	appendCStrToStr(&comment, strExpr(expr));
    }
}

void
commentAddCStr(const char *str)
{
    appendCStrToStr(&comment, str);
}

void
commentAddLiteral(uint64_t val)
{
    char s[30];
    snprintf(s, 30, "%" PRIu64, val);
    appendCStrToStr(&comment, s);
}

void
commentAddCurrentToken(void)
{
    if (token.kind == EOL) {
	return;
    }

    appendStrToStr(&comment, &token.val);

    if ((MNEMONIC_FIRST != MNEMONIC_END && token.kind >= MNEMONIC_FIRST &&
	 token.kind < MNEMONIC_END) ||
	(token.kind >= DIRECTIVE_FIRST && token.kind < DIRECTIVE_END) ||
	token.kind == COMMA)
    {
	appendCharToStr(&comment, ' ');
    }
}

const char *
commentGetCStr(void)
{
    return comment.cstr;
}


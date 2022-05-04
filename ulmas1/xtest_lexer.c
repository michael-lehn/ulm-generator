#include <stdio.h>

#include "lexer.h"

int
main()
{
    setLexerIn(stdin, 0);
    while (getToken() != EOI) {
	printf("%zu.%zu-%zu.%zu: %s '%s' '%s'\n",
	       token.loc.begin.line, token.loc.begin.col,
	       token.loc.end.line, token.loc.end.col,
	       strTokenKind(token.kind), token.val.cstr,
	       token.processedVal.cstr);
    }
}

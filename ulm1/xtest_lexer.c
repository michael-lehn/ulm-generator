#include <stdio.h>

#include "lexer.h"

int
main()
{
    setLexerIn(stdin, "<stdin>");

    const char *comment;
    for (enum TokenKind kind; (kind = getToken()) != EOI;) {
	if (*(comment = getPendingComment())) {
	    printf("comment:\n%s", comment);
	}
	
	printf("%s:%zu.%zu-%zu.%zu: %s %s\n", token.loc.filename,
	       token.loc.begin.line, token.loc.begin.col, token.loc.end.line,
	       token.loc.end.col, strTokenKind(token.kind), token.val.cstr);
    }
    printf("%s:%zu.%zu-%zu.%zu: %s %s\n", token.loc.filename,
	   token.loc.begin.line, token.loc.begin.col, token.loc.end.line,
	   token.loc.end.col, strTokenKind(token.kind), token.val.cstr);
}

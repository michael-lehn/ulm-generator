#include <stdio.h>
#include <stdlib.h>

#include "expected.h"
#include "lexer.h"

void
expected(enum TokenKind kind)
{
    if (token.kind != kind) {
	fprintfLoc(stderr, &token.loc, "expected '%s' got '%s'\n",
		   valTokenKind(kind), valTokenKind(token.kind));
	exit(1);
    }
}


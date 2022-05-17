#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "cg.h"
#include "comment.h"
#include "error.h"
#include "expected.h"
#include "encode.h"
#include "lexer.h"
#include "parsedirective.h"
#include "parseexpr.h"
#include "parseinstruction.h"
#include "symtab.h"
#include <utils/printcode.h>

int
main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
	fprintf(stderr, "usage: %s input [output]\n", argv[0]);
	return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
	fprintf(stderr, "can not open input file '%s'\n", argv[1]);
	return 1;
    }
    const char *output = argc == 2 ? "a.out" : argv[2];
    FILE *out = fopen("a.out", "w");
    if (!out) {
	fprintf(stderr, "can not open output file '%s'\n", output);
	return 1;
    }

    setLexerIn(in, argv[1]);

    getToken();
    while (token.kind != EOI) {
	if (token.kind == UNKNOWN_TOKEN) {
	    error("bad token\n");
	}

	// skip empty lines
	if (token.kind == EOL) {
	    getToken();
	    continue;
	}

	expectedOneOf(2, EMPTY_LABEL, IDENT);
	if (token.kind == IDENT) {
	    const struct UStr *label = makeUStr(token.val.cstr);
	    cgSetLabel(token.loc, label);
	    getToken();
	    if (token.kind == COLON) {
		getToken();
	    }
	} else {
	    getToken();
	}

	if (token.kind == UNKNOWN_TOKEN) {
	    error("bad token\n");
	}

	if (!parseInstruction() && !parseDirective()) {
	    expected(EOL);
	}
    }
    cgPrint(out);
    symtabPrint(out);
    encodePrintFixups(out);

    fclose(in);
    fclose(out);
}

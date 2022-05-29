#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cg.h"
#include "comment.h"
#include "encode.h"
#include "error.h"
#include "expected.h"
#include "lexer.h"
#include "parsedirective.h"
#include "parseexpr.h"
#include "parseinstruction.h"
#include "symtab.h"
#include <utils/printcode.h>

void
usage(const char *prg)
{
    fprintf(stderr, "usage: %s [-o output] input\n", prg);
    exit(1);
}

int
main(int argc, char **argv)
{
    if (argc != 2 && argc != 4) {
	usage(argv[0]);
    }

    const char *input = 0, *output = "a.out";

    for (int i = 1; i < argc; ++i) {
	if (!strcmp(argv[i], "-o")) {
	    output = argv[++i];
	    continue;
	}
	if (!strcmp(argv[i], "--")) {
	    continue;
	} else {
	    input = argv[i];
	    continue;
	}
	if (*argv[i] == '-') {
	    usage(argv[0]);
	}
    }

    FILE *in = input ? fopen(input, "r") : stdin;
    FILE *out = fopen(output, "w");

    if (!in) {
	fprintf(stderr, "can not open input file '%s'\n", argv[1]);
	return 1;
    }

    if (!out) {
	fprintf(stderr, "can not open output file '%s'\n", output);
	return 1;
    }

    setLexerIn(in, input);

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

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
parse(void)
{
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
}

void
usage(const char *prg)
{
    fprintf(stderr, "usage: %s [-o output] input ...\n", prg);
    exit(1);
}

int
main(int argc, char **argv)
{
    const char *prg = argv[0];
    --argc;
    ++argv;

    const char *output = "a.out";

    if (argc == 0) {
	usage(prg);
    } else if (!strcmp(argv[0], "-o")) {
	if (argc < 3) {
	    usage(prg);
	}
	output = argv[1];
	argc -= 2;
	argv += 2;
    }

    FILE *out = fopen(output, "w");
    if (!out) {
	fprintf(stderr, "can not open output file '%s'\n", output);
	return 1;
    }

    while (argc) {
	FILE *in = fopen(argv[0], "r");
	if (!in) {
	    fprintf(stderr, "can not open input file '%s'\n", argv[1]);
	    return 1;
	}
	setLexerIn(in, argv[0]);
	--argc;
	++argv;
	parse();
	fclose(in);
    }
    cgPrint(out);
    symtabPrint(out);
    encodePrintFixups(out);
    fclose(out);
}

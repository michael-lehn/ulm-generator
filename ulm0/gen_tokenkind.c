#include <stdbool.h>
#include <stdio.h>

#include <utils/printcode.h>
#include <utils/ustr.h>

static bool
isWhitespace(int ch)
{
    return ch == ' ' || ch == '\r' || ch == '\f' || ch == '\v' || ch == '\t' ||
	   ch == '\n';
}

void
split(char *line, const char **token, const char **descr)
{
    *token = line;
    *descr = 0;

    for (; *line && !isWhitespace(*line); ++line)
	;

    if (isWhitespace(*line)) {
	// terminate token
	*line++ = 0;

	// skip to '"'
	for (; *line && *line != '"'; ++line)
	    ;
	if (*line == '"') {
	    *descr = ++line;
	}
	// skip to '"'
	for (; *line && *line != '"'; ++line)
	    ;
	if (*line == '"') {
	    *line = 0;
	}
    }
}

enum HeaderFooter
{
    HEADER,
    FOOTER,
};

void
printTokenKind(FILE *out, enum HeaderFooter t)
{
    if (t == HEADER) {
	printCode(out, 0, "enum TokenKind {\n");
    } else {
	printCode(out, 1, "NUM_TOKENKIND,\n");
	printCode(out, 0, "};\n");
    }
}

void
printStrTokenKind(FILE *out, enum HeaderFooter t)
{
    if (t == HEADER) {
	printCode(out, 0, "const char *\n");
	printCode(out, 0, "strTokenKind(enum TokenKind kind)\n");
	printCode(out, 0, "{\n");
	printCode(out, 1, "switch (kind) {\n");
    } else {
	printCode(out, 2, "case NUM_TOKENKIND: /* cause internal error */;\n");
	printCode(out, 3, ";\n");
	printCode(out, 1, "}\n");
	printCode(
	  out, 1,
	  "fprintf(stderr, \"in 'strTokenKind': internal error\\n\");\n");
	printCode(out, 1, "abort();\n");
	printCode(out, 1, "return 0;\n");
	printCode(out, 0, "}\n");
    }
}

void
printValTokenKind(FILE *out, enum HeaderFooter t)
{
    if (t == HEADER) {
	printCode(out, 0, "const char *\n");
	printCode(out, 0, "valTokenKind(enum TokenKind kind)\n");
	printCode(out, 0, "{\n");
	printCode(out, 1, "switch (kind) {\n");
    } else {
	printCode(out, 2, "case NUM_TOKENKIND: /* cause internal error */;\n");
	printCode(out, 3, ";\n");
	printCode(out, 1, "}\n");
	printCode(
	  out, 1,
	  "fprintf(stderr, \"in 'valTokenKind': internal error\\n\");\n");
	printCode(out, 1, "abort();\n");
	printCode(out, 1, "return 0;\n");
	printCode(out, 0, "}\n");
    }
}

int
main(int argc, char **argv)
{
    if (argc != 5) {
	fprintf(stderr,
		"Example usage: %s token.txt tokenkind.h strtokenkind.c "
		"valtokenkind.c\n",
		argv[0]);
	return 1;
    }
    FILE *in = fopen(argv[1], "r");
    FILE *tk = fopen(argv[2], "w");
    FILE *stk = fopen(argv[3], "w");
    FILE *vtk = fopen(argv[4], "w");

    if (!in) {
	fprintf(stderr, "can not open input file '%s'.\n", argv[1]);
	return 1;
    }
    if (!tk) {
	fprintf(stderr, "can not open output file '%s'.\n", argv[2]);
	return 1;
    }
    if (!stk) {
	fprintf(stderr, "can not open output file '%s'.\n", argv[3]);
	return 1;
    }
    if (!vtk) {
	fprintf(stderr, "can not open output file '%s'.\n", argv[4]);
	return 1;
    }

    printTokenKind(tk, HEADER);
    printStrTokenKind(stk, HEADER);
    printValTokenKind(vtk, HEADER);

    char *line = 0;
    size_t linecap = 0;
    for (ssize_t n; (n = getline(&line, &linecap, in)) > 0;) {
	const char *token, *descr;
	split(line, &token, &descr);

	bool added;
	struct UStr *ustr = makeUStr_(token, &added);
	if (!added) {
	    fprintf(stderr, "%s failed: token '%s' occurs more than once.\n",
		    argv[0], ustr->cstr);
	    return 1;
	}
	// >> tokenkind.h
	//
	//     TOKEN,
	//
	printCode(tk, 1, "%s,\n", ustr->cstr);

	// >> strtokenkind.c
	//
	// case TOKEN:
	//     return "TOKEN";
	//
	printCode(stk, 2, "case %s:\n", ustr->cstr);
	printCode(stk, 3, "return \"%s\";\n", ustr->cstr);

	// >> valtokenkind.c
	//
	// case TOKEN:
	//     return "description";
	printCode(vtk, 2, "case %s:\n", ustr->cstr);
	printCode(vtk, 3, "return \"%s\";\n", descr ? descr : ustr->cstr);
    }

    printTokenKind(tk, FOOTER);
    printStrTokenKind(stk, FOOTER);
    printValTokenKind(vtk, FOOTER);

    fclose(in);
    fclose(tk);
    fclose(stk);
}

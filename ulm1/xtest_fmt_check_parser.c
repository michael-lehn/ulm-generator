#include <stdio.h>

#include "lexer.h"
#include "fmt_check_parser.h"
#include "fmt.h"
#include "testcase.h"

int
main(int argc, char **argv)
{
    if (argc != 3) {
	fprintf(stderr, "usage: %s fmt_check.txt check.c\n", argv[0]);
	return 1;
    }

    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    if (!in) {
	fprintf(stderr, "can not open input file '%s'\n", argv[1]);
	return 1;
    }
    if (!out) {
	fprintf(stderr, "can not open output file '%s'\n", argv[2]);
	return 1;
    }

    setLexerIn(in, argv[1]);

    parseFmtCheck();
    printTestCaseList(out);
}

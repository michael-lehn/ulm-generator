#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "testcase.h"
#include <utils/printcode.h>

struct CaseNode
{
    struct TestCase
    {
	struct CodeNode
	{
	    const char *line;
	    struct CodeNode *next;
	} * codeNode[TC_NUM_CODE_BLOCK], *codeNodeLast[TC_NUM_CODE_BLOCK];
    } testCase;
    struct CaseNode *next;
} * caseNode, *caseNodeLast;

struct TestCase *
addTestCase()
{
    struct CaseNode *cn = malloc(sizeof(*caseNode));
    if (!cn) {
	fprintf(stderr, "addTestCase: out of memory\n");
	exit(1);
    }

    cn->testCase.codeNode[TC_INIT] = 0;
    cn->testCase.codeNode[TC_TEST] = 0;
    cn->testCase.codeNode[TC_CHECK] = 0;
    cn->next = 0;

    if (caseNode) {
	caseNodeLast = caseNodeLast->next = cn;
    } else {
	caseNode = caseNodeLast = cn;
    }
    return &cn->testCase;
}

void
appendTestCaseCode(struct TestCase *testCase, enum TestCaseCodeBlock codeBlock,
		   const struct Str *code)
{
    assert(codeBlock >= 0 && codeBlock < TC_NUM_CODE_BLOCK);
    assert(testCase);

    char *line = strndup(code->cstr, code->end - code->cstr);
    struct CodeNode *cn = malloc(sizeof(*cn));
    if (!cn || !line) {
	fprintf(stderr, "addTestCase: out of memory\n");
	exit(1);
    }
    cn->line = line;
    cn->next = 0;

    if (testCase->codeNode[codeBlock]) {
	testCase->codeNodeLast[codeBlock] =
	  testCase->codeNodeLast[codeBlock]->next = cn;
    } else {
	testCase->codeNode[codeBlock] = testCase->codeNodeLast[codeBlock] = cn;
    }
}

void
printTestCaseList(FILE *out)
{
    size_t count = 1;
    for (const struct CaseNode *n = caseNode; n; n = n->next, ++count) {
	printCode(out, 0, "bool\n");
	printCode(out, 0, "check%04zu()\n", count);
	printCode(out, 0, "{\n");

	struct TestCase tc = n->testCase;
	for (struct CodeNode *cn = tc.codeNode[TC_INIT]; cn; cn = cn->next) {
	    printCode(out, 0, "%s\n", cn->line);
	}
	printCode(out, 0, "\n");

	for (struct CodeNode *cn = tc.codeNode[TC_TEST]; cn; cn = cn->next) {
	    printCode(out, 1, "ulm_load_str(\"%s\");\n", cn->line);
	}
	printCode(out, 0, "\n");
	printCode(out, 1, "run();\n");
	printCode(out, 0, "\n");

	for (struct CodeNode *cn = tc.codeNode[TC_CHECK]; cn; cn = cn->next) {
	    printCode(out, 0, "%s\n", cn->line);
	}
	printCode(out, 0, "}\n");
    }
    printCode(out, 0, "\n");

    printCode(out, 0, "int\n");
    printCode(out, 0, "main(int argc, char **argv)\n", count);
    printCode(out, 0, "{\n");
    count = 1;
    for (const struct CaseNode *n = caseNode; n; n = n->next, ++count) {
	printCode(out, 1, "if (! check%04zu()) {\n", count);
	printCode(out, 2,
		  "fprintf(stderr, \"%%s: case %04zu failed\\n\", argv[0]);\n",
		  count);
	printCode(out, 2, "exit(1);\n");
	printCode(out, 1, "}\n");
    }
    printCode(
      out, 1,
      "fprintf(stderr, \"%%s: *** all tests passed ***\\n\", argv[0]);\n");
    printCode(out, 0, "}\n");
}


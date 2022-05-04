#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "expected.h"
#include "fmt.h"
#include "fmt_check_parser.h"
#include "instr.h"
#include "lexer.h"
#include "testcase.h"
#include <utils/ustr.h>

enum TestCaseKeyword
{
    KW_CASE,
    KW_TEST,
    KW_CHECK,
    KW_ESAC,
    KW_NUM_KEYWORDS,
};

static void parseBlock(enum TestCaseKeyword kw);
static void parseCode(enum TestCaseKeyword kw);

static struct UStr *kwIdent[KW_NUM_KEYWORDS];
static struct TestCase *testCase;

void
parseFmtCheck()
{
    kwIdent[KW_CASE] = makeUStr("case");
    kwIdent[KW_TEST] = makeUStr("test");
    kwIdent[KW_CHECK] = makeUStr("check");
    kwIdent[KW_ESAC] = makeUStr("esac");

    for (getToken(); token.kind != EOI;) {
	if (token.kind == EOL) {
	    getToken();
	    continue;
	}

	parseBlock(KW_CASE);
	parseBlock(KW_TEST);
	parseBlock(KW_CHECK);
	parseBlock(KW_ESAC);
    }
}

static void
parseBlock(enum TestCaseKeyword kw)
{
    expected(IDENT);
    struct UStr *ident = makeUStr(token.val.cstr);
    getToken();
    expected(EOL);
    getToken();

    if (ident != kwIdent[kw]) {
	error("expected '%s' got '%s'\n", kwIdent[kw]->cstr, ident->cstr);
    }

    if (kw == KW_CASE) {
	testCase = addTestCase();
    }
    if (kw != KW_ESAC) {
	parseCode(kw);
    }
}

static void
parseCode(enum TestCaseKeyword kw)
{
    enum TestCaseCodeBlock codeBlock;

    switch (kw) {
	case KW_CASE:
	    codeBlock = TC_INIT;
	    break;
	case KW_TEST:
	    codeBlock = TC_TEST;
	    break;
	case KW_CHECK:
	    codeBlock = TC_CHECK;
	    break;
	default:
	    fprintf(stderr, "internal error in 'parseCode'\n");
	    exit(1);
    }

    while (token.kind == CODE || token.kind == EOL) {
	appendTestCaseCode(testCase, codeBlock, &token.val);
	getToken();
    }
}

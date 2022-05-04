#ifndef ULM1_TESTCASE_H
#define ULM1_TESTCASE_H

#include <stdint.h>
#include <stdio.h>

#include <utils/str.h>

struct TestCase;

enum TestCaseCodeBlock
{
    TC_INIT,
    TC_TEST,
    TC_CHECK,
    TC_NUM_CODE_BLOCK,
};

struct TestCase *addTestCase();
void appendTestCaseCode(struct TestCase *testCase,
			enum TestCaseCodeBlock codeBlock,
			const struct Str *code);
void printTestCaseList(FILE *out);

#endif // ULM1_TESTCASE_H

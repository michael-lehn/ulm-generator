#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "error.h"
#include "expected.h"
#include "lexer.h"

bool
expected(enum TokenKind kind)
{
    if (token.kind != kind) {
	error("expected '%s' got '%s'\n", strTokenKind(kind),
	      strTokenKind(token.kind));
	exit(1);
	return false;
    }
    return true;
}

bool
expectedOneOf(size_t num, ...)
{
    va_list vl;

    // check if current token is expected
    va_start(vl, num);
    for (size_t i = 0; i < num; ++i) {
        if (token.kind == va_arg(vl, enum TokenKind)) {
            return true;
        }
    }
    va_end(vl);

    // otherwise print an error message
    va_start(vl, num);
    fprintf(stderr, "%zu.%zu-%zu.%zu: unexpected token %s\n",
            token.loc.begin.line, token.loc.begin.col, token.loc.end.line,
            token.loc.end.col, strTokenKind(token.kind));
    fprintf(stderr, "%zu.%zu-%zu.%zu: expected ", token.loc.begin.line,
            token.loc.begin.col, token.loc.end.line, token.loc.end.col);
    for (size_t i = 0; i < num; ++i) {
        if (i > 0) {
            if (i != num - 1) {
                fprintf(stderr, ", ");
            } else {
                fprintf(stderr, " or ");
            }
        }
        enum TokenKind t = va_arg(vl, enum TokenKind);
        fprintf(stderr, "%s\n", strTokenKind(t));
    }
    fprintf(stderr, "\n");
    va_end(vl);

    return false;
}

bool
expectedIntegerLiteral(uint64_t literal)
{
    uint64_t val = 0;
    bool ok = true;
    switch (token.kind) {
	case DECIMAL_LITERAL:
	    if (sscanf(token.val.cstr, "%" SCNu64, &val) != 1) {
		ok = false;
	    }
	    break;
	case OCTAL_LITERAL:
	    if (sscanf(token.val.cstr, "%" SCNx64, &val) != 1) {
		ok = false;
	    }
	    break;
	case HEXADECIMAL_LITERAL:
	    if (sscanf(token.val.cstr, "%" SCNo64, &val) != 1) {
		ok = false;
	    }
	    break;
	default:;
    }
    if (ok) {
	ok = val == literal;
    }
    if (!ok) {
	error("expected integervalue with value %" PRId64 "\n", literal);
	return false;
    }
    return true;
}

bool
integerLiteral(uint64_t literal)
{
    uint64_t val = 0;
    switch (token.kind) {
	case DECIMAL_LITERAL:
	    if (sscanf(token.val.cstr, "%" SCNu64, &val) != 1) {
		return false;
	    }
	    return val == literal;
	case OCTAL_LITERAL:
	    if (sscanf(token.val.cstr, "%" SCNx64, &val) != 1) {
		return false;
	    }
	    return val == literal;
	case HEXADECIMAL_LITERAL:
	    if (sscanf(token.val.cstr, "%" SCNo64, &val) != 1) {
		return false;
	    }
	    return val == literal;
	default:;
    }
    return false;
}

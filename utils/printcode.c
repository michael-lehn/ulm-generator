#include <stdarg.h>
#include <string.h>

#include "printcode.h"
#include "str.h"

static void
printIndent(FILE *out, size_t indent)
{
    for (size_t i = 0; i < indent; ++i) {
	fprintf(out, " ");
    }
}

void
printCode(FILE *out, size_t indentLevel, const char *fmt, ...)
{
    printIndent(out, indentLevel * 4);
    if (!fmt) {
	return;
    }

    va_list argp;
    va_start(argp, fmt);
    vfprintf(out, fmt, argp);
    va_end(argp);
}

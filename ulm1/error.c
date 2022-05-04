#include <stdarg.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"

void
error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vfprintfLoc(stderr, &token.loc, fmt, argp);
    va_end(argp);
    exit(1);
}

void
errorAt(struct Loc loc, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vfprintfLoc(stderr, &loc, fmt, argp);
    va_end(argp);
    exit(1);
}

#include <stdarg.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"

#include <utils/color.h>

void
error(const char *fmt, ...)
{
    setColor(BOLD);
    fprintfLoc(stderr, &token.loc, "");
    setColor(SAD);
    setColor(BOLD_RED);
    fprintf(stderr, " error: ");
    setColor(RED);

    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    setColor(NORMAL);
    exit(1);
}

void
errorAt(struct Loc loc, const char *fmt, ...)
{
    setColor(BOLD);
    fprintfLoc(stderr, &loc, "");
    setColor(SAD);
    setColor(BOLD_RED);
    fprintf(stderr, " error: ");
    setColor(RED);

    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    setColor(NORMAL);
    exit(1);
}

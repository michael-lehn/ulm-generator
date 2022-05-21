#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"

#include <utils/color.h>

void
error(const char *fmt, ...)
{
    setColor(RED);
    fprintfLoc(stderr, &token.loc, "");
    setColor(SAD);
    fprintf(stderr, " ");

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
    setColor(RED);
    fprintfLoc(stderr, &token.loc, "");
    setColor(SAD);
    fprintf(stderr, " ");

    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    setColor(NORMAL);
    exit(1);
}

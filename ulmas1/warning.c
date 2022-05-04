#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "warning.h"
#include "lexer.h"

void
warning(const char *fmt, ...)
{
    fprintfLoc(stderr, &token.loc, "Warning: ");

    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
}

void
warningAt(struct Loc loc, const char *fmt, ...)
{
    fprintfLoc(stderr, &loc, "Warning: ");

    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    exit(1);
}


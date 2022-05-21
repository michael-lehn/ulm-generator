#ifndef UTILS_LOC_H
#define UTILS_LOC_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

struct Pos
{
    size_t line, col;
};

struct Loc
{
    struct Pos begin, end;
    const char *filename;
};

extern struct Loc nullLoc;

struct Loc combineLoc(struct Loc loc0, struct Loc loc1);

void fprintfLoc(FILE *out, const struct Loc *loc, const char *fmt, ...);
void vfprintfLoc(FILE *out, const struct Loc *loc, const char *fmt, va_list ap);

#endif // UTILS_LOC_H

#ifndef LOC_H
#define LOC_H

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

#endif // LOC_H
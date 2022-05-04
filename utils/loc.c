#include <stdarg.h>

#include "loc.h"

static size_t
min(size_t a, size_t b)
{
    return a < b ? a : b;
}

static size_t
max(size_t a, size_t b)
{
    return a > b ? a : b;
}

struct Loc nullLoc;

struct Loc
combineLoc(struct Loc loc0, struct Loc loc1)
{
    struct Loc loc = { {
			 min(loc0.begin.line, loc1.begin.line),
			 min(loc0.begin.col, loc1.begin.col),
		       },
		       {
			 max(loc0.begin.line, loc1.begin.line),
			 max(loc0.begin.col, loc1.begin.col),
		       } };
    return loc;
}

void
fprintfLoc(FILE *out, const struct Loc *loc, const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    vfprintfLoc(out, loc, fmt, argp);
    va_end(argp);

    /*
    fprintf(stderr, "%s:%zu.%zu-%zu.%zu: ", loc->filename, loc->begin.line,
	    loc->begin.col, loc->end.line, loc->end.col);

    va_list argp;
    va_start(argp, fmt);
    vfprintf(out, fmt, argp);
    va_end(argp);
    */
}

void
vfprintfLoc(FILE *out, const struct Loc *loc, const char *fmt, va_list ap)
{
    if (loc) {
	const char *f = loc->filename ? loc->filename : "<undefined filename>";
	fprintf(stderr, "%s:%zu.%zu-%zu.%zu: ", f, loc->begin.line,
		loc->begin.col, loc->end.line, loc->end.col);
    }
    vfprintf(out, fmt, ap);
}

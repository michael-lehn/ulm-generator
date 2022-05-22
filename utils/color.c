#include <stdio.h>

#include "color.h"

static const char *colorStr[] = {
    "\033[0m",
    "\033[1;10m",
    "\033[0;31m",
    "\033[0;34m",
    "\033[1;31m",
    "\033[1;34m",
    "\xF0\x9F\x98\xA2",
};

void
setColor(enum Color color)
{
    fprintf(stderr, "%s", colorStr[color]);
}

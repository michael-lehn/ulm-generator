#ifndef ULMAS1_WARNING_H
#define ULMAS1_WARNING_H

#include <utils/loc.h>

void warning(const char *fmt, ...);
void warningAt(struct Loc loc, const char *fmt, ...);

#endif // ULMAS1_WARNING_H


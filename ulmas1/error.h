#ifndef ULMAS1_ERROR_H
#define ULMAS1_ERROR_H

#include <utils/loc.h>

void error(const char *fmt, ...);
void errorAt(struct Loc loc, const char *fmt, ...);

#endif // ULMAS1_ERROR_H


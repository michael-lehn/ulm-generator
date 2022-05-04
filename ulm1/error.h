#ifndef ULM1_ERROR_H
#define ULM1_ERROR_H

#include <utils/loc.h>

void error(const char *fmt, ...);
void errorAt(struct Loc loc, const char *fmt, ...);

#endif // ULM1_ERROR_H


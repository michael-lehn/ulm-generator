#ifndef UTILS_USTR_H
#define UTILS_USTR_H

#include <stdbool.h>
#include <stddef.h>

struct UStr
{
    size_t len;
    char cstr[];
};

struct UStr *makeUStr_(const char *s, bool *added);
struct UStr *makeUStr(const char *s);

#endif // UTILS_USTR_H

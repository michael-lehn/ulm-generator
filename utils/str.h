#ifndef UTILS_STR_H
#define UTILS_STR_H

#include <stddef.h>

struct Str
{
    size_t capacity;
    char *cstr, *end;
};

void makeEmptyStr(struct Str *str);
void destroyStr(struct Str *str); // deallocte string
void clearStr(struct Str *str);	  // set to empty string but keep memory
void appendCharToStr(struct Str *str, char c);
void appendStrToStr(struct Str *str, struct Str *append);
void copyCStrToStr(struct Str *str, const char *cstr);
void appendCStrToStr(struct Str *str, const char *cstr);

#endif // UTILS_STR_H

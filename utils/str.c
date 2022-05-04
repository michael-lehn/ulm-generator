#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"

enum
{
    MIN_CAPACITY = 4, // required to be greater than zero
};

void
makeEmptyStr(struct Str *str)
{
    str->capacity = MIN_CAPACITY;
    str->cstr = str->end = malloc(str->capacity);
    if (!str->cstr) {
	fprintf(stderr, "in appendToStr: out of memory.\n");
	abort();
    }
    *str->cstr = 0;
}

void
destroyStr(struct Str *str)
{
    str->capacity = 0;
    free(str->cstr);
    str->cstr = str->end = 0;
}

void
clearStr(struct Str *str)
{
    if (!str->capacity) {
	makeEmptyStr(str);
    } else {
	str->end = str->cstr;
	*str->cstr = 0;
    }
}

void
appendCharToStr(struct Str *str, char c)
{
    size_t len = str->end - str->cstr; // length without terminating 0

    if (len + 2 > str->capacity) {
	str->capacity = len + 2;
	if (str->capacity < MIN_CAPACITY) {
	    str->capacity = MIN_CAPACITY;
	} else {
	    str->capacity *= 2;
	}
	str->cstr = realloc(str->cstr, str->capacity);
	if (!str->cstr) {
	    fprintf(stderr, "in appendToStr: out of memory.\n");
	    abort();
	}
	str->end = str->cstr + len;
    }
    *str->end++ = c;
    *str->end = 0;
}

void
appendStrToStr(struct Str *str, struct Str *append)
{
    // lengths of 'str' and 'append' without terminating 0
    size_t len0 = str->end - str->cstr;
    size_t len1 = append->end - append->cstr;

    if (len0 + len1 + 1 > str->capacity) {
	str->capacity = len0 + len1 + 1;
	if (str->capacity < MIN_CAPACITY) {
	    str->capacity = MIN_CAPACITY;
	} else {
	    str->capacity *= 2;
	}
	str->cstr = realloc(str->cstr, str->capacity);
	if (!str->cstr) {
	    fprintf(stderr, "in appendToStr: out of memory.\n");
	    abort();
	}
	str->end = str->cstr + len0;
    }
    strcpy(str->end, append->cstr);
    str->end += len1;
}

void
copyCStrToStr(struct Str *str, const char *cstr)
{
    size_t len = strlen(cstr);
    if (len + 1 > str->capacity) {
	str->capacity = len + 1;
	str->cstr = realloc(str->cstr, str->capacity);
	if (!str->cstr) {
	    fprintf(stderr, "in copyToStr: out of memory.\n");
	    abort();
	}
    }
    strcpy(str->cstr, cstr);
    str->end = str->cstr + len;
}

void
appendCStrToStr(struct Str *str, const char *cstr)
{
    // lengths of 'str' and 'append' without terminating 0
    size_t len0 = str->end - str->cstr;
    size_t len1 = strlen(cstr);

    if (len0 + len1 + 1 > str->capacity) {
	str->capacity = len0 + len1 + 1;
	if (str->capacity < MIN_CAPACITY) {
	    str->capacity = MIN_CAPACITY;
	} else {
	    str->capacity *= 2;
	}
	str->cstr = realloc(str->cstr, str->capacity);
	if (!str->cstr) {
	    fprintf(stderr, "in appendToStr: out of memory.\n");
	    abort();
	}
	str->end = str->cstr + len0;
    }
    strcpy(str->end, cstr);
    str->end += len1;
}


#ifndef ULMAS1_LEXER_H
#define ULMAS1_LEXER_H

#include <stdio.h>

#include <utils/loc.h>
#include <utils/str.h>
#include <ulmas0/_gen_tokenkind.h>

struct Token {
    struct Loc loc;
    enum TokenKind kind;
    struct Str val;
    struct Str processedVal;
};

extern struct Token token;

void setLexerIn(FILE *in, const char *inputfile);
enum TokenKind getToken();
const char *strTokenKind(enum TokenKind kind);

#endif // ULMAS1_LEXER_H

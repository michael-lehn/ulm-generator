#ifndef ULM1_LEXER_H
#define ULM1_LEXER_H

#include <stdio.h>

#include <utils/loc.h>
#include <utils/str.h>
#include <ulm0/_gen_tokenkind.h>

struct Token {
    struct Loc loc;
    enum TokenKind kind;
    struct Str val;
};

extern struct Token token;

void setLexerIn(FILE *in, const char *inputfile);
enum TokenKind getToken();
const char *strTokenKind(enum TokenKind kind);
const char *valTokenKind(enum TokenKind kind);
const char *getPendingComment();

// escape '%', '\'
const char *valTokenKindEscaped(enum TokenKind kind);

#endif // ULM1_LEXER_H

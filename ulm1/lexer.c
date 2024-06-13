#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"

// include generated source code
#include <ulm0/_gen_strtokenkind.c>
#include <ulm0/_gen_valtokenkind.c>

static const char *
escape(const char *s)
{
    static struct Str buf;

    clearStr(&buf);
    for (; *s; ++s) {
	switch (*s) {
	    case '%':
	    case '\\':
		appendCharToStr(&buf, *s);
		appendCharToStr(&buf, *s);
		break;
	    default:
		appendCharToStr(&buf, *s);
	}
    }
    return buf.cstr;
}

const char *
valTokenKindEscaped(enum TokenKind kind)
{
    return escape(valTokenKind(kind));
}

static FILE *in;
static struct Pos currPos = { 1, 1 };
static int ch;

static struct Str comment;

struct Token token;

void
setLexerIn(FILE *in_, const char *inputfile)
{
    in = in_;
    token.loc.filename = inputfile;
}

static int
nextCh()
{
    assert(in);
    token.loc.end = currPos;

    do {
	ch = fgetc(in);
    } while (ch == '\r');

    if (ch == '\n') {
	++currPos.line;
	currPos.col = 1;
    } else if (ch == '\t') {
	currPos.col += 8 - currPos.col % 8;
    } else {
	++currPos.col;
    }
    return ch;
}

static bool
isSpace(int ch)
{
    return ch == ' ' || ch == '\r' || ch == '\f' || ch == '\v' || ch == '\t';
}

bool
isLetter(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '.' ||
	   ch == '_';
}

bool
isDigit(char ch)
{
    return ch >= '0' && ch <= '9';
}

bool
isHexdigit(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
	   (ch >= 'A' && ch <= 'F');
}

static enum TokenKind
getToken_()
{
    if (!ch) {
	nextCh();
    }

    if (token.loc.end.col > 1) {
	// skip whitespace
	while (isSpace(ch) || ch == 0) {
	    nextCh();
	}
    }

    // assemble next token
    token.loc.begin = token.loc.end;
    clearStr(&token.val);

    if (token.loc.end.col == 1 && isSpace(ch)) {
	// found code
	do {
	    if (ch == '\t') {
		for (size_t i = 0; i < 8 - token.loc.end.col % 8; ++i) {
		    appendCharToStr(&token.val, ' ');
		}
	    } else {
		appendCharToStr(&token.val, ch);
	    }
	    nextCh();
	} while (ch != '\n');
	nextCh();
	token.kind = CODE;
    } else if (isLetter(ch)) {
	// found identifier
	do {
	    appendCharToStr(&token.val, ch);
	    nextCh();
	} while (isLetter(ch) || isDigit(ch));
	token.kind = IDENT;
    } else if (isDigit(ch)) {
	if (ch == '0') {
	    appendCharToStr(&token.val, ch);
	    nextCh();
	    if (ch == 'x' || ch == 'X') {
		appendCharToStr(&token.val, ch);
		nextCh();
		if (!isHexdigit(ch)) {
		    token.kind = BAD_TOKEN;
		} else {
		    token.kind = HEXADECIMAL_LITERAL;
		    do {
			appendCharToStr(&token.val, ch);
			nextCh();
		    } while (isHexdigit(ch));
		}
	    } else {
		// no leading zeros in decimal representation, so we are done.
		token.kind = DECIMAL_LITERAL;
	    }
	} else {
	    token.kind = DECIMAL_LITERAL;
	    do {
		appendCharToStr(&token.val, ch);
		nextCh();
	    } while (isDigit(ch));
	}
	if (isDigit(ch) || isLetter(ch)) {
	    fprintf(stderr, "%zu.%zu: warning: bad suffix: '%c' (ignored)\n",
		    token.loc.begin.line, token.loc.begin.col, ch);
	}
    } else if (ch == '#' && currPos.col == 2) {
	do {
	    nextCh();
	    switch (ch) {
		case '\n':
		    appendCStrToStr(&comment, "\\n");
		    break;
		default:
		    appendCharToStr(&comment, ch);
	    }
	} while (ch != EOF && ch != '\n');
	nextCh();
	return getToken_();
    } else if (ch == '\\') {
	appendCStrToStr(&token.val, "\\");
	nextCh();
	token.kind = BACKSLASH;
    } else if (ch == '$') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = DOLLAR;
    } else if (ch == ',') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = COMMA;
    } else if (ch == '%') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = PERCENT;
    } else if (ch == '<') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = LANGLE;
    } else if (ch == '>') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = RANGLE;
    } else if (ch == '{') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = LBRACE;
    } else if (ch == '}') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = RBRACE;
    } else if (ch == '#') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = LATTENZAUN;
    } else if (ch == '@') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = KLAMMERAFFE;
    } else if (ch == ':') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = COLON;
    } else if (ch == '\n') {
	nextCh();
	token.kind = EOL;
    } else if (ch == '(') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = LPAREN;
    } else if (ch == ')') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = RPAREN;
    } else if (ch == '[') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = LBRACKET;
    } else if (ch == ']') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = RBRACKET;
    } else if (ch != EOF) {
	// if we get here no legal token was found or end of file was reached
	token.kind = BAD_TOKEN;
	error("bad token '%c'\n", ch);
	nextCh();
    } else {
	token.kind = EOI;
    }
    return token.kind;
}

const char *
getPendingComment()
{
    return comment.cstr;
}

enum TokenKind
getToken()
{
    makeEmptyStr(&comment);
    return getToken_();
}

#include <stdbool.h>
#include <stdlib.h>

#include "lexer.h"

static FILE *in;
static struct Pos currPos = { 1, 1 };
static int ch = 0;

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
    token.loc.end = currPos;

    ch = fgetc(in);

    if (ch == '\n') {
	++currPos.line;
	currPos.col = 1;
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
isOctaldigit(char ch)
{
    return ch >= '0' && ch <= '7';
}

bool
isHexdigit(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
	   (ch >= 'A' && ch <= 'F');
}

// include generated code
#include <ulmas0/_gen_strtokenkind.c>

enum FieldPos
{
    EolField = 0,
    LabelField,
    MnemonicField,
    OpField,
};

static enum FieldPos fieldPos;

static unsigned
hex_to_val(char ch)
{
    if (ch >= '0' && ch <= '9') {
	return ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
	return 10 + ch - 'a';
    } else {
	return 10 + ch - 'A';
    }
}

static enum TokenKind
parseCharacterLiteral()
{
    // ch == '\''
    nextCh();
    if (ch == '\'') {
	appendCharToStr(&token.val, ch);
	return UNKNOWN_TOKEN;
    }
    do {
	appendCharToStr(&token.val, ch);
	if (ch == '\n') {
	    return UNKNOWN_TOKEN;
	} else if (ch == '\\') {
	    nextCh();
	    if (isOctaldigit(ch)) {
		unsigned octalval = ch - '0';
		appendCharToStr(&token.val, ch);
		nextCh();
		if (isOctaldigit(ch)) {
		    octalval = octalval * 8 + ch - '0';
		    appendCharToStr(&token.val, ch);
		    nextCh();
		}
		if (isOctaldigit(ch)) {
		    octalval = octalval * 8 + ch - '0';
		    appendCharToStr(&token.val, ch);
		    nextCh();
		}
		appendCharToStr(&token.processedVal, octalval);
	    } else {
		appendCharToStr(&token.val, ch);
		switch (ch) {
		    /* simple-escape-sequence */
		    case '\'':
			appendCharToStr(&token.processedVal, '\'');
			nextCh();
			break;
		    case '"':
			appendCharToStr(&token.processedVal, '\"');
			nextCh();
			break;
		    case '?':
			appendCharToStr(&token.processedVal, '\?');
			nextCh();
			break;
		    case '\\':
			appendCharToStr(&token.processedVal, '\\');
			nextCh();
			break;
		    case 'a':
			appendCharToStr(&token.processedVal, '\a');
			nextCh();
			break;
		    case 'b':
			appendCharToStr(&token.processedVal, '\b');
			nextCh();
			break;
		    case 'f':
			appendCharToStr(&token.processedVal, '\f');
			nextCh();
			break;
		    case 'n':
			appendCharToStr(&token.processedVal, '\n');
			nextCh();
			break;
		    case 'r':
			appendCharToStr(&token.processedVal, '\r');
			nextCh();
			break;
		    case 't':
			appendCharToStr(&token.processedVal, '\t');
			nextCh();
			break;
		    case 'v':
			appendCharToStr(&token.processedVal, '\v');
			nextCh();
			break;
		    case 'x': {
			nextCh();
			if (!isHexdigit(ch)) {
			    appendCharToStr(&token.val, ch);
			    return UNKNOWN_TOKEN;
			}
			unsigned hexval = hex_to_val(ch);
			appendCharToStr(&token.val, ch);
			nextCh();
			while (isHexdigit(ch)) {
			    appendCharToStr(&token.val, ch);
			    hexval = hexval * 16 + hex_to_val(ch);
			    nextCh();
			}
			appendCharToStr(&token.processedVal, hexval);
			break;
		    }
		    default:
			return UNKNOWN_TOKEN;
		}
	    }
	} else if (ch == EOF) {
	    // error: unexpected EOF
	    return UNKNOWN_TOKEN;
	} else {
	    appendCharToStr(&token.processedVal, ch);
	    nextCh();
	}
    } while (ch != '\'');
    nextCh();
    return CHARACTER_LITERAL;
}

static enum TokenKind
parseStringLiteral()
{
    // ch == '"'
    nextCh();
    while (ch != '"') {
	if (ch == '\n') {
	    return UNKNOWN_TOKEN;
	} else if (ch == '\\') {
	    appendCharToStr(&token.val, ch);
	    nextCh();
	    if (isOctaldigit(ch)) {
		unsigned octalval = ch - '0';
		appendCharToStr(&token.val, ch);
		nextCh();
		if (isOctaldigit(ch)) {
		    octalval = octalval * 8 + ch - '0';
		    appendCharToStr(&token.val, ch);
		    nextCh();
		}
		if (isOctaldigit(ch)) {
		    octalval = octalval * 8 + ch - '0';
		    appendCharToStr(&token.val, ch);
		    nextCh();
		}
		ch = octalval;
		appendCharToStr(&token.processedVal, ch);
	    } else {
		appendCharToStr(&token.val, ch);
		switch (ch) {
		    /* simple-escape-sequence */
		    case '\'':
			appendCharToStr(&token.processedVal, '\'');
			nextCh();
			break;
		    case '"':
			appendCharToStr(&token.processedVal, '\"');
			nextCh();
			break;
		    case '?':
			appendCharToStr(&token.processedVal, '\?');
			nextCh();
			break;
		    case '\\':
			appendCharToStr(&token.processedVal, '\\');
			nextCh();
			break;
		    case 'a':
			appendCharToStr(&token.processedVal, '\a');
			nextCh();
			break;
		    case 'b':
			appendCharToStr(&token.processedVal, '\b');
			nextCh();
			break;
		    case 'f':
			appendCharToStr(&token.processedVal, '\f');
			nextCh();
			break;
		    case 'n':
			appendCharToStr(&token.processedVal, '\n');
			nextCh();
			break;
		    case 'r':
			appendCharToStr(&token.processedVal, '\r');
			nextCh();
			break;
		    case 't':
			appendCharToStr(&token.processedVal, '\t');
			nextCh();
			break;
		    case 'v':
			appendCharToStr(&token.processedVal, '\v');
			nextCh();
			break;
		    case 'x': {
			nextCh();
			appendCharToStr(&token.val, ch);
			if (!isHexdigit(ch)) {
			    return UNKNOWN_TOKEN;
			}
			unsigned hexval = hex_to_val(ch);
			nextCh();
			while (isHexdigit(ch)) {
			    appendCharToStr(&token.val, ch);
			    hexval = hexval * 16 + hex_to_val(ch);
			    nextCh();
			}
			appendCharToStr(&token.processedVal, hexval);
			break;
		    }
		    default:
			nextCh();
			return UNKNOWN_TOKEN;
		}
	    }
	} else if (ch == EOF) {
	    return UNKNOWN_TOKEN;
	} else {
	    appendCharToStr(&token.val, ch);
	    appendCharToStr(&token.processedVal, ch);
	    nextCh();
	}
    }
    nextCh();
    return STRING_LITERAL;
}

enum TokenKind
getToken()
{
    if (!ch) {
	nextCh();
	token.loc.begin = token.loc.end;
	fieldPos = LabelField;
    } else if (fieldPos != OpField) {
	++fieldPos;
    }

    if (fieldPos == LabelField && isSpace(ch)) {
	token.loc.begin = token.loc.end;
	clearStr(&token.val);
	clearStr(&token.processedVal);
	token.kind = EMPTY_LABEL;
	return token.kind;
    }

    // skip whitespace
    while (isSpace(ch) || ch == 0) {
	nextCh();
    }

    // assemble next token
    token.loc.begin = token.loc.end;
    clearStr(&token.val);
    clearStr(&token.processedVal);

    // check for identifiers
    if (isLetter(ch)) {
	if (fieldPos == MnemonicField) {
#include <ulmas0/_gen_scanner_mnem.c>
	} else {
	    do {
		appendCharToStr(&token.val, ch);
		appendCharToStr(&token.processedVal, ch);
		nextCh();
	    } while (isLetter(ch) || isDigit(ch));
	    token.kind = IDENT;
	}
    } else if (isDigit(ch)) {
	if (ch == '0') {
	    appendCharToStr(&token.val, ch);
	    // representation?
	    nextCh();
	    if (ch == 'x' || ch == 'X') {
		appendCharToStr(&token.val, ch);
		token.kind = HEXADECIMAL_LITERAL;
		nextCh();
		if (!isHexdigit(ch)) {
		    token.kind = UNKNOWN_TOKEN;
		}
		do {
		    appendCharToStr(&token.val, ch);
		    nextCh();
		} while (isHexdigit(ch));
	    } else {
		token.kind = OCTAL_LITERAL;
		while (isOctaldigit(ch)) {
		    appendCharToStr(&token.val, ch);
		    nextCh();
		}
	    }
	} else {
	    token.kind = DECIMAL_LITERAL;
	    do {
		appendCharToStr(&token.val, ch);
		nextCh();
	    } while (isDigit(ch));
	}
	// check for character literals
    } else if (ch == '\'') {
	token.kind = parseCharacterLiteral();
	// check for string literals
    } else if (ch == '"') {
	token.kind = parseStringLiteral();
	// check for punctuators and @w[0-3] operator
    } else if (ch == '\n') {
	// ignore empty lines
	do {
	    nextCh();
	} while (ch == '\n');
	token.kind = EOL;
	fieldPos = EolField;
    } else if (ch == '@') {
	appendCharToStr(&token.val, ch);
	nextCh();
	if (ch != 'w') {
	    token.kind = UNKNOWN_TOKEN;
	} else {
	    appendCharToStr(&token.val, ch);
	    nextCh();
	    if (ch >= '0' && ch <= '3') {
		switch (ch) {
		    case '0':
			token.kind = W0;
			break;
		    case '1':
			token.kind = W1;
			break;
		    case '2':
			token.kind = W2;
			break;
		    case '3':
			token.kind = W3;
			break;
		}
		appendCharToStr(&token.val, ch);
		nextCh();
	    } else {
		token.kind = UNKNOWN_TOKEN;
	    }
	}
    } else if (ch == ':') {
	appendCharToStr(&token.val, ch);
	nextCh();
	if (fieldPos == MnemonicField) {
	    fieldPos = LabelField;
	}
	token.kind = COLON;
    } else if (ch == '#') {
	// ignore single line comment
	do {
	    nextCh();
	} while (ch != EOF && ch != '\n');
	return getToken();
    } else if (ch == '/') {
	nextCh();
	if (ch == '/') {
	    // ignore single line comment
	    do {
		nextCh();
	    } while (ch != EOF && ch != '\n');
	    nextCh();
	    return getToken();
	} else if (ch == '*') {
	    // ignore delimited comment
	    nextCh();
	    bool asterisk = false;
	    while (ch != EOF && (!asterisk || ch != '/')) {
		asterisk = ch == '*';
		nextCh();
	    }
	    if (ch == EOF) {
		// error: EOF_IN_DELIMITED_COMMENT
		token.kind = UNKNOWN_TOKEN;
	    } else {
		nextCh();
		return getToken();
	    }
	} else {
	    token.kind = SLASH;
	}
    } else if (ch == '(') {
	appendCharToStr(&token.val, ch);
	nextCh();
	// skip whitespace for LPAREN_MEM lock ahead
	while (isSpace(ch) || ch == 0) {
	    nextCh();
	}
	// now look ahead
	if (ch == '%') {
	    token.kind = LPAREN_MEM;
	} else {
	    token.kind = LPAREN;
	}
    } else if (ch == ')') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = RPAREN;
    } else if (ch == '+') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = PLUS;
    } else if (ch == '-') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = MINUS;
    } else if (ch == '*') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = ASTERISK;
    } else if (ch == '$') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = DOLLAR;
    } else if (ch == '%') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = PERCENT;
    } else if (ch == ',') {
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = COMMA;
    } else if (ch != EOF) {
	// if we get here no legal token was found
	fprintf(stderr, "illegal character '%c'\n", ch);
	appendCharToStr(&token.val, ch);
	nextCh();
	token.kind = UNKNOWN_TOKEN;
    } else {
	token.kind = EOI;
    }
    return token.kind;
}

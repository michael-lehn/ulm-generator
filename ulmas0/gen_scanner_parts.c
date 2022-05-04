#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/printcode.h>
#include <utils/str.h>

bool
isSpace(int ch)
{
    return ch == ' ' || ch == '\r' || ch == '\f' || ch == '\v' || ch == '\t';
}

bool
isEol(int ch)
{
    return ch == 0 || ch == '\n';
}

struct TokenNode
{
    bool ignore;
    const char *alias;
    const char *ident;
    struct TokenNode *next;
} * tokenNode, *tokenNodeLast;

struct
{
    struct Str identBeg, identEnd;
    bool open, usedBeg, addedEnd;

} tokenGroup;

void addToken(const char *ident);
void ignoreLastToken();
void tokenGroupEnd();

void
tokenGroupBegin(const char *ident)
{
    if (tokenGroup.open) {
	tokenGroupEnd();
    }

    /*
	now: closed == false;
	if addedEnd == true we still need the old 'identEnd' as alias
    */
    copyCStrToStr(&tokenGroup.identBeg, ident);
    appendCStrToStr(&tokenGroup.identBeg, "_FIRST");

    // use 'identEnd' if a group was just closed
    addToken(tokenGroup.identBeg.cstr);
    ignoreLastToken();

    copyCStrToStr(&tokenGroup.identEnd, ident);
    appendCStrToStr(&tokenGroup.identEnd, "_END");

    tokenGroup.open = true;
    tokenGroup.usedBeg = false;
    tokenGroup.addedEnd = false;
}

void
tokenGroupEnd()
{
    addToken(tokenGroup.identEnd.cstr);
    ignoreLastToken();
    tokenGroup.addedEnd = true;
    tokenGroup.open = false;
}

const char *
tokenGroupMark()
{
    if (tokenGroup.open && !tokenGroup.usedBeg) {
	tokenGroup.usedBeg = true;
	return tokenGroup.identBeg.cstr;
    }
    if (!tokenGroup.open && tokenGroup.addedEnd) {
	tokenGroup.addedEnd = false;
	return tokenGroup.identEnd.cstr;
    }
    return 0;
}

void
addToken(const char *ident)
{
    struct TokenNode *node = malloc(sizeof(*node));
    ident = strdup(ident);
    if (!node || !ident) {
	fprintf(stderr, "addToken: out of memory\n");
	exit(1);
    }

    node->ignore = false;
    node->ident = ident;
    const char *groupMark = tokenGroupMark();
    node->alias = groupMark ? strdup(groupMark) : 0;
    node->next = 0;

    if (tokenNode) {
	tokenNodeLast = tokenNodeLast->next = node;
    } else {
	tokenNodeLast = tokenNode = node;
    }
}

void
ignoreLastToken()
{
    if (tokenNodeLast) {
	tokenNodeLast->ignore = true;
    }
} 

void
printEnumTokenKind(FILE *out)
{
    printCode(out, 0, "enum TokenKind {\n");
    for (const struct TokenNode *n = tokenNode; n; n = n->next) {
	if (n->alias) {
	    printCode(out, 1, "%s = %s,\n", n->ident, n->alias);
	} else {
	    printCode(out, 1, "%s,\n", n->ident);
	}
    }
    printCode(out, 1, "%s,\n", "NUM_TOKENKIND");
    printCode(out, 0, "};\n");
}

void
printStrTokenKind(FILE *out)
{
    printCode(out, 0, "const char *\n");
    printCode(out, 0, "strTokenKind(enum TokenKind kind) {\n");
    printCode(out, 1, "switch (kind) {\n");

    for (const struct TokenNode *n = tokenNode; n; n = n->next) {
	if (n->ignore) {
	    continue;
	}
	printCode(out, 2, "case %s:\n", n->ident);
	printCode(out, 3, "return \"%s\";\n", n->ident);
    }

    printCode(out, 2, "case NUM_TOKENKIND: /* cause internal error */;\n");
    printCode(out, 3, ";\n");
    printCode(out, 1, "}\n");
    printCode(out, 1,
	      "fprintf(stderr, \"in 'valTokenKind': internal error\\n\");\n");
    printCode(out, 1, "exit(1);\n");
    printCode(out, 0, "}\n");
}

struct CharNode
{
    struct CharNode *next[256];
    const char *token, *tokenStr;
} root;

void
addMnemonic(const char *s, const char *token)
{
    const char *tokenStr = s;
    struct CharNode *n = &root;
    for (size_t c; (c = *s++); n = n->next[c]) {
	if (!n->next[c]) {
	    n->next[c] = calloc(1, sizeof(*n->next[c]));
	}
    }
    n->token = strdup(token);
    n->tokenStr = strdup(tokenStr);
}

void
printMnemonicC_Content_(FILE *out, struct CharNode *n, size_t level)
{
    for (size_t i = 0; i < sizeof(n->next) / sizeof(n->next[0]); ++i) {
	if (n->next[i]) {
	    printCode(out, level, "if (ch == '%c') {\n", (char)i);
	    printCode(out, level + 1, "nextCh();\n");
	    printMnemonicC_Content_(out, n->next[i], level + 1);
	    printCode(out, level, "}\n");
	}
    }
    if (n->token) {
	printCode(out, level, "copyCStrToStr(&token.val, \"%s\");\n",
		  n->tokenStr);
	printCode(out, level, "token.kind = %s;\n", n->token);
	printCode(out, level, "return token.kind;\n", n->token);
    }
}

void
printMnemonicC_Content(FILE *out)
{
    printMnemonicC_Content_(out, &root, 1);
    printCode(out, 1, "appendCharToStr(&token.val, ch);\n");
    printCode(out, 1, "nextCh();\n");
}

void
split(char *line, char **token, char **mnem)
{
    *token = 0;
    *mnem = 0;
    // skip space
    for (; isSpace(*line) && !isEol(*line); ++line) {
    }
    if (isEol(*line)) {
	return;
    }
    // mark begin of token name
    *token = line;
    // advance to end of token
    for (; !isSpace(*line) && !isEol(*line); ++line) {
    }
    if (isEol(*line)) {
	// mark end of token
	*line = 0;
	return;
    }
    // mark end of token
    *line++ = 0;
    // skip space
    for (; isSpace(*line); ++line) {
    }
    // mark begin of mnem name
    *mnem = line;
    // advance to end of mnem
    for (; !isSpace(*line) && !isEol(*line); ++line) {
    }
    *line = 0;
}

void
readTokenList(FILE *in)
{
    char *line = 0;
    size_t linecap = 0;

    for (ssize_t n; (n = getline(&line, &linecap, in)) > 0;) {
	size_t len = strlen(line);
	if (len && line[len - 1] == '\n') {
	    line[len - 1] = 0;
	}
	addToken(line);
    }
}

void
readKeywordList(FILE *in)
{
    char *line = 0;
    size_t linecap = 0;

    for (ssize_t n; (n = getline(&line, &linecap, in)) > 0;) {
	char *token = 0, *mnem = 0;
	split(line, &token, &mnem);
	if (token) {
	    addToken(token);
	    if (mnem) {
		addMnemonic(mnem, token);
	    }
	}
    }
}

int
main(int argc, char **argv)
{
    enum
    {
	IN_MNEMONIC_TXT,
	IN_DIRECTIVES_TXT,
	IN_TOKENS_TXT,
	NUM_IN,

	OUT_SCANNER_MNEM = NUM_IN,
	OUT_TOKENKIND,
	OUT_VALTOKENKIND,
	NUM_IO,
    };

    if (argc != NUM_IO + 1) {
	fprintf(stderr,
		"usage: %s mnemonic.txt directives.txt other_tokens.txt "
		"scanner_mnem.c tokenkind.h valtokenkind.h\n",
		argv[0]);
	return 1;
    }

    FILE *fd[NUM_IO];
    for (size_t i = 0; i < NUM_IO; ++i) {
	fd[i] = fopen(argv[i + 1], i < NUM_IN ? "r" : "w");
	if (!fd[i]) {
	    fprintf(stderr, "can not open %s file '%s'\n",
		    i < NUM_IN ? "input" : "output", argv[i + 1]);
	    return 1;
	}
    }

    // process mnemonics and directives
    tokenGroupBegin("MNEMONIC");
    readKeywordList(fd[IN_MNEMONIC_TXT]);
    tokenGroupBegin("DIRECTIVE");
    readKeywordList(fd[IN_DIRECTIVES_TXT]);
    tokenGroupEnd();

    // process other tokens
    readTokenList(fd[IN_TOKENS_TXT]);

    printMnemonicC_Content(fd[OUT_SCANNER_MNEM]);
    printEnumTokenKind(fd[OUT_TOKENKIND]);
    printStrTokenKind(fd[OUT_VALTOKENKIND]);

    for (size_t i = 0; i < NUM_IO; ++i) {
	fclose(fd[i]);
    }
}

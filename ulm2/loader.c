#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "loader.h"
#include "udb.h"
#include "vmem.h"

uint64_t ulm_bss;
uint64_t ulm_brk;

static FILE *in;
static char ch;
static char *inbuf, *cp;
static size_t inbufLen;
static size_t line, col;
static bool skipSpace;
static bool colInLine;
static bool directive;
static uint64_t addr;

void
ulm_loadRestart()
{
    addr = 0;
}

static void
error(const char *msg)
{
    fprintf(stderr, "error: %zu.%zu: %s\n", line, col, msg);
    exit(1);
}

static void
warning(const char *msg)
{
    fprintf(stderr, "warning: %zu.%zu: %s\n", line, col, msg);
}

static bool
isSpace(char ch)
{
    return ch == ' ' || ch == '\r' || ch == '\f' || ch == '\v' || ch == '\t';
}

static bool
isStr(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2) {
	++s1;
	++s2;
    }
    return *s2 == 0;
}

static void
readLine()
{
    if (!in) {
	// reading from string
	if (!inbuf || (inbuf != cp)) {
	    // inbuf empty or never set
	    ch = EOF;
	    return;
	}
    } else if (getline(&inbuf, &inbufLen, in) == EOF) {
	ch = EOF;
	return;
    }

    skipSpace = true;
    cp = inbuf;
    col = 0;
    ++line;
    colInLine = false;
    directive = false;
    for (char *s = inbuf; *s; ++s) {
	if (*s == '#') {
	    if (s == inbuf && (isStr(s + 1, "TEXT") || isStr(s + 1, "DATA") ||
			       isStr(s + 1, "BSS") || isStr(s + 1, "SYMTAB")))
	    {
		directive = true;
	    } else {
		*s = '\n';
	    }
	    break;
	} else if (*s == ':') {
	    colInLine = true;
	}
    }
}

static char
nextch()
{
    if (ch == EOF) {
	return EOF;
    } else if (!ch || ch == '\n') {
	readLine();
	if (ch == EOF) {
	    return EOF;
	}
    }

    do {
	ch = *cp++;
	++col;
	if (ch == '#') {
	    skipSpace = false;
	}
    } while (skipSpace && isSpace(ch));

    if (!ch) {
	return nextch();
    }
    return ch;
}

static bool
isHexDigit(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') ||
	   (ch >= 'a' && ch <= 'f');
}

static bool
isOctDigit(char ch)
{
    return ch >= '0' && ch <= '7';
}

static bool
isDecDigit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static uint8_t
hexDigitVal(char ch)
{
    if (ch >= '0' && ch <= '9') {
	return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
	return 10 + ch - 'A';
    }
    if (ch >= 'a' && ch <= 'f') {
	return 10 + ch - 'a';
    }
    fprintf(stderr, "internal error: '%c' is not a hex digit\n", ch);
    abort();
}

static bool
parseHexLiteral(uint64_t *val)
{
    if (!isHexDigit(ch)) {
	return false;
    }

    uint64_t v = 0;
    for (; isHexDigit(ch); nextch()) {
	v *= 16;
	v += hexDigitVal(ch);
    }
    *val = v;
    return true;
}

static bool
parseOctLiteral(uint64_t *val)
{
    // only called if last char was '0' so never fails
    uint64_t v = 0;
    for (; isOctDigit(ch); nextch()) {
	v *= 8;
	v += ch - '0';
    }
    *val = v;
    return true;
}

static bool
parseDecLiteral(uint64_t *val)
{
    if (!isDecDigit(ch)) {
	return false;
    }

    uint64_t v = 0;
    for (; isDecDigit(ch); nextch()) {
	v *= 10;
	v += ch - '0';
    }
    *val = v;
    return true;
}

static bool
parseLiteral(uint64_t *val)
{
    if (ch == '0') {
	nextch();
	if (ch == 'x') {
	    nextch();
	    return parseHexLiteral(val);
	} else {
	    return parseOctLiteral(val);
	}
    } else if (isDecDigit(ch)) {
	return parseDecLiteral(val);
    }
    return false;
}

static void
ignoreSpace()
{
    for (; isSpace(ch); nextch()) {
    }
}

static void
alignTo(uint64_t align)
{
    addr = (addr + align - 1) / align * align;
}

static bool
expectStr(const char *s)
{
    while (*s == ch) {
	++s;
	nextch();
    }
    return *s == 0;
}

static bool inTextSeg = true;

static bool
parseDirective()
{
    nextch(); // skip '#'
    if (ch == 'B') {
	expectStr("BSS");
	uint64_t align, size;

	ignoreSpace();
	if (!parseLiteral(&align)) {
	    error("literal for BSS alignment expected");
	}
	ignoreSpace();
	if (!parseLiteral(&size)) {
	    error("literal for BSS size expected");
	}
	alignTo(align);
	ulm_bss = addr;
	for (uint64_t i = 0; i < size; ++i, ++addr) {
	    ulm_store8(addr, 0);
	}
	inTextSeg = false;
	return true;
    } else if (ch == 'D') {
	expectStr("DATA");
	uint64_t align;

	ignoreSpace();
	if (!parseLiteral(&align)) {
	    error("literal for DATA alignment expected");
	}
	alignTo(align);
	inTextSeg = false;
	return true;
    } else if (ch == 'T') {
	expectStr("TEXT");
	uint64_t align;

	ignoreSpace();
	if (!parseLiteral(&align)) {
	    error("literal for TEXT alignment expected");
	}
	alignTo(align);
	inTextSeg = true;
	return true;
    } else if (ch == 'S') {
	expectStr("SYMTAB");
	while (ch != EOF) {
	    if (ch == '\n') {
		nextch();
		if (ch == 'U') {
		    error("program contains undefined symbols");
		}
	    }
	    nextch();
	}
	return true;
    }
    return false;
}

static bool
parse()
{
    while (nextch() != EOF) {
	if (directive) {
	    if (!parseDirective()) {
		error("directive expected");
		return false;
	    }
	} else {
	    if (colInLine) {
		uint64_t addr_;
		if (!parseLiteral(&addr_)) {
		    error("address expected");
		    return false;
		}
		if (ch != ':') {
		    error("expected ':'");
		}
		nextch();
		if (addr_ < addr) {
		    fprintf(stderr, "current address: 0x%016" PRIx64 "\n",
			    addr);
		    fprintf(stderr, "address found:   0x%016" PRIx64 "\n",
			    addr_);
		    error("illegal address");
		    return false;
		}
		addr = addr_;
	    }
	    uint64_t byte = 0;
	    size_t numHalfBytes;
	    for (numHalfBytes = 0; ch != '\n'; ++numHalfBytes) {
		if (!isHexDigit(ch)) {
		    break;
		}
		if (numHalfBytes % 2 == 0) {
		    byte = 0;
		}
		byte = byte * 16 + hexDigitVal(ch);
		if (numHalfBytes % 2 == 1) {
		    ulm_store8(addr++, byte);
		}
		nextch();
	    }
	    if (numHalfBytes % 2 == 1) {
		error("hex digit expected");
		return false;
	    }
	    if (ch != '\n') {
		error("newline expected");
		return false;
	    }
	}
	if (isSpace(ch)) {
	    warning("trailing space");
	    ignoreSpace();
	}
	if (ch != '\n' && ch != EOF) {
	    error("newline or eof expected");
	    return false;
	}
    }
    ulm_brk = addr;
    ulm_setReg(ulm_bss, 1);
    ulm_setReg(ulm_brk, 2);
    ulm_setReg((ulm_brk + 7) / 8 * 8, 3);
    // printf("prog loaded\n");
    // printVMem();
    return true;
}

bool
ulm_load_str(const char *line)
{
    ch = 0;
    char *line_ = strdup(line);
    cp = inbuf = line_;
    bool succ = parse();
    free(line_);
    return succ;
}

bool
ulm_load_fstream(FILE *in_)
{
    in = in_;
    return parse();
}

bool
ulm_load(const char *prog)
{
    in = fopen(prog, "r");
    if (!in) {
	return false;
    }
    return ulm_load_fstream(in);
}

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cg.h"
#include "encode.h"
#include "error.h"
#include "symtab.h"
#include "todo.h"
#include <utils/loc.h>
#include <utils/printcode.h>

static enum CgSeg seg = CGSEG_TEXT;
static uint64_t addr[CGSEG_NUM_SEGMENTS];
static uint64_t alignment[CGSEG_NUM_SEGMENTS] = { 4, 1, 1 };

uint64_t cgSegStart[CGSEG_NUM_SEGMENTS];
uint64_t cgSegEnd[CGSEG_NUM_SEGMENTS];
bool cgSegFinalized;

static uint64_t
roundUp(uint64_t a, uint64_t m)
{
    return (a + m - 1) / m * m;
}

static uint64_t
gcd(uint64_t a, uint64_t b)
{
    while (b != 0) {
	size_t r = a % b;
	a = b;
	b = r;
    }
    return a;
}

enum
{
    PAGE_BITS = 16,
    PAGE_SIZE = 1 << PAGE_BITS,
    SET_BITS = 8,
    SET_SIZE = 1 << SET_BITS,
};

struct Page
{
    uint64_t tag;
    uint8_t byte[PAGE_SIZE];
    struct Page *next;
};

static struct Page *table[CGSEG_DATA + 1][SET_SIZE];
static struct Page *page[CGSEG_DATA + 1];
static uint64_t pageOffset[CGSEG_DATA + 1];

static struct Page *
getPage(enum CgSeg fromSeg, uint64_t addr, uint64_t *pageOffset)
{
    *pageOffset = addr & (PAGE_SIZE - 1);
    uint64_t set = (addr >> PAGE_BITS) & (SET_SIZE - 1);
    uint64_t tag = addr >> (PAGE_BITS + SET_BITS);

    for (struct Page *p = table[fromSeg][set]; p; p = p->next) {
	if (p->tag == tag) {
	    return p;
	}
    }

    // create a clean new memory page
    struct Page *p = calloc(1, sizeof(*p));

    if (!p) {
	fprintf(stderr, "internal error: out of memory\n");
	exit(1);
    }

    p->tag = tag;
    if (table[fromSeg][set]) {
	p->next = table[fromSeg][set];
    }
    table[fromSeg][set] = p;
    return p;
}

void
cgFixBytes(enum CgSeg cgSeg, uint64_t addr, size_t numBytes, uint64_t val)
{
    uint64_t pageOffset;
    struct Page *p = getPage(cgSeg, addr, &pageOffset);
    assert(pageOffset + numBytes <= PAGE_SIZE);

    for (size_t i = 0; i < numBytes; ++i) {
	p->byte[pageOffset + numBytes - i - 1] |= val;
	val >>= 8;
    }
}

void
cgAppendBytes(size_t numBytes, uint64_t val)
{
    assert(numBytes <= 8);
    assert(!cgSegFinalized);
    if (addr[seg] % numBytes) {
	cgAlign(numBytes);
    }

    if (seg == CGSEG_BSS) {
	if (val) {
	    error("can not write non-zero values into BSS segment\n");
	}
    } else {
	assert(pageOffset[seg] % numBytes == 0);

	if (!page[seg] || pageOffset[seg] >= PAGE_SIZE) {
	    // get new page
	    page[seg] = getPage(seg, addr[seg], &pageOffset[seg]);
	}

	// write bytes in big endian format
	for (size_t i = 0; i < numBytes; ++i) {
	    page[seg]->byte[pageOffset[seg] + numBytes - i - 1] = val & 0xFF;
	    val >>= 8;
	}
	pageOffset[seg] += numBytes;
    }
    addr[seg] += numBytes;
}

void
cgAppendString(const char *str)
{
    for (const char *s = str; *s; ++s) {
	cgAppendBytes(1, *s);
    }
    cgAppendBytes(1, 0);
}

void
cgAppendSpace(size_t numBytes)
{
    while (numBytes--) {
	cgAppendBytes(1, 0);
    }
}

void
cgSetActiveSegment(enum CgSeg cgSeg_)
{
    seg = cgSeg_;
}

enum CgSeg
cgGetActiveSegment(void)
{
    return seg;
}

uint64_t
cgGetAddr(void)
{
    return addr[seg];
}

void
cgAlign(size_t alignTo)
{
    if (alignTo == 0) {
	error("alignment has to be larger than 0\n");
    }
    uint64_t padding = roundUp(addr[seg], alignTo) - addr[seg];

    if (seg == CGSEG_TEXT) {
	if (padding % 4) {
	    error("can not achieve alignment with nop operations (required in "
		  "text segment)");
	}
	for (uint64_t i = 0; i < padding; i += 4) {
	    cgAppendBytes(4, TODO_NOP_INSTR);
	    cgAppendComment("nop");
	}
    } else if (seg == CGSEG_DATA || seg == CGSEG_BSS) {
	for (uint64_t i = 0; i < padding; ++i) {
	    cgAppendBytes(1, 0x00);
	}
	if (padding) {
	    cgAppendComment(" for alignment");
	}
    } else {
	fprintf(stderr, "internal error in 'segAlign': seg = %d\n", seg);
	assert(0); // seg has illegal value
    }

    alignment[seg] *= alignTo / gcd(alignment[seg], alignTo);
}

static struct CommentNode
{
    uint64_t addr;
    const char *str;
    struct CommentNode *next;
} * comment[CGSEG_DATA + 1], *commentLast[CGSEG_DATA + 1];

void
cgAppendComment(const char *str)
{
    if (!*str || seg == CGSEG_BSS) {
	return;
    }
    str = strdup(str);
    struct CommentNode *node = malloc(sizeof(*node));

    if (!str || !node) {
	fprintf(stderr, "addInstr: out of memory\n");
	exit(1);
    }

    node->addr = addr[seg];
    node->str = str;
    node->next = 0;

    if (comment[seg]) {
	commentLast[seg] = commentLast[seg]->next = node;
    } else {
	commentLast[seg] = comment[seg] = node;
    }
}

void
cgAppendInstr(uint32_t opCode, ...)
{
    va_list argp;
    va_start(argp, opCode);
    uint32_t code = encodeInstr(seg, addr[seg], opCode, argp);
    va_end(argp);

    cgAppendBytes(4, code);
}

void
cgAppendInteger(size_t numBytes, struct Expr *expr)
{
    uint64_t val = encodeExpr(seg, addr[seg], numBytes, expr);
    cgAppendBytes(numBytes, val);
}

void
cgSetLabel(struct Loc loc, const struct UStr *ident)
{
    enum ExprType type = seg == CGSEG_TEXT   ? REL_TEXT
			 : seg == CGSEG_DATA ? REL_DATA
					     : REL_BSS;
    symtabSet(ident, makeValExpr(loc, type, addr[seg]));
}

static void
finalizeSegments()
{
    for (size_t i = CGSEG_DATA; i <= CGSEG_BSS; ++i) {
	cgSegEnd[i - 1] = cgSegStart[i - 1] + addr[i - 1];
	cgSegStart[i] = roundUp(cgSegEnd[i - 1], alignment[i]);
    }
    cgSegFinalized = true;
}

void
cgPrint(FILE *out)
{
    finalizeSegments();
    encodeFixables();

    uint64_t pageOffset;
    struct Page *p;

    p = 0;
    pageOffset = 0;
    printCode(out, 0, "#TEXT %" PRIu64 "\n", alignment[CGSEG_TEXT]);
    for (uint64_t i = 0; i < addr[CGSEG_TEXT]; i += 4, pageOffset += 4) {
	printCode(out, 0, "0x%016" PRIX64 ":", i + cgSegStart[CGSEG_TEXT]);

	if (!p || pageOffset >= PAGE_SIZE) {
	    // get new page
	    p = getPage(CGSEG_TEXT, i, &pageOffset);
	}

	printCode(out, 0, " %02" PRIX8, p->byte[pageOffset + 0]);
	printCode(out, 0, " %02" PRIX8, p->byte[pageOffset + 1]);
	printCode(out, 0, " %02" PRIX8, p->byte[pageOffset + 2]);
	printCode(out, 0, " %02" PRIX8, p->byte[pageOffset + 3]);

	while (comment[CGSEG_TEXT] && comment[CGSEG_TEXT]->addr <= i + 4) {
	    printCode(out, 0, " #     %s", comment[CGSEG_TEXT]->str);
	    comment[CGSEG_TEXT] = comment[CGSEG_TEXT]->next;
	}

	printCode(out, 0, "\n");
    }

    p = 0;
    pageOffset = 0;
    printCode(out, 0, "#DATA %" PRIu64 "\n", alignment[CGSEG_DATA]);

    bool printDataAddr = true;
    for (uint64_t i = 0; i < addr[CGSEG_DATA]; ++i, ++pageOffset) {
	if (printDataAddr) {
	    printCode(out, 0, "0x%016" PRIX64 ":", i + cgSegStart[CGSEG_DATA]);
	    printDataAddr = false;
	}

	if (!p || pageOffset >= PAGE_SIZE) {
	    // get new page
	    p = getPage(CGSEG_DATA, i, &pageOffset);
	}

	printCode(out, 0, " %02" PRIX8, p->byte[pageOffset]);

	while (comment[CGSEG_DATA] && comment[CGSEG_DATA]->addr <= i + 1) {
	    printCode(out, 0, " #     %s\n", comment[CGSEG_DATA]->str);
	    comment[CGSEG_DATA] = comment[CGSEG_DATA]->next;
	    printDataAddr = true;
	}
    }
    if (!printDataAddr) {
	printCode(out, 0, "\n");
    }

    printCode(out, 0, "#BSS %" PRIu64 " %" PRIu64 "\n", alignment[CGSEG_BSS],
	      addr[CGSEG_BSS]);

}


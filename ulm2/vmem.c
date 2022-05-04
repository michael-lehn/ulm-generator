#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cu.h"
#include "udb.h"
#include "vmem.h"

enum
{
    PAGE_BITS = 16,
    PAGE_SIZE = 1 << PAGE_BITS,
    SET_BITS = 8,
    SET_SIZE = 1 << SET_BITS,
};

enum
{
    BYTE = 1,
    WORD = 2,
    LONG = 4,
    QUAD = 8
};

struct SegmentNode *segment;

const struct SegmentNode *
ulm_vmemGetSegmentList()
{
    return segment;
}

static void
insertSegmentNode(struct SegmentNode *after, uint64_t offset)
{
    struct SegmentNode *node = calloc(1, sizeof(*node));
    if (!node) {
	fprintf(stderr, "makeSegmentNode: out of memory\n");
	exit(1);
    }
    node->offset = offset;
    node->size = PAGE_SIZE;
    if (after) {
	node->next = after->next;
	after->next = node;
    } else {
	node->next = segment;
	segment = node;
    }
}

static void
addPageToSegment(uint64_t addr)
{
    uint64_t pageAddr = (addr >> PAGE_BITS) << PAGE_BITS;

    for (struct SegmentNode *n = segment; n; n = n->next) {
	if (pageAddr == n->offset + n->size) {
	    // extend segment
	    n->size += PAGE_SIZE;
	    struct SegmentNode *next = n->next;
	    // if possible: join current segment with next segment
	    if (next && next->offset == n->offset + n->size) {
		n->size += next->size;
		n->next = next->next;
		free(next);
	    }
	    return;
	} else if (pageAddr > n->offset + n->size) {
	    // insert new node after current node n
	    insertSegmentNode(n, pageAddr);
	    return;
	}
    }
    insertSegmentNode(segment, pageAddr);
}

void
ulm_vmemGetSegment(uint64_t addr, uint64_t *segOffset, uint64_t *segSize)
{
    assert(segOffset);
    assert(segSize);
    for (struct SegmentNode *n = segment; n; n = n->next) {
	if (addr >= n->offset && addr <= n->offset + n->size - 1) {
	    *segOffset = n->offset;
	    *segSize = n->size;
	    return;
	}
    }
    *segSize = 0;
}

void
ulm_vmemSegmentPrint(void)
{
    printf("segments\n");
    printf("========\n");
    for (struct SegmentNode *n = segment; n; n = n->next) {
	printf("0x%016" PRIX64 " - 0x%016" PRIX64 "\n", n->offset,
	       n->offset + n->size - 1);
    }
    printf("\n");
}

struct Page
{
    uint64_t tag;
    uint8_t byte[PAGE_SIZE];
    struct Page *next;
};

static struct Page *table[SET_SIZE];

static struct Page *
getPage(uint64_t addr, uint64_t *offset, bool createPage)
{
    *offset = addr & (PAGE_SIZE - 1);
    uint64_t set = (addr >> PAGE_BITS) & (SET_SIZE - 1);
    uint64_t tag = addr >> (PAGE_BITS + SET_BITS);

    for (struct Page *p = table[set]; p; p = p->next) {
	if (p->tag == tag) {
	    return p;
	}
    }
    if (!createPage) {
	return 0;
    }

    // create a clean new memory page
    struct Page *p = calloc(1, sizeof(*p));

    if (!p) {
	fprintf(stderr, "internal error: out of memory\n");
	abort();
    }

    p->tag = tag;
    if (table[set]) {
	p->next = table[set];
    }
    table[set] = p;
    ++udb_numPages;
    addPageToSegment(addr);
    return p;
}

uint8_t
ulm_vmemPeek(uint64_t addr)
{
    uint64_t offset;
    struct Page *p = getPage(addr, &offset, false);

    if (!p) {
	return 0;
    }
    return p->byte[offset];
}

void
ulm_vmemClear()
{
    for (uint64_t set = 0; set < SET_SIZE; ++set) {
	// clear all pages in this set
	while (table[set]) {
	    struct Page *page = table[set];
	    table[set] = table[set]->next;
	    free(page);
	}
    }
}

void
ulm_fetch32(uint64_t addr, uint32_t *val)
{
    if (!val) {
	fprintf(stderr, "internal error: 'fetch32' requires non zero register");
	abort();
    }
    if (addr % 4) {
	fprintf(stderr, "%%IP: 0x%016" PRIx64 "\n", ulm_instrPtr); 
	fprintf(stderr, "bad alignment: addr = 0x%016" PRIx64 ", size = 4\n",
		addr);
	abort();
    }
    uint64_t offset;
    struct Page *p = getPage(addr, &offset, true);

    *val = 0;
    for (size_t i = 0; i < 4; ++i) {
	*val = *val << 8 | p->byte[offset + i];
    }
}

void
ulm_fetch64(int64_t disp, ulm_Reg base, ulm_Reg index, uint64_t scale,
	    enum ulm_Extension ext, size_t numBytes, ulm_Reg dest)
{
    if (!dest) {
	return;
    }

    uint64_t addr = disp + ulm_regVal(base) + scale * ulm_regVal(index);

    if (addr % numBytes) {
	fprintf(stderr, "%%IP: 0x%016" PRIx64 "\n", ulm_instrPtr); 
	fprintf(stderr, "bad alignment: addr = 0x%016" PRIx64 ", size = %zu\n",
		addr, numBytes);
	fprintf(stderr,
		"disp = %" PRId64 ", base = %" PRIu64
		", ulm_regVal(base) = 0x%" PRIu64 ", "
		"index = %" PRIu64 ", ulm_regVal(index) = 0x%" PRIu64 "\n",
		disp, base, ulm_regVal(base), index, ulm_regVal(index));
	abort();
    }

    uint64_t offset;
    struct Page *p = getPage(addr, &offset, true);
    uint64_t *val = ulm_regDevice(dest);

    *val = 0;
    for (size_t i = 0; i < numBytes; ++i) {
	*val = *val << 8 | p->byte[offset + i];
    }
    if (udb_inStep) {
	for (size_t i = 0; i < numBytes; ++i) {
	    udb_addMemMsg(addr + i, UDB_READ);
	}
	udb_addRegMsg(dest, UDB_WRITE);
    }

    if (numBytes == QUAD || ext == ULM_ZERO_EXT) {
	return;
    }
    *val = ((int64_t)*val << (64 - 8 * numBytes)) >> (64 - 8 * numBytes);
}

void
ulm_store64(int64_t disp, ulm_Reg base, ulm_Reg index, uint64_t scale,
	    size_t numBytes, ulm_Reg src)
{
    uint64_t addr = disp + ulm_regVal(base) + scale * ulm_regVal(index);

    if (addr % numBytes) {
	fprintf(stderr, "%%IP: 0x%016" PRIx64 "\n", ulm_instrPtr); 
	fprintf(stderr, "bad alignment: addr = 0x%016" PRIu64 ", size = %zu\n",
		addr, numBytes);
	abort();
    }

    /*
    fprintf(stderr, "store64: addr = 0x%016" PRIX64 ", size = %zu\n",
	    addr, numBytes);
    fprintf(stderr,
	    "disp = 0x%" PRIX64 ", base = %" PRIX64
	    ", ulm_regVal(base) = 0x%" PRIX64 ", "
	    "index = %" PRIX64 ", ulm_regVal(index) = 0x%" PRIX64 "\n",
	    disp, base, ulm_regVal(base), index, ulm_regVal(index));
	    */

    uint64_t offset;
    struct Page *p = getPage(addr, &offset, true);

    uint64_t data = src ? *ulm_regDevice(src) : 0;
    // fprintf(stderr, "data = 0x%" PRIX64 "\n", data);

    for (size_t i = 0; i < numBytes; ++i) {
	p->byte[offset + numBytes - i - 1] = data & 0xFF;
	data >>= 8;
    }
    if (udb_inStep) {
	for (size_t i = 0; i < numBytes; ++i) {
	    udb_addMemMsg(addr + i, UDB_WRITE);
	}
	udb_addRegMsg(src, UDB_READ);
    }
}

uint8_t
ulm_fetch8(uint64_t addr)
{
    uint64_t offset;
    struct Page *p = getPage(addr, &offset, true);

    return p->byte[offset];
}

void
ulm_store8(uint64_t addr, uint8_t val)
{
    uint64_t offset;
    struct Page *p = getPage(addr, &offset, true);

    p->byte[offset] = val;
    if (udb_inStep) {
	udb_addMemMsg(addr, UDB_WRITE);
    }
}

void
ulm_printVMemMapped()
{
    for (size_t set = 0; set < SET_SIZE; ++set) {
	printf("set 0x%zx:\n", set);
	for (struct Page *p = table[set]; p; p = p->next) {
	    printf("tag 0x%" PRIx64 ":\n", p->tag);
	    for (size_t offset = 0; offset < PAGE_SIZE; ++offset) {
		printf(" 0x%02x", (int)p->byte[offset]);
	    }
	    printf("\n");
	}
    }
}

void
ulm_printVMem(uint64_t begin, uint64_t end, size_t chunkSize)
{
    for (uint64_t addr = begin; addr < end; addr += chunkSize) {
	printf("0x%016" PRIx64 ": ", addr);

	for (size_t i = 0; i < chunkSize; ++i) {
	    uint64_t offset;
	    struct Page *p = getPage(addr + i, &offset, true);

	    printf("%02x ", p->byte[offset]);
	}
	printf("\n");
    }
}

#ifndef ULMAS1_CG_H
#define ULMAS1_CG_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "expr.h"
#include <utils/loc.h>
#include <utils/ustr.h>

enum CgSeg
{
    CGSEG_TEXT,
    CGSEG_DATA,
    CGSEG_BSS,
    CGSEG_NUM_SEGMENTS,
};

uint64_t cgSegStartAddr(enum CgSeg);
uint64_t cgSegEndAddr(enum CgSeg);

extern bool cgSegFinalized;

void cgFixBytes(enum CgSeg cgSeg, uint64_t addr, size_t numBytes, uint64_t mask,
		uint64_t val);
void cgAppendBytes(size_t numBytes, uint64_t val);

void cgSetActiveSegment(enum CgSeg cgSeg_);
enum CgSeg cgGetActiveSegment(void);
void cgAlign(size_t alignTo);
void cgAppendSpace(size_t numBytes);
void cgSetLabel(struct Loc loc, const struct UStr *ident);
void cgAppendInstr(uint32_t opCode, ...);
void cgAppendInteger(size_t numBytes, struct Expr *expr);
void cgAppendString(const char *str);
void cgAppendComment(const char *str);
void cgPrint(FILE *out);

#endif // ULMAS1_CG_H


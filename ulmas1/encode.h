#ifndef ULMAS1_ENCODE_H
#define ULMAS1_ENCODE_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "cg.h"

// each expr in vl will be destructed, i.e. freeExpr() is called
uint32_t encodeInstr(enum CgSeg cgSeg, uint64_t addr, uint32_t opCode,
		     va_list vl);

// will destruct expr, i.e. freeExpr() is called
uint64_t encodeExpr(enum CgSeg cgSeg, uint64_t addr, size_t numBytes,
		    struct Expr *expr);

void encodeFixables();
void encodePrintFixups(FILE *out);


#endif // ULMAS1_ENCODE_H

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cg.h"
#include "comment.h"
#include "error.h"
#include "expected.h"
#include "lexer.h"
#include "symtab.h"
#include <ulm1/fmt.h>
#include <utils/printcode.h>

static struct FixupNode
{
    // where to fix
    enum CgSeg cgSeg;
    uint64_t addr;
    size_t bitOffset, numBits;

    // how to fix
    struct Expr *expr;
    enum FmtFieldType fieldType;

    struct FixupNode *next;
} * fixups;

static struct FixupNode *
addFixupNode(enum CgSeg cgSeg, uint64_t addr, size_t bitOffset, size_t numBits,
	     struct Expr *expr)
{
    struct FixupNode *node = malloc(sizeof(*node));
    if (!node) {
	fprintf(stderr, "addFixupNode: out of memory\n");
	exit(1);
    }

    node->cgSeg = cgSeg;
    node->addr = addr;
    node->bitOffset = bitOffset;
    node->numBits = numBits;
    node->expr = expr;

    // prepend to list
    node->next = fixups;
    fixups = node;

    return node;
}

static void
addFixupField(enum CgSeg cgSeg, uint64_t addr, size_t bitOffset, size_t numBits,
	      struct Expr *expr, enum FmtFieldType fieldType)
{
    struct FixupNode *node =
      addFixupNode(cgSeg, addr, bitOffset, numBits, expr);

    node->fieldType = fieldType;
}

static void
addFixupInteger(enum CgSeg cgSeg, uint64_t addr, size_t bitOffset,
		size_t numBits, struct Expr *expr)
{
    addFixupField(cgSeg, addr, bitOffset, numBits, expr, SIGNED);
}

static uint64_t
encodeFieldType(uint64_t addr, enum FmtFieldType fieldType, struct Loc loc,
		enum ExprType valType, uint64_t val)
{
    if (fieldType == JMP_OFFSET) {
	assert(valType == ABS || valType == REL_TEXT || valType == REL_DATA ||
	       valType == REL_BSS);
	if (valType != ABS) {
	    val -= addr;
	}
	if (val % 4) {
	    errorAt(loc,
		    "offset for a relative jump has to be a multiple of 4. "
		    "Offset is %" PRId64 "\n",
		    val);
	    return 0;
	}
	val = (int64_t)val / 4;
    }
    return val;
}

static bool
checkRange(size_t numBits, enum FmtFieldType fieldType, struct Loc loc,
	   uint64_t val, bool withErrorMsg)
{
    uint64_t compare;
    switch (fieldType) {
	case JMP_OFFSET:
	case SIGNED: {
	    compare = ((int64_t)val << (64 - numBits)) >> (64 - numBits);
	    if (compare != val) {
		if (withErrorMsg) {
		    errorAt(loc,
			    "expression value %" PRId64 " (0x%" PRIX64
			    ") not within range [-2^%zu, 2^%zu)\n",
			    (int64_t)val, val, numBits - 1, numBits - 1);
		}
		return false;
	    }
	    return true;
	}
	case UNSIGNED: {
	    compare = (val << (64 - numBits)) >> (64 - numBits);
	    if (compare != val) {
		if (withErrorMsg) {
		    errorAt(loc,
			    "expression value %" PRId64 " (0x%" PRIX64
			    ") not within range [0, 2^%zu)\n",
			    val, val, numBits);
		}
		return false;
	    }
	    return true;
	}
	default:
	    assert(0);
	    return false;
    }
}

static void
encodeOperand(uint32_t *instr, size_t bitPos, size_t numBits, uint64_t val)
{
    assert(numBits <= 32);
    uint32_t mask = ~(uint32_t)0 >> (32 - numBits);
    val = (val & mask) << bitPos;
    *instr |= val;
}

static bool
fixupRequired(enum CgSeg cgSeg, enum ExprType valType,
	      enum FmtFieldType fieldType)
{
    if (valType == ABS) {
	return false;
    }
    if (fieldType == JMP_OFFSET) {
	if (valType == REL_TEXT && cgSeg == CGSEG_TEXT) {
	    return false;
	}
	if (valType == REL_DATA && cgSeg == CGSEG_DATA) {
	    return false;
	}
	if (valType == REL_BSS && cgSeg == CGSEG_BSS) {
	    return false;
	}
    }
    return true;
}

static void
encodeExprOperand(enum CgSeg cgSeg, uint64_t addr, uint32_t *instr,
		  size_t bitPos, size_t numBits, enum FmtFieldType fieldType,
		  struct Expr *expr)
{
    if (!expr) {
	return;
    }

    uint64_t val = evalExpr(expr);
    enum ExprType valType = typeExpr(expr);
    struct Loc loc = locExpr(expr);

    if (fixupRequired(cgSeg, valType, fieldType)) {
	addFixupField(cgSeg, addr, 32 - bitPos - numBits, numBits, expr,
		      fieldType);
    } else {
	freeExpr(expr);
	val = encodeFieldType(addr, fieldType, loc, valType, val);
	checkRange(numBits, fieldType, loc, val, true);
	encodeOperand(instr, bitPos, numBits, val);
    }
}

// include implementation of 'static uint32_t encodeInstr()'
#include <ulm1/_gen_fmt_instr_encoding.c>

uint64_t
encodeExpr(enum CgSeg cgSeg, uint64_t addr, size_t numBytes, struct Expr *expr)
{
    if (!expr) {
	return 0;
    }

    uint64_t val = evalExpr(expr);
    enum ExprType valType = typeExpr(expr);
    struct Loc loc = locExpr(expr);

    if (expr && fixupRequired(cgSeg, valType, SIGNED)) {
	addFixupInteger(cgSeg, addr, 0, numBytes * 8, expr);
    } else {
	size_t numBits = numBytes * 8;
	bool checkSigned = checkRange(numBits, SIGNED, loc, val, false);
	bool checkUnsigned = checkRange(numBits, UNSIGNED, loc, val, false);

	if (!checkSigned && !checkUnsigned) {
	    errorAt(loc,
		    "expression value %s can not be encoded with %zu bits\n",
		    strExpr(expr), numBits);
	}
	freeExpr(expr);
    }
    return val;
}

void
encodeFixables()
{
    symtabResolve();
    for (struct FixupNode *n = fixups; n; n = n->next) {
	uint64_t val = evalExpr(n->expr);
	enum ExprType valType = typeExpr(n->expr);
	struct Loc loc = locExpr(n->expr);


	/*
	   if expr type is REL_TEXT, REL_DATA or REL_BSS the value has changed
	   as now the size of the segements is known.
	*/
	if (valType == UNKNOWN) {
            continue;
        } else if (valType == REL_TEXT) {
            val += cgSegStartAddr(CGSEG_TEXT);
        } else if (valType == REL_DATA) {
            val += cgSegStartAddr(CGSEG_DATA);
        } else if (valType == REL_BSS) {
            val += cgSegStartAddr(CGSEG_BSS);
        }
	val = encodeFieldType(cgSegStartAddr(n->cgSeg) + n->addr, n->fieldType,
			      loc, valType, val);
	checkRange(n->numBits, n->fieldType, loc, val, true);
	size_t numBytes = (n->bitOffset + n->numBits + 8 - 1) / 8;
	val <<= (numBytes * 8 - n->bitOffset - n->numBits);
	uint64_t mask = 0;
	if (n->numBits < 64) {
	    mask = ((uint64_t)1 << n->numBits) - 1;
	    mask <<= 64 - n->bitOffset;
	    mask = ~mask;
	}
	cgFixBytes(n->cgSeg, n->addr, numBytes, mask, val);
    }
}

void
encodePrintFixups(FILE *out)
{
    bool first = true;
    for (struct FixupNode *n = fixups; n; n = n->next) {
	if (!fixupRequired(n->cgSeg, typeExpr(n->expr), n->fieldType)) {
	    continue;
	}
	if (first) {
	    printCode(out, 0, "#FIXUPS\n");
	    first = false;
	}
	assert(n->cgSeg != CGSEG_BSS);
	printCode(out, 0, "%s ", n->cgSeg == CGSEG_TEXT ? "text" : "data");
	printCode(out, 0, "0x%016" PRIX64 " ", n->addr);
	printCode(out, 0, "%2zu %2zu ", n->bitOffset, n->numBits);
	printCode(out, 0, "%s ",
		  n->fieldType == JMP_OFFSET ? "relative" : "absolute");
	const char *relocStr = relocStrExpr(n->expr);
	if (!relocStr) {
	    errorAt(locExpr(n->expr), "Expression '%s' is not relocatable\n",
		    strExpr(n->expr));
	}
	printCode(out, 0, "%s\n", relocStr ? relocStr : "not relocatable");
    }
}

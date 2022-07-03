#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "fmt.h"
#include <utils/printcode.h>
#include <utils/str.h>

/*
    List of formats linke 'RRR (OP u 8) (X u 8) (Y  u  8) (Z u  8)'
    Each format node has:
    - an identifier
    - a list with its fields
    - the number of fields
    - a list with opcodes using this format
    - a pointer to the next format node
*/
static struct FmtNode
{
    const struct UStr *id;

    // struct FmtField represents fields like (OP u 8)
    struct FmtFieldNode
    {
	const struct UStr *id;
	// position in list
	size_t index;
	// bitPos = position of least significant bit
	size_t bitPos, numBits;
	enum FmtFieldType type;
	struct FmtFieldNode *next;
    } * field, *fieldLast;

    size_t numFieldNodes;

    struct OpCodeNode
    {
	uint32_t cached;
	struct OpCodeNode *next;
    } * opCode, *opCodeLast;

    struct FmtNode *next;

} * fmtNode, *fmtNodeLast;

// size of Op code needs to be the same for all formats
static size_t sizeOpCode;

struct FmtNode *
addFmt(const struct UStr *fmtId)
{
    if (getFmt(fmtId)) {
	error("Format '%s' already defined\n", fmtId->cstr);
	return 0;
    }
    struct FmtNode *node = malloc(sizeof(*node));
    if (!node) {
	fprintf(stderr, "addFmt: out of memory\n");
	exit(1);
    }
    node->id = fmtId;
    node->field = node->fieldLast = 0;
    node->numFieldNodes = 0;
    node->opCode = node->opCodeLast = 0;
    node->next = 0;

    if (fmtNode) {
	fmtNodeLast = fmtNodeLast->next = node;
    } else {
	fmtNode = fmtNodeLast = node;
    }
    return fmtNodeLast;
}

struct FmtNode *
getFmt(const struct UStr *fmtId)
{
    for (struct FmtNode *n = fmtNode; n; n = n->next) {
	if (n->id == fmtId) {
	    return n;
	}
    }
    return 0;
}

const char *
getFmtId(const struct FmtNode *fmt)
{
    return fmt->id->cstr;
}

void
appendFmtField(struct FmtNode *fmt, const struct UStr *fieldId, size_t numBits,
	       enum FmtFieldType type)
{
    assert(fmt);
    assert(fmtNode);

    if (!fmt->field) {
	// this is the first field with the op code
	if (fmtNode == fmt) {
	    // this is the first format and defines the size of the op code
	    sizeOpCode = numBits;
	} else {
	    if (numBits != sizeOpCode) {
		error("Size of the op code fiels can not vary. Its size was "
		      "specified to have  %zu bits by format '%s'.\n",
		      sizeOpCode, fmtNode->id->cstr);
	    }
	}
    }

    size_t prevFieldSize = 0;
    size_t index = 0;
    for (const struct FmtFieldNode *f = fmt->field; f; f = f->next, ++index) {
	if (f->id == fieldId) {
	    error("Format '%s' already has a field called '%s'\n",
		  fmt->id->cstr, fieldId->cstr);
	}
	prevFieldSize += f->numBits;
    }
    if (prevFieldSize + numBits > 32) {
	error("previous fields have a total size of %zu bits. Appending field "
	      "'%s' of with %zu bits exceeds 32 bits.\n",
	      prevFieldSize, fieldId->cstr, numBits);
    }

    struct FmtFieldNode *node = malloc(sizeof(*node));
    if (!node) {
	fprintf(stderr, "appendFmtField: out of memory\n");
	exit(1);
    }
    node->id = fieldId;
    node->index = index;
    node->bitPos = 32 - (prevFieldSize + numBits);
    node->numBits = numBits;
    node->type = type;
    node->next = 0;

    // append field
    if (fmt->field) {
	fmt->fieldLast = fmt->fieldLast->next = node;
    } else {
	fmt->field = fmt->fieldLast = node;
    }
    ++fmt->numFieldNodes;
}

size_t
getFmtNumFields(const struct FmtNode *fmt)
{
    return fmt->numFieldNodes;
}

const struct FmtFieldNode *
getFmtField(const struct FmtNode *fmt, const struct UStr *fieldId)
{
    for (const struct FmtFieldNode *f = fmt->field; f; f = f->next) {
	if (f->id->cstr == fieldId->cstr) {
	    return f;
	}
    }
    return 0;
}

const char *
getFmtFieldId(const struct FmtFieldNode *field)
{
    assert(field);
    return field->id->cstr;
}

size_t
getFmtFieldIndex(const struct FmtFieldNode *field)
{
    assert(field);
    return field->index;
}

void
printRefmanFieldsDescription(FILE *out)
{
    const char *fType[] = { "Signed", "Unsigned", "JumpOffset" };
    for (const struct FmtNode *n = fmtNode; n; n = n->next) {
	printCode(out, 0, "namespace %s {\n", n->id->cstr);

	for (const struct FmtFieldNode *f = n->field; f; f = f->next) {
	    printCode(out, 1,
		      "FormatFieldNode %s{\"%s\", \"%s\", %zu, %zu, "
		      "FormatFieldNode::%s};\n",
		      f->id->cstr, n->id->cstr, f->id->cstr, f->bitPos,
		      f->numBits, fType[f->type]);
	}

	printCode(out, 0, "} // namespace %s\n\n", n->id->cstr);
    }
}

void
printRefmanFormatDescription(FILE *out)
{
    for (const struct FmtNode *n = fmtNode; n; n = n->next) {
	printCode(out, 0, "namespace ULM_FMT { namespace %s {\n", n->id->cstr);
	for (const struct FmtFieldNode *f = n->field; f; f = f->next) {
	    printCode(
	      out, 1,
	      "auto %s = ulmdoc::declareBitField(\"%s\", Expr::%s, %zu);\n",
	      f->id->cstr, f->id->cstr,
	      f->type == SIGNED	    ? "SIGNED"
	      : f->type == UNSIGNED ? "UNSIGNED"
				    : "JUMP_OFFSET",
	      f->numBits);
	}
	printCode(out, 1,
		  "auto ulm_instrFmt = ulmdoc::InstrFmt({");
	for (const struct FmtFieldNode *f = n->field; f; f = f->next) {
	    printCode(out, 0, "%s", f->id->cstr);
	    if (f->next) {
		printCode(out, 0, ", ", f->id->cstr);
	    }
	}
	printCode(out, 0, "});\n");

	printCode(out, 0, "}} // namespace ULM_FMT::%s\n\n", n->id->cstr);
    }

    /*
    for (const struct FmtNode *n = fmtNode; n; n = n->next) {
	printCode(out, 0, "addFormat(\"%s\", {", n->id->cstr);
	for (const struct FmtFieldNode *f = n->field; f; f = f->next) {
	    printCode(out, 0, "%s::%s", n->id->cstr, f->id->cstr);
	    if (f->next) {
		printCode(out, 0, ", ");
	    }
	}
	printCode(out, 0, "});\n");
	for (const struct FmtFieldNode *f = n->field; f; f = f->next) {
	    printCode(out, 0, "mathOperator.insert(\"%s\");\n", f->id->cstr);
	}
    }
    */
}

void
addOpCodeToFmt(struct FmtNode *fmt, uint32_t opCode)
{
    struct OpCodeNode *node = malloc(sizeof(*node));
    if (!node) {
	fprintf(stderr, "addOpCodeToFmt: out of memory\n");
	exit(1);
    }
    node->cached = opCode;
    node->next = 0;

    if (fmt->opCode) {
	fmt->opCodeLast = fmt->opCodeLast->next = node;
    } else {
	fmt->opCode = fmt->opCodeLast = node;
    }
}

void
printOpcodeDef(FILE *out, const char *macroIdent, const char *instrRegId)
{
    printCode(out, 0, "#define %s ((%s) >> %zu)\n", macroIdent, instrRegId,
	      32 - sizeOpCode);
}

void
printFmtDef(FILE *out, const struct FmtNode *fmt, const char *instrRegId)
{
    for (const struct FmtFieldNode *f = fmt->field; f; f = f->next) {
	printCode(out, 0, "#define %s \t(((%s) (%s) << %zu) >> %zu)\n",
		  f->id->cstr, f->type == UNSIGNED ? "uint32_t" : "int32_t",
		  instrRegId, 32 - (f->bitPos + f->numBits),
		  32 - f->numBits - (f->type == JMP_OFFSET ? 2 : 0));
    }
}

void
printFmtUndef(FILE *out, const struct FmtNode *fmt)
{
    for (const struct FmtFieldNode *f = fmt->field; f; f = f->next) {
	printCode(out, 0, "#undef %s \n", f->id->cstr);
    }
}

void
printFmtInstrEncoding(FILE *out)
{
    const char *ft[NUM_FIELD_TYPES] = { "SIGNED", "UNSIGNED", "JMP_OFFSET" };
    static const struct UStr *opFieldId;
    static bool first = true;

    if (first) {
	opFieldId = makeUStr("OP");
	first = false;
    }

    printCode(out, 0, "uint32_t\n");
    printCode(out, 0,
	      "encodeInstr(enum CgSeg cgSeg, uint64_t addr, uint32_t opCode, "
	      "va_list vl)\n");
    printCode(out, 0, "{\n");
    printCode(out, 1, "uint32_t instr = 0;\n");
    printCode(out, 1, "switch (opCode) {\n");

    for (const struct FmtNode *n = fmtNode; n; n = n->next) {
	bool fmtUsed = false;
	for (const struct OpCodeNode *op = n->opCode; op; op = op->next) {
	    printCode(out, 3, "case 0x%02" PRIX32 ":\n", op->cached);
	    fmtUsed = true;
	}
	if (!fmtUsed) {
	    continue;
	}
	assert(n->field); // we need at least the OP field
	for (const struct FmtFieldNode *f = n->field; f; f = f->next) {
	    if (f->id == opFieldId) {
		// type is always UNSIGNED
		printCode(out, 3, "encodeOperand(&instr, %zu, %zu, opCode);\n",
			  f->bitPos, f->numBits);
	    } else {
		printCode(out, 3,
			  "encodeExprOperand(cgSeg, addr, &instr, %zu, %zu, "
			  "%s, va_arg(vl, struct Expr *));\n",
			  f->bitPos, f->numBits, ft[f->type]);
	    }
	}
	printCode(out, 2, "break;\n");
    }

    printCode(out, 2, "default:;\n");
    printCode(out, 1, "}\n");
    printCode(out, 0, "\n");
    printCode(out, 1, "return instr;\n");
    printCode(out, 0, "}\n");
}


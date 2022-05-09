#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "instr.h"
#include <utils/printcode.h>

struct InstrNode
{
    struct Instr
    {
	uint32_t opCode;
	const struct FmtNode *fmt;
	struct CodeNode
	{
	    const char *line;
	    struct CodeNode *next;
	} * codeNode, *codeNodeLast;

	struct AsmNode
	{
	    const char *notation, *copyOperands;
	    const char *refmanNotation;
	    const char *refmanMnemonic;
	    struct AsmNode *next;
	} * asmNode, *asmNodeLast;

    } instr;
    struct InstrNode *next;
} * instrNode, *instrNodeLast;

struct Instr *
makeInstr(uint32_t opCode)
{
    // check if opCode is already defined
    for (struct InstrNode *in = instrNode; in; in = in->next) {
	if (in->instr.opCode == opCode) {
	    error("opcode 0x%" PRIX32 "already defined\n", opCode);
	}
    }

    struct InstrNode *in = malloc(sizeof(*in));
    if (!in) {
	fprintf(stderr, "makeInstr: out of memory\n");
	exit(1);
    }
    in->instr.opCode = opCode;
    in->instr.fmt = 0;
    in->instr.codeNode = in->instr.codeNodeLast = 0;
    in->instr.asmNode = in->instr.asmNodeLast = 0;
    in->next = 0;

    if (instrNode) {
	instrNodeLast = instrNodeLast->next = in;
    } else {
	instrNode = instrNodeLast = in;
    }
    return &in->instr;
}

void
setInstrFmt(struct Instr *instr, const struct FmtNode *fmt)
{
    instr->fmt = fmt;
}

const struct FmtNode *
getInstrFmt(struct Instr *instr)
{
    return instr->fmt;
}

uint32_t
getInstrOpCode(const struct Instr *instr)
{
    return instr->opCode;
}

void
appendInstrCode(struct Instr *instr, const struct Str *code)
{
    char *line = strndup(code->cstr, code->end - code->cstr);
    struct CodeNode *codeNode = malloc(sizeof(*codeNode));
    if (!codeNode || !line) {
	fprintf(stderr, "appendInstrCode: out of memory\n");
	exit(1);
    }
    codeNode->line = line;
    codeNode->next = 0;

    if (instr->codeNode) {
	instr->codeNodeLast = instr->codeNodeLast->next = codeNode;
    } else {
	instr->codeNode = instr->codeNodeLast = codeNode;
    }
}

void
appendInstrAsmNotation(struct Instr *instr, const char *asmNotation,
		       const char *asmCopyOperands, const char *refmanMnemonic,
		       const char *refmanAsmNotation)
{
    assert(asmNotation);
    asmNotation = strdup(asmNotation);
    asmCopyOperands = strdup(asmCopyOperands);
    refmanMnemonic = strdup(refmanMnemonic);
    refmanAsmNotation = strdup(refmanAsmNotation);
    struct AsmNode *node = malloc(sizeof(*node));
    if (!node || !asmNotation || !refmanAsmNotation || !asmCopyOperands) {
	fprintf(stderr, "appendInstrAsmNotation: out of memory\n");
	exit(1);
    }
    node->notation = asmNotation;
    node->copyOperands = asmCopyOperands;
    node->refmanNotation = refmanAsmNotation;
    node->refmanMnemonic = refmanMnemonic;
    node->next = 0;

    if (instr->asmNode) {
	instr->asmNodeLast = instr->asmNodeLast->next = node;
    } else {
	instr->asmNode = instr->asmNodeLast = node;
    }
}

void
instrPrintInstrList(FILE *out)
{
    printOpcodeDef(out, "FMT_OPCODE", "ulm_instrReg");
    printCode(out, 1, "switch (FMT_OPCODE) {\n");
    printCode(out, 0, "#undef FMT_OPCODE\n");
    for (struct InstrNode *in = instrNode; in; in = in->next) {
	printCode(out, 2, "case 0x%02" PRIX32 ":\n", in->instr.opCode);
	printCode(out, 3, "{\n");
	printFmtDef(out, in->instr.fmt, "ulm_instrReg");
	for (struct CodeNode *cn = in->instr.codeNode; cn; cn = cn->next) {
	    if (!cn->next && *cn->line == 0) {
		break;
	    }
	    printCode(out, 3, "%s\n", cn->line);
	}
	printFmtUndef(out, in->instr.fmt);
	printCode(out, 3, "}\n");
	printCode(out, 3, "break;\n");
    }
    printCode(out, 2, "default:\n");
    printOpcodeDef(out, "FMT_OPCODE", "ulm_instrReg");
    printCode(out, 3, "illegalInstr(FMT_OPCODE);\n");
    printCode(out, 0, "#undef FMT_OPCODE\n");
    printCode(out, 1, "}\n");
}

void
instrPrintAsmNotation(FILE *out)
{
    printCode(out, 0, "void\n");
    printCode(out, 0, "ulm_asm(uint32_t instr, char *s, size_t len)\n");
    printCode(out, 0, "{\n");
    printCode(out, 1, "strncpy(s, \"%s\", len);\n", "");

    printOpcodeDef(out, "FMT_OPCODE", "instr");
    printCode(out, 1, "switch (FMT_OPCODE) {\n");
    printCode(out, 0, "#undef FMT_OPCODE\n");

    for (const struct InstrNode *in = instrNode; in; in = in->next) {
	printCode(out, 2, "case 0x%02" PRIX32 ": {\n", in->instr.opCode);

	bool alt = false;
	for (const struct AsmNode *an = in->instr.asmNode; an;
	     an = an->next, alt = true)
	{
	    if (!alt) {
		printFmtDef(out, in->instr.fmt, "instr");
		printCode(out, 3, "%s\n", an->copyOperands);
		printFmtUndef(out, in->instr.fmt);
		printCode(out, 3, "snprintf(s, len, %s);\n", an->notation);
	    } else {
		printCode(out, 3, "/* ignoring alternatives for now */\n");
		printCode(out, 3, "//snprintf(s, len, %s);\n", an->notation);
	    }
	}

	printCode(out, 3, "} break;\n");
    }
    printCode(out, 2, "default:\n");
    printCode(out, 3, "strncpy(s, \"%s\", len);\n", "illegal instruction");
    printCode(out, 1, "}\n");
    printCode(out, 0, "}\n");
}

void
instrPrintInstrRefman(FILE *out)
{
    for (const struct InstrNode *in = instrNode; in; in = in->next) {
	for (const struct AsmNode *an = in->instr.asmNode; an; an = an->next) {
	    printCode(out, 1,
		      "addDescription(\"%s\", 0x%02" PRIX32 ", \"%s\", %s);\n",
		      an->refmanMnemonic, in->instr.opCode,
		      getFmtId(in->instr.fmt), an->refmanNotation);
	}
	printCode(out, 1, "addImplementation(0x%02" PRIX32 ", [](){ \n",
		  in->instr.opCode);
	printCode(out, 2, "using namespace %s;\n", getFmtId(in->instr.fmt));
	for (struct CodeNode *cn = in->instr.codeNode; cn; cn = cn->next) {
	    if (!cn->next && *cn->line == 0) {
		break;
	    }
	    printCode(out, 1, "%s\n", cn->line);
	}
	printCode(out, 1, "});\n");
    }
}

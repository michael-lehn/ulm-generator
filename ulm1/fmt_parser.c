#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "asm.h"
#include "error.h"
#include "expected.h"
#include "fmt.h"
#include "fmt_parser.h"
#include "instr.h"
#include "lexer.h"
#include <utils/ustr.h>

static void parseFmtLayout();
static void parseInstrLayout();
static void parseAssemblyNotation(struct Instr *instr);
static void parseMnemonicDescription();
void
parseFmtDestroy()
{
    asmDestroy();
}

static size_t
tokenValToInteger()
{
    size_t val = 0;
    if (token.kind == DECIMAL_LITERAL) {
	if (sscanf(token.val.cstr, "%zu", &val) != 1) {
	    error(
	      "conversion of decimal literal '%s' to numerical value failed",
	      token.val.cstr);
	    assert(0);
	}
    } else if (token.kind == HEXADECIMAL_LITERAL) {
	if (sscanf(token.val.cstr, "%zx", &val) != 1) {
	    error("conversion of hexadecimal literal '%s' to numerical value "
		  "failed",
		  token.val.cstr);
	    assert(0);
	}
    }
    return val;
}

void
parseFmt()
{
    for (getToken(); token.kind != EOI;) {
	if (token.kind == EOL) {
	    getToken();
	    continue;
	}
	if (token.kind == IDENT) {
	    parseFmtLayout();
	} else if (token.kind == HEXADECIMAL_LITERAL) {
	    parseInstrLayout();
	} else if (token.kind == KLAMMERAFFE) {
	    parseMnemonicDescription();
	} else {
	    error("begin of format definition or begin of intruction "
		  "definition expected\n");
	}
    }
}

static void
parseFmtLayout()
{
    static struct UStr *unsignedType;
    static struct UStr *signedType;
    static struct UStr *jumpType;
    static bool first = true;

    if (first) {
	unsignedType = makeUStr("u");
	signedType = makeUStr("s");
	jumpType = makeUStr("j");
	first = false;
    }

    expected(IDENT);
    struct FmtNode *fmt = addFmt(makeUStr(token.val.cstr));
    assert(fmt);
    getToken();

    struct UStr *opFieldId = makeUStr("OP");
    bool hasOp = false;

    while (token.kind == LPAREN) {
	// field ident
	getToken();
	expected(IDENT);
	struct UStr *fieldId = makeUStr(token.val.cstr);

	if (fieldId == opFieldId) {
	    hasOp = true;
	}

	// field type ('u' for unsigend, 's' for signed, or 'j' for jump)
	getToken();
	expected(IDENT);
	struct UStr *type_ = makeUStr(token.val.cstr);
	enum FmtFieldType type = -1;
	if (type_ == unsignedType) {
	    type = UNSIGNED;
	} else if (type_ == signedType) {
	    type = SIGNED;
	} else if (type_ == jumpType) {
	    type = JMP_OFFSET;
	} else {
	    error("signed ('%s'), unsigned ('%s'), or jump offset ('%s') type "
		  "expected\n",
		  unsignedType->cstr, signedType->cstr, jumpType->cstr);
	}

	// field size in number of bits
	getToken();
	expected(DECIMAL_LITERAL);
	size_t numBits = tokenValToInteger();
	if (numBits == 0) {
	    error("bit width has to be at least 1\n");
	}
	appendFmtField(fmt, fieldId, numBits, type);

	// closing parenthesis
	getToken();
	expected(RPAREN);
	getToken();
    }
    if (! hasOp) {
	error("Format has no field with identifier 'OP'\n");
    }
    expected(EOL);
    getToken();
}

static void
parseInstrLayout()
{
    expected(HEXADECIMAL_LITERAL);

    uint32_t opCode = 0;
    if (sscanf(token.val.cstr, "%" PRIX32, &opCode) != 1) {
	assert(0);
    }
    struct Instr *instr = makeInstr(opCode);
    assert(instr);
    getToken();

    expected(IDENT);
    const struct UStr *fmtId = makeUStr(token.val.cstr);
    struct FmtNode *fmt = getFmt(fmtId);
    if (!fmt) {
	error("OpCode 0x%02" PRIX32 " has undefined format %s\n", opCode,
	      fmtId->cstr);
    }
    // instructions need to know about arguments (e.g. by calling getFmtFieldId)
    setInstrFmt(instr, fmt);
    // formats need to generate code for enconding instructions of given opCode
    addOpCodeToFmt(fmt, getInstrOpCode(instr));
    getToken();
    expected(EOL);
    getToken();

    const char *comment = getPendingComment();
    if (*comment) {
	appendInstrComment(instr, comment);
    }

    // parse assembly notation(s)
    if (token.kind == COLON) {
	parseAssemblyNotation(instr);
    }

    // parse code
    expected(CODE);
    while (token.kind == CODE || token.kind == EOL) {
	appendInstrCode(instr, &token.val);
	getToken();
    }
}

static void
parseAssemblyNotation(struct Instr *instr)
{
    static struct Str asmNotation;
    static struct Str asmOperands;
    static struct Str asmCopyOperands;
    static struct Str refmanAsmMnemonic;
    static struct Str refmanAsmNotation;

    uint32_t opCode = getInstrOpCode(instr);
    const struct FmtNode *fmt = getInstrFmt(instr);
    size_t numFields = getFmtNumFields(fmt);

    expected(COLON);
    while (token.kind == COLON) {
	getToken();
	size_t *exprIndex = calloc(numFields, sizeof(*exprIndex));

	clearStr(&asmNotation);
	clearStr(&asmOperands);
	clearStr(&asmCopyOperands);
	clearStr(&refmanAsmMnemonic);
	clearStr(&refmanAsmNotation);

	appendCharToStr(&asmNotation, '"');
	appendCharToStr(&refmanAsmNotation, '"');

	struct AsmNode *asmNode = 0;
	size_t numOperands = 0;
	for (bool first = true; token.kind != EOL; first = false) {
	    struct UStr *mnemonic = first ? makeUStr(token.val.cstr) : 0;
	    enum TokenKind edgeToken = token.kind;
	    size_t integerLiteral = tokenValToInteger();

	    if (!mnemonic && token.kind == IDENT) {
		const struct UStr *fieldId = makeUStr(token.val.cstr);
		const struct FmtFieldNode *field = getFmtField(fmt, fieldId);
		getToken();

		if (!field) {
		    error("instruction format for OpCode 0x%02" PRIX32
			  " has no field '%s'.\n",
			  opCode, fieldId->cstr);
		    exit(1);
		}

		++numOperands;
		size_t index = getFmtFieldIndex(field);
		integerLiteral = numOperands;

		if (index == 0) {
		    error("operand can not address the opcode field.\n");
		    exit(1);
		}
		exprIndex[index] = numOperands;

		appendCStrToStr(&asmOperands, ", ");
		appendCStrToStr(&asmNotation, "%\" PRId64 \"");
		appendCStrToStr(&asmOperands, fieldId->cstr);
		appendCStrToStr(&asmOperands, "_asmSuffix");

		appendCStrToStr(&asmCopyOperands, "int64_t ");
		appendCStrToStr(&asmCopyOperands, fieldId->cstr);
		appendCStrToStr(&asmCopyOperands, "_asmSuffix");
		appendCStrToStr(&asmCopyOperands, " = ");
		appendCStrToStr(&asmCopyOperands, fieldId->cstr);
		appendCStrToStr(&asmCopyOperands, "; ");

		appendCStrToStr(&refmanAsmNotation, fieldId->cstr);

	    } else {
		if (token.kind == PERCENT) {
		    appendCStrToStr(&asmNotation, "%%");
		    appendCStrToStr(&refmanAsmNotation, "%");
		} else {
		    appendStrToStr(&asmNotation, &token.val);
		    appendStrToStr(&refmanAsmNotation, &token.val);
		}
		if (first || token.kind == COMMA) {
		    appendCharToStr(&asmNotation, ' ');
		    appendCharToStr(&refmanAsmNotation, ' ');
		}
		getToken();
	    }

	    if (mnemonic) {
		appendCStrToStr(&refmanAsmMnemonic, mnemonic->cstr);
		asmNode = asmAddMnemonic(mnemonic);
		assert(asmNode);
	    } else {
		assert(asmNode);
		asmNode = asmAddEdge(asmNode, edgeToken, integerLiteral);
		if (!asmNode) {
		    error("internal error\n");
		}
	    }
	}
	assert(asmNode);
	asmNode = asmAccept(asmNode, opCode, numFields, exprIndex);
	if (!asmNode) {
	    error("already defined\n");
	}
	appendCharToStr(&asmNotation, '"');
	appendStrToStr(&asmNotation, &asmOperands);

	appendCharToStr(&refmanAsmNotation, '"');

	appendInstrAsmNotation(instr, asmNotation.cstr, asmCopyOperands.cstr,
			       refmanAsmMnemonic.cstr, refmanAsmNotation.cstr);
	getToken();
    }
}

static void
parseMnemonicDescription()
{
    expected(KLAMMERAFFE);
    getToken(); // consume '@'

    expected(IDENT);
    const struct UStr *ident = makeUStr(token.val.cstr);
    struct Loc identLoc = token.loc;
    getToken(); // consume IDENT

    expected(EOL);
    getToken(); // consume EOL

    const char *comment = getPendingComment();
    if (*comment) {
	asmAddMnemonicComment(ident, comment);
    } else {
	errorAt(identLoc, "expected subsequent comment");
    }
}

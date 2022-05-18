#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "utils/printcode.h"

struct EdgeNode
{
    enum TokenKind tokenKind;
    size_t literalValue;
    struct AsmNode *asmNode;

    struct EdgeNode *next;
};

struct AsmNode
{
    bool accepting;
    size_t acceptingIndex; // used if accepting == true
    size_t exprIndexDim;   // used if accepting == true
    size_t *exprIndex;	   // used if accepting == true

    uint32_t opCodeCached;

    struct EdgeNode *edge, *edgeLast;
};

static struct MnemonicListNode
{
    struct Mnemonic
    {
	const struct UStr *ident, *enumIdent, *key;
	struct AsmNode *asmNode;
	const struct Mnemonic *parseLike;
	size_t maxNumFields;
	const char *comment;

	struct AcceptingNode
	{
	    const struct AsmNode *acceptingAsmNode;
	    uint32_t opCodeCached;
	    struct AcceptingNode *next;
	} * accepting, *acceptingLast;
	size_t numAccepting;

    } mnemonic;

    struct MnemonicListNode *next;
} * mnemonicList, *mnemonicListLast;

static void
destroyAsmNode(struct AsmNode *asmNode)
{
    if (!asmNode) {
	return;
    }
    for (struct EdgeNode *e = asmNode->edge, *next = 0; e; e = next) {
	destroyAsmNode(e->asmNode);
	next = e->next;
	free(e);
    }
    free(asmNode);
}

static void
destroyAcceptingNode(struct AcceptingNode *accepting)
{
    for (struct AcceptingNode *a = accepting, *next = 0; a; a = next) {
	next = a->next;
	free(a);
    }
}

void
asmDestroy()
{
    for (struct MnemonicListNode *n = mnemonicList, *next = 0; n; n = next) {
	destroyAcceptingNode(n->mnemonic.accepting);
	destroyAsmNode(n->mnemonic.asmNode);
	next = n->next;
	free(n);
    }
    mnemonicList = mnemonicListLast = 0;
}

static const char *
makeUpperCase(const char *str)
{
    static struct Str buf;

    makeEmptyStr(&buf);
    for (; *str; ++str) {
	appendCharToStr(&buf, toupper(*str));
    }
    return buf.cstr;
}

struct Mnemonic *
addMnemonic(const struct UStr *ident)
{
    for (struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	if (n->mnemonic.ident == ident) {
	    return &n->mnemonic;
	}
    }

    struct MnemonicListNode *node = calloc(1, sizeof(*node));
    struct AsmNode *asmNode = calloc(1, sizeof(*asmNode));
    if (!node || !asmNode) {
	fprintf(stderr, "addMnemonic: out of memory\n");
	exit(1);
    }
    node->mnemonic.ident = ident;
    node->mnemonic.enumIdent = makeUStr(makeUpperCase(ident->cstr));
    node->mnemonic.asmNode = asmNode;

    if (mnemonicList) {
	mnemonicListLast = mnemonicListLast->next = node;
    } else {
	mnemonicList = mnemonicListLast = node;
    }
    return &node->mnemonic;
}

struct AsmNode *
asmAddMnemonic(const struct UStr *ident)
{
    return addMnemonic(ident)->asmNode;
}

void
asmAddMnemonicComment(const struct UStr *ident, const char *comment)
{
    assert(comment);
    comment = strdup(comment);
    if (!comment) {
	fprintf(stderr, "asmAddMnemonicComment: out of memory\n");
	exit(1);
    }

    struct Mnemonic *mnem = addMnemonic(ident);
    assert(mnem);
    mnem->comment = comment;
}

struct AsmNode *
asmAddEdge(struct AsmNode *asmNode, enum TokenKind tokenKind,
	   size_t literalValue)
{
    for (struct EdgeNode *e = asmNode->edge; e; e = e->next) {
	if (e->tokenKind == tokenKind && e->literalValue == literalValue) {
	    return e->asmNode;
	}
    }
    struct EdgeNode *edge = calloc(1, sizeof(*edge));
    struct AsmNode *asmFollowNode = calloc(1, sizeof(*asmFollowNode));
    if (!edge || !asmFollowNode) {
	fprintf(stderr, "asmAddMnemonic: out of memory\n");
	exit(1);
    }

    edge->tokenKind = tokenKind;
    edge->literalValue = literalValue;
    edge->asmNode = asmFollowNode;

    // sorted insert of new node 'edge'
    bool inserted = false;
    for (struct EdgeNode *e = asmNode->edge, *p = 0; e; p = e, e = e->next) {
	if (e->tokenKind > edge->tokenKind ||
	    (e->tokenKind == edge->tokenKind &&
	     e->literalValue > edge->literalValue))
	{
	    // insert new node 'edge' before node 'e'
	    edge->next = e;
	    if (p) {
		// now node 'edge' becomes sucessor of previous node
		p->next = edge;
	    } else {
		// otherwise new node 'edge' is new head of the list
		asmNode->edge = edge;
	    }
	    inserted = true;
	    break;
	}
    }
    if (!inserted) {
	// append ...
	if (asmNode->edge) {
	    // ... to a not-empty list
	    asmNode->edgeLast = asmNode->edgeLast->next = edge;
	} else {
	    // ... to an empty list
	    asmNode->edgeLast = asmNode->edge = edge;
	}
    }
    return edge->asmNode;
}

struct AsmNode *
asmAccept(struct AsmNode *asmNode, uint32_t opCode, size_t exprIndexDim,
	  size_t *exprIndex)
{
    // accepting requires that next token is an EOL:
    struct AsmNode *node = asmAddEdge(asmNode, EOL, 0);

    // check if node was actually created. Exiting nodes have to be accepting
    // nodes.
    if (node->accepting) {
	fprintf(stderr,
		"previous opcode 0x%02" PRIX32 ", now 0x%02" PRIX32 "\n",
		node->opCodeCached, opCode);
	return 0;
    }
    node->accepting = true;
    node->opCodeCached = opCode;
    node->exprIndexDim = exprIndexDim;
    node->exprIndex = exprIndex;
    return node;
}

static const char *
strTokenKind_patch(enum TokenKind tokenKind)
{
    if (tokenKind == LPAREN) {
	return "LPAREN_MEM";
    }
    return strTokenKind(tokenKind);
}

static void
printEdge(FILE *out, size_t indent, enum TokenKind tokenKind,
	  size_t literalValue)
{
    switch (tokenKind) {
	case DECIMAL_LITERAL:
	case HEXADECIMAL_LITERAL:
	    printCode(out, indent,
		      "expectedIntegerLiteral(%zu); commentAddLiteral(%zu); "
		      "getToken();\n",
		      literalValue, literalValue);
	    break;
	case IDENT:
	    printCode(out, indent, "if (!(expr[%zu] = parseExpression())) {\n",
		      literalValue);
	    printCode(out, indent + 1, "error(\"expression expected\\n\");\n");
	    printCode(out, indent + 1, "return false;\n");
	    printCode(out, indent, "}\n");
	    printCode(out, indent, "commentAddExpr(expr[%zu]);\n",
		      literalValue);
	    break;
	default:
	    printCode(out, indent,
		      "expected(%s); commentAddCurrentToken(); getToken();\n",
		      strTokenKind_patch(tokenKind));
    }
}

static void
printBranch(FILE *out, size_t indent, bool firstCase, enum TokenKind tokenKind,
	    size_t literalValue)
{
    const char *branchStr = firstCase ? "if" : "} else if";
    switch (tokenKind) {
	case DECIMAL_LITERAL:
	case HEXADECIMAL_LITERAL:
	    printCode(out, indent, "%s (integerLiteral(%zu)) {\n", branchStr,
		      literalValue);
	    printCode(out, indent + 1,
		      "commentAddCurrentToken(); getToken();\n");
	    break;
	case IDENT:
	    printCode(out, indent, "%s ((expr[%zu] = parseExpression())) {\n",
		      branchStr, literalValue);
	    printCode(out, indent + 1, "commentAddExpr(expr[%zu]);\n",
		      literalValue);
	    break;
	default:
	    printCode(out, indent, "%s (token.kind == %s) {\n", branchStr,
		      strTokenKind_patch(tokenKind));
	    printCode(out, indent + 1,
		      "commentAddCurrentToken(); getToken();\n");
    }
}

static void
printBranchEnd(FILE *out, size_t indent, const struct EdgeNode *expected)
{
    printCode(out, indent, "} else {\n");
    printCode(out, indent + 1, "error(\"expected: ");
    for (const struct EdgeNode *e = expected; e; e = e->next) {
	switch (e->tokenKind) {
	    case DECIMAL_LITERAL:
	    case HEXADECIMAL_LITERAL:
		printCode(out, 0, "'%zu'", e->literalValue);
		break;
	    case IDENT:
		printCode(out, 0, "expression");
		break;
	    default:
		printCode(out, 0, "'%s'", valTokenKindEscaped(e->tokenKind));
	}
	if (e->next) {
	    if (e->next->next) {
		printCode(out, 0, ", ");
	    } else {
		printCode(out, 0, " or ");
	    }
	}
    }
    printCode(out, 0, "\\n\");\n");
    printCode(out, indent + 1, "return false;\n");
    printCode(out, indent, "}\n");
}

void
printParseCode(FILE *out, size_t indent, const struct Mnemonic *mnemonic,
	       const struct AsmNode *asmNode)
{
    if (asmNode->edge) {
	if (asmNode->edge == asmNode->edgeLast) {
	    // just one edge
	    printEdge(out, indent, asmNode->edge->tokenKind,
		      asmNode->edge->literalValue);
	    printParseCode(out, indent, mnemonic, asmNode->edge->asmNode);
	} else {
	    // branch
	    for (const struct EdgeNode *e = asmNode->edge; e; e = e->next) {
		printBranch(out, indent, e == asmNode->edge, e->tokenKind,
			    e->literalValue);
		printParseCode(out, indent + 1, mnemonic, e->asmNode);
	    }
	    printBranchEnd(out, indent, asmNode->edge);
	}
    }
    if (asmNode->accepting) {
	printCode(out, indent, "cgAppendInstr(opCode%zu",
		  asmNode->acceptingIndex);
	for (size_t i = 1; i < asmNode->exprIndexDim; ++i) {
	    printCode(out, 0, ", expr[%zu]", asmNode->exprIndex[i]);
	}
	printCode(out, 0, ");\n");
	printCode(out, indent, "return true;\n");
    }
}

static size_t
getMaxNumFields(const struct AsmNode *asmNode)
{
    size_t max = asmNode->accepting ? asmNode->exprIndexDim : 0;
    for (const struct EdgeNode *e = asmNode->edge; e; e = e->next) {
	size_t val = getMaxNumFields(e->asmNode);
	max = max < val ? val : max;
    }
    return max;
}

static void
printExprDecl(FILE *out, const struct Mnemonic *mnemonic)
{
    printCode(out, 1, "struct Expr *expr[%zu] = { ", mnemonic->maxNumFields);
    for (size_t i = 1; i <= mnemonic->maxNumFields; ++i) {
	printCode(out, 0, "0, ");
    }
    printCode(out, 0, "};\n");
    printCode(out, 1, "expr[0] = expr[0]; // prevent warning if unused\n");
}

static void
getParseCodeKey_(struct Str *key, const struct AsmNode *asmNode)
{
    static char buf[10];

    bool hasBranch = asmNode->edge != asmNode->edgeLast;
    if (hasBranch) {
	appendCStrToStr(key, "[");
    }
    for (struct EdgeNode *e = asmNode->edge; e; e = e->next) {
	switch (e->tokenKind) {
	    case DECIMAL_LITERAL:
	    case HEXADECIMAL_LITERAL:
		snprintf(buf, sizeof(buf), "%zu", e->literalValue);
		appendCStrToStr(key, buf);
		break;
	    case IDENT:
		appendCStrToStr(key, "<expr>");
		break;
	    case EOL:
		appendCStrToStr(key, "\\n");
		break;
	    default:
		appendCStrToStr(key, valTokenKind(e->tokenKind));
	}
	getParseCodeKey_(key, e->asmNode);
	if (e->next) {
	    appendCStrToStr(key, "|");
	}
    }
    if (asmNode->accepting) {
	appendCStrToStr(key, "{");
	for (size_t i = 1; i < asmNode->exprIndexDim; ++i) {
	    snprintf(buf, sizeof(buf), "%zu", asmNode->exprIndex[i]);
	    appendCStrToStr(key, buf);
	    if (i + 1 < asmNode->exprIndexDim) {
		appendCStrToStr(key, ",");
	    }
	}
	appendCStrToStr(key, "}");
    }
    if (hasBranch) {
	appendCStrToStr(key, "]");
    }
}

static const char *
getParseCodeKey(const struct AsmNode *asmNode)
{
    static struct Str key;

    makeEmptyStr(&key);
    getParseCodeKey_(&key, asmNode);
    return key.cstr;
}

static void
makeOpCodeList(struct Mnemonic *mnemonic, struct AsmNode *asmNode)
{
    if (asmNode->accepting) {
	struct AcceptingNode *node = malloc(sizeof(*node));
	if (!node) {
	    fprintf(stderr, "makeOpCodeList: out of memory\n");
	    exit(1);
	}
	asmNode->acceptingIndex = ++mnemonic->numAccepting;
	node->acceptingAsmNode = asmNode;
	node->opCodeCached = asmNode->opCodeCached;
	node->next = 0;

	// append node
	if (mnemonic->accepting) {
	    mnemonic->acceptingLast = mnemonic->acceptingLast->next = node;
	} else {
	    mnemonic->accepting = mnemonic->acceptingLast = node;
	}
    }
    for (struct EdgeNode *e = asmNode->edge; e; e = e->next) {
	makeOpCodeList(mnemonic, e->asmNode);
    }
}

static const char *
strOpCodeParamList(const struct Mnemonic *mnemonic)
{
    static struct Str param;
    static char buf[20];

    size_t count = 0;
    makeEmptyStr(&param);
    for (const struct AcceptingNode *a = mnemonic->accepting; a; a = a->next) {
	snprintf(buf, sizeof(buf), "uint32_t opCode%zu", ++count);
	appendCStrToStr(&param, buf);
	if (a->next) {
	    appendCStrToStr(&param, ", ");
	}
    }
    return param.cstr;
}

static const char *
strOpCodeList(struct Mnemonic *mnemonic)
{
    static struct Str param;
    static char buf[20];

    makeEmptyStr(&param);
    for (struct AcceptingNode *a = mnemonic->accepting; a; a = a->next) {
	snprintf(buf, sizeof(buf), "0x%02" PRIX32, a->opCodeCached);
	appendCStrToStr(&param, buf);
	if (a->next) {
	    appendCStrToStr(&param, ", ");
	}
    }
    return param.cstr;
}

static const struct Mnemonic *
findMnemonic(const struct UStr *key)
{
    for (struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	if (n->mnemonic.key == key) {
	    return &n->mnemonic;
	}
    }
    return 0;
}

static void
printParseInstructionFunction(FILE *out)
{
    printCode(out, 0, "bool\n");
    printCode(out, 0, "parseInstruction(void)\n");
    printCode(out, 0, "{\n");
    printCode(out, 1, "bool ok = false;\n");
    printCode(out, 1,
	      "if (MNEMONIC_FIRST == MNEMONIC_END || token.kind < "
	      "MNEMONIC_FIRST || token.kind >= MNEMONIC_END) {\n");
    printCode(out, 2, "return false;\n");
    printCode(out, 1, "}\n");
    printCode(out, 1, "commentClear(); commentAddCurrentToken();\n");
    printCode(out, 1, "enum TokenKind mnem = token.kind; getToken();\n");
    printCode(out, 1, "switch(mnem) {\n");

    for (struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	printCode(out, 2, "case %s:\n", n->mnemonic.enumIdent->cstr);
	printCode(out, 3, "ok = parse_like_%s(%s);\n",
		  n->mnemonic.asmNode ? n->mnemonic.ident->cstr
				      : n->mnemonic.parseLike->ident->cstr,
		  strOpCodeList(&n->mnemonic));
	printCode(out, 3, "break;\n");
    }

    printCode(out, 2, "default:\n");
    printCode(out, 3, "assert(0);\n");
    printCode(out, 1, "}\n");
    printCode(out, 1, "cgAppendComment(commentGetCStr());\n");
    printCode(out, 1, "return ok;\n");
    printCode(out, 0, "}\n");
}

void
asmPrintParseFunctions(FILE *out)
{
    // From this point on no further grammer rule gets added. So finalize rules
    // for parsing an instruction starting with its mnemonic:
    // - compute a key for each parser
    // - if such a key already exists reuse the parser
    // -
    for (struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	makeOpCodeList(&n->mnemonic, n->mnemonic.asmNode);
	if (!n->mnemonic.key) {
	    bool added;
	    const char *key = getParseCodeKey(n->mnemonic.asmNode);
	    n->mnemonic.key = makeUStr_(key, &added);
	    n->mnemonic.maxNumFields = getMaxNumFields(n->mnemonic.asmNode);
	    if (!added) {
		destroyAsmNode(n->mnemonic.asmNode);
		n->mnemonic.asmNode = 0;
		n->mnemonic.parseLike = findMnemonic(n->mnemonic.key);
	    }
	}
    }
    for (const struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	if (!n->mnemonic.asmNode) {
	    continue;
	}

	printCode(out, 0, "static bool\n");
	printCode(out, 0, "parse_like_%s(%s)\n", n->mnemonic.ident->cstr,
		  strOpCodeParamList(&n->mnemonic));
	printCode(out, 0, "{\n");

	printCode(out, 1, "// key: %s\n", n->mnemonic.key->cstr);

	printExprDecl(out, &n->mnemonic);
	printParseCode(out, 1, &n->mnemonic, n->mnemonic.asmNode);
	// printCode(out, 1, "return false; // never reached\n");
	printCode(out, 0, "}\n");
	printCode(out, 0, "\n");
    }
    printParseInstructionFunction(out);
}

void
asmPrintMnemonicList(FILE *out)
{
    for (struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	fprintf(out, "%s %s\n", n->mnemonic.enumIdent->cstr,
		n->mnemonic.ident->cstr);
    }
}

void
asmPrintMnemonicDescription(FILE *out)
{
    for (struct MnemonicListNode *n = mnemonicList; n; n = n->next) {
	if (n->mnemonic.comment) {
	    printCode(out, 1,
		      "ulmDoc.addMnemonicDescription(\"%s\", \"%s\");\n",
		      n->mnemonic.ident->cstr, n->mnemonic.comment);
	}
    }
}

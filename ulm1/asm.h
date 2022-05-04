#ifndef ULM1_ASMGRAMMAR_H
#define ULM1_ASMGRAMMAR_H

#include <stdbool.h>
#include <stdint.h>

#include "lexer.h"
#include "utils/ustr.h"

struct AsmNode;

struct AsmNode *asmAddMnemonic(const struct UStr *ident);
void asmAddMnemonicComment(const struct UStr *ident, const char *comment);
void asmDestroy(); // release all memory
struct AsmNode *asmAddEdge(struct AsmNode *asmNode, enum TokenKind tokenKind,
			   size_t literalValue);
struct AsmNode *asmAccept(struct AsmNode *asmNode, uint32_t opCode,
			  size_t exprIndexDim, size_t *exprIndex);
void asmPrintParseFunctions(FILE *out);
void asmPrintMnemonicList(FILE *out);
void asmPrintMnemonicDescription(FILE *out);

#endif // ULM1_ASMGRAMMAR_H

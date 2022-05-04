#ifndef ULM1_INSTR_H
#define ULM1_INSTR_H

#include <stdint.h>
#include <stdio.h>

#include "fmt.h"
#include <utils/str.h>

struct Instr;

struct Instr *makeInstr(uint32_t opCode);
void setInstrFmt(struct Instr *instr, const struct FmtNode *fmt);
const struct FmtNode *getInstrFmt(struct Instr *instr);
uint32_t getInstrOpCode(const struct Instr *instr);

void appendInstrCode(struct Instr *instr, const struct Str *code);
void appendInstrAsmNotation(struct Instr *instr, const char *asmNotation,
			    const char *asmCopyOperands,
			    const char *refmanMnemonic,
			    const char *refmanAsmNotation);

void instrPrintInstrList(FILE *out);
void instrPrintAsmNotation(FILE *out);
void instrPrintInstrRefman(FILE *out);

#endif // ULM1_INSTR_H

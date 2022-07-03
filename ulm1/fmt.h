#ifndef ULM1_FMT_H
#define ULM1_FMT_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <utils/ustr.h>

enum FmtFieldType
{
    SIGNED,
    UNSIGNED,
    JMP_OFFSET,
    NUM_FIELD_TYPES,
};

struct FmtNode;
struct FmtFieldNode;

struct FmtNode *addFmt(const struct UStr *fmtId);
struct FmtNode *getFmt(const struct UStr *fmtId);
const char *getFmtId(const struct FmtNode *fmt);
void appendFmtField(struct FmtNode *fmt, const struct UStr *fieldId,
		    size_t numBits, enum FmtFieldType type);

size_t getFmtNumFields(const struct FmtNode *fmt);
const struct FmtFieldNode *getFmtField(const struct FmtNode *fmt,
				       const struct UStr *fieldId);
const char *getFmtFieldId(const struct FmtFieldNode *field);
size_t getFmtFieldIndex(const struct FmtFieldNode *field);

void printOpcodeDef(FILE *out, const char *macroIdent, const char *instrRegId);
void printFmtDef(FILE *out, const struct FmtNode *fmt, const char *instrRegId);
void printFmtUndef(FILE *out, const struct FmtNode *fmt);

void printRefmanFieldsDescription(FILE *out);
void printRefmanFormatDescription(FILE *out);

// adding an opCode to an format defines its encoding
void addOpCodeToFmt(struct FmtNode *fmt, uint32_t opCode);
void printFmtInstrEncoding(FILE *out);

#endif // ULM1_FMT_H

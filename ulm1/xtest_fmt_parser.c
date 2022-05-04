#include <stdio.h>

#include "asm.h"
#include "fmt.h"
#include "fmt_parser.h"
#include "instr.h"
#include "lexer.h"

int
main(int argc, char **argv)
{
    enum
    {
	IN_FMT_TXT,
	FMT_ENCODE_C,
	INSTR_C,
	INSTR_ASM_C,
	REF_FIELDS_CPP,
	REF_FORMAT_CPP,
	REF_INSTR_CPP,
	MNEM_TOKEN,
	ASM_PARSE,
	NUM_IO,
    };

    if (argc != NUM_IO + 1) {
	fprintf(stderr,
		"usage: %s is.txt fmt_instr_encoding.c instr.c "
		"instr_asm.c refman_fields.cpp refman_format.cpp"
		"refman_instr.cpp mnemonic_tokens.txt parse_functions\n",
		argv[0]);
	return 1;
    }

    FILE *fd[NUM_IO];
    for (size_t i = 0; i < NUM_IO; ++i) {
	fd[i] = fopen(argv[i + 1], i == 0 ? "r" : "w");
	if (!fd[i]) {
	    fprintf(stderr, "can not open %s file '%s'\n",
		    i == 0 ? "input" : "output", argv[i + 1]);
	    return 1;
	}
    }
    setLexerIn(fd[IN_FMT_TXT], argv[IN_FMT_TXT + 1]);
    parseFmt();
    printFmtInstrEncoding(fd[FMT_ENCODE_C]);
    printRefmanFieldsDescription(fd[REF_FIELDS_CPP]);
    printRefmanFormatDescription(fd[REF_FORMAT_CPP]);
    instrPrintInstrList(fd[INSTR_C]);
    instrPrintAsmNotation(fd[INSTR_ASM_C]);
    instrPrintInstrRefman(fd[REF_INSTR_CPP]);
    asmPrintMnemonicDescription(fd[REF_INSTR_CPP]);

    asmPrintParseFunctions(fd[ASM_PARSE]);
    asmPrintMnemonicList(fd[MNEM_TOKEN]);

    parseFmtDestroy();
    for (size_t i = 0; i < NUM_IO; ++i) {
	fclose(fd[i]);
    }
}

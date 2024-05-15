$(this).requires.lib := \
	utils/libutils.a

# list of required generated files are relative to the top of build dir:
$(this).requires.gen := \
	ulm1/_gen_parseinstruction.c \
	ulm1/_gen_fmt_instr_encoding.c \
	ulmas0/_gen_scanner_mnem.c \
	ulmas0/_gen_tokenkind.h \
	ulmas0/_gen_strtokenkind.c

$(this).prg.c := \
 	ulmas.c \
	xtest_lexer.c

$(this).install := \
 	ulmas

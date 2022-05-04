$(this).requires.gen := \
	ulm1/_gen_instr.c \
	ulm1/_gen_fmt_testcase.c

$(this).requires.lib := ulm2/libulm2.a

$(this).prg.c := \
 	ulm.c

$(this).install := \
 	ulm

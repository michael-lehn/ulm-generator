$(this).requires.lib := \
    utils/libutils.a

# list of required generated files are relative to the top of build dir:
$(this).requires.gen := \
    ulm0/_gen_strtokenkind.c \
    ulm0/_gen_valtokenkind.c \
    ulm0/_gen_tokenkind.h

$(this).prg.c := \
    xtest_fmt_parser.c \
    xtest_fmt_check_parser.c

$(this).xtest_fmt_parser.in := \
    $$(variant_dir)isa.txt
$(this).xtest_fmt_parser.out := \
    _gen_fmt_instr_encoding.c \
    _gen_instr.c \
    _gen_instr_asm.c \
    _gen_refman_fields.cpp \
    _gen_refman_format.cpp \
    _gen_refman_instr.cpp \
    _gen_mnemonic.txt \
    _gen_parseinstruction.c

$(this).requires.lib := \
    utils/libutils.a

$(this).prg.c := \
    gen_scanner_parts.c

$(this).gen_scanner_parts.in := \
    $$(build_dir.top)ulm1/_gen_mnemonic.txt \
    $$(src_dir)directive.txt \
    $$(src_dir)other_token.txt

$(this).gen_scanner_parts.out := \
    _gen_scanner_mnem.c \
    _gen_tokenkind.h \
    _gen_strtokenkind.c

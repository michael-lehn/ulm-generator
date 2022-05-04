$(this).requires.lib := utils/libutils.a

# All files in source directory beginning with 'xtest' are programs. Here we
# enlist additional program files in the source directory:
$(this).prg.c := gen_tokenkind.c

# input file require explicit path, e.g.:
#   $$(src_dir),
#   $$(variant_dir),
#   $$(build_dir)
#   $$(build_dir.top)some_other_module
#
$(this).gen_tokenkind.in := \
    $$(src_dir)token.txt

# output file will be created in build directory
$(this).gen_tokenkind.out := \
    _gen_tokenkind.h \
    _gen_strtokenkind.c \
    _gen_valtokenkind.c

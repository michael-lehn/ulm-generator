$(this).requires.gen := \
	ulm1/_gen_refman_format.cpp \
	ulm1/_gen_refman_instr.cpp

$(this).prg.cpp := \
 	ulmrefman.cpp

$(this).ulmrefman.in := \
    $$(src_dir)tex/main.tex

$(this).ulmrefman.out := \
    refman.tex


$(this).has.extra := 1

# $1	(src_dir)
# $2	(build_dir)  
# $3	(install_dir)  
define $(this).extra
$(eval REFMAN += $3refman.pdf)

$2% : $1tex/%
	ln -f $$< $$@

$2refman.pdf : $2refman.tex $2by-sa.pdf $2refman.cls $2title.tex
	(cd $2; lualatex refman.tex) 
	(cd $2; lualatex refman.tex) 

$3refman.pdf : $2refman.pdf
	ln -f $$< $$@
endef

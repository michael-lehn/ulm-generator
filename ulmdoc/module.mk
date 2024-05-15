$(this).requires.gen := \
	ulm1/_gen_refman_format.cpp \
	ulm1/_gen_refman_instr.cpp

$(this).prg.cpp := \
 	ulmrefman.cpp

$(this).ulmrefman.in := \
    $$(variant_isa) \
    $$(src_dir)tex/main.tex

$(this).ulmrefman.out := \
    refman.tex


$(this).has.extra := 1

# $1	(src_dir)
# $2	(build_dir)  
# $3	(install_dir)  
# $4	(variant)  
define $(this).extra
$(eval REFMAN += $3refman.pdf)
$(eval REFMAN.theon += $3refman-theon.pdf)

$2% : $1tex/%
	ln -f $$< $$@

$2refman.pdf : $2refman.tex $2by-sa.pdf $2refman.cls $2title.tex
	(cd $2; lualatex refman.tex) 
	(cd $2; lualatex refman.tex) 

$3refman.pdf : $2refman.pdf
	ln -f $$< $$@

$2refman-theon.pdf : $2refman.tex $2by-sa.pdf $2refman.cls $2title.tex
	rsync -r $2 theon:_tmp_ulm/$(shell echo $$USER)_refman/$(strip $4)
	ssh theon 'cd _tmp_ulm/$(shell echo $$USER)_refman/$(strip $4)/; lualatex refman.tex'
	ssh theon 'cd _tmp_ulm/$(shell echo $$USER)_refman/$(strip $4)/; lualatex refman.tex'
	scp theon:_tmp_ulm/$(shell echo $$USER)_refman/$(strip $4)/refman.pdf $2refman-theon.pdf

$3refman-theon.pdf : $2refman-theon.pdf
	ln -f $$< $$@


endef

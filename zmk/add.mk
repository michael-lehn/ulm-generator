#in:	$1	(file, e.g. '1_ulm_build/release/build.mk')
#out:	$2	(name of identifier list, e.g BUILD )

# effect:
# $1.dir	(directory of file, e.g. 'build/release/')

define add
$(eval add.dir := $(dir $1))
$(eval add.ident := $(lastword $(subst /, ,$(add.dir))))
$(eval add.found := $(filter $(add.ident),$($2)))
$(if $(add.found),\
    $(error "Name conflict: Multiple defintions of '$(add.found)'."))

$(eval this := $(add.ident))
$(eval this.dir := $(add.dir))
$(eval include $1)

$(if $($(this).ignore),\
    $(info module '$(this)' ignored),\
    $(eval $2 += $(add.ident)) \
)

$(eval $(add.ident).dir := $(add.dir))
endef

#in:	$1	(file, e.g. '0_ulm_variants/fpga/isa.txt')

# effect:
# $1.dir	(directory of file, e.g. 'build/release/')

define add_isa
$(eval add.dir := $(dir $1))
$(eval add.ident := $(lastword $(subst /, ,$(add.dir))))
$(eval add.found := $(filter $(add.ident),$(VARIANT)))
$(if $(add.found),\
    $(error "Name conflict: Multiple defintions of '$(add.found)'."))

$(eval this := $(add.ident))
$(eval this.dir := $(add.dir))

$(if $($(this).ignore),\
    $(info module '$(this)' ignored),\
    $(eval VARIANT += $(add.ident)) \
)

$(eval $(add.ident).dir := $(add.dir))
endef

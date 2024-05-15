include zmk/add.mk
include zmk/setup_build.mk
include zmk/setup_file_lists.mk

#.NOTPARALLEL:

RANLIB := ranlib
INSTALL :=
TARGET :=
OPT_TARGET :=
DEP_FILES :=
REFMAN :=
EXTRA_DIRS :=

BUILD :=
VARIANT :=
MODULE :=

ifneq ($(isa),)
$(call add_isa,$(isa))
else
variant_list := \
	$(sort $(wildcard *.isa) $(wildcard */*.isa))

$(foreach v,$(variant_list),\
    $(call add_isa,$v))
endif

ifneq ($(build),)
$(call add,$(build),BUILD)
else
$(foreach b,$(wildcard */build.mk */*/build.mk),\
    $(call add,$b,BUILD))
endif

$(foreach m,$(wildcard */module.mk */*/module.mk),\
    $(call add,$m,MODULE))

$(foreach b,$(BUILD),\
    $(eval $(call setup_build,$b)))

$(foreach m,$(MODULE),\
    $(foreach v,$(VARIANT),\
	$(foreach b,$(BUILD),\
	    $(eval $(call setup_file_lists,$m,$v,$b)))))

#$(call mk0_info)

#
# For installing a variant
#
default.build := $(build)
default.variant := $(isa)

-include build/default
ifneq ($(words $(BUILD)),1)
	ifeq ($(strip $(build)),)
		install.target := install_select_build
		install.build := $(firstword $(BUILD))
	else
		install.build := $(build)
	endif
else
	install.build := $(BUILD)
	default.build := $(BUILD)
endif

ifneq ($(words $(VARIANT)),1)
	ifeq ($(strip $(isa)),)
		install.target := install_select_isa
	else
		ifeq ($(filter $(isa),$(VARIANT)),)
$(info error: unknown isa '$(isa)')
$(info select isa from: $(VARIANT))
$(error )
		endif
		install.variant := $(notdir $(basename $(isa)))
	endif
else
	install.variant := $(VARIANT)
	default.variant := $(VARIANT)
endif

ifeq ($(install.target),)
	install.target := install-$(install.variant)
endif

default.target := default-$(install.variant)
ifeq ($(default.variant),)
	default.target := default_select_isa
endif
ifeq ($(default.build),)
	default.build := default_select_build
endif

ifeq ($(strip $(prefix)),)
	install.prefix := /usr/local
else
	install.prefix := $(prefix)
endif

$(info install.prefix=$(install.prefix))

install_select_build:
	@echo "select build to install from: $(BUILD)"
	@echo "select isa to install from: $(VARIANT)"
	@echo "usage: make install isa=<variant>.isa build=<build>"

install_select_isa:
	@echo "select isa to install from: $(VARIANT)"
	@echo "usage: make install isa=<variant>.isa"

install-$(install.variant): \
		$(install.build)/$(install.variant)/ulm \
		$(install.build)/$(install.variant)/ulmas \
		$(install.build)/$(install.variant)/udb-tui
	cp $(install.build)/$(install.variant)/ulm $(install.prefix)
	cp $(install.build)/$(install.variant)/ulmas $(install.prefix)
	cp $(install.build)/$(install.variant)/udb-tui $(install.prefix)

default_select_build:
	@echo "select build from: $(BUILD)"
	@echo "select isa from: $(VARIANT)"
	@echo "usage: make default isa=<variant> build=<build>"

default_select_isa:
	@echo "select isa from: $(VARIANT)"
	@echo "usage: make default isa=<variant>"



default-$(install.variant):
	echo "isa=$(default.variant)" > build/default
	echo "build=$(default.build)" >> build/default

install.list := \
	Makefile \
	$(wildcard *.isa) \
	config \
	udb-tui \
	ulm0 \
	ulm1 \
	ulm2 \
	ulm3 \
	ulmas0 \
	ulmas1 \
	ulmdoc \
	utils \
	zmk

patch_ulm_generator := \
	sed "s\#FIXME\#prefix=$(install.prefix)/\#" \
		./ulm-generator.in \
		> $(install.prefix)/bin/ulm-generator

install:
	@mkdir -p $(install.prefix)/bin
	@mkdir -p $(install.prefix)/share/ulm-generator
	$(patch_ulm_generator)
	@chmod +x $(install.prefix)/bin/ulm-generator
	cp -r $(install.list) $(install.prefix)/share/ulm-generator

	
default: $(default.target)
	

.DEFAULT_GOAL := all
.PHONY: all
all: $(TARGET) $(install)

opt: $(OPT_TARGET)

info:
	@echo "VARIANT=$(VARIANT)"
	@echo "BUILD=$(BUILD)"
	@echo "MODULE=$(MODULE)"
	@echo "default=$(default)"

clean:
	$(RM) $(CLEAN_LIST)
	$(RM) $(REFMAN)
	$(RM) $(INSTALL)
	$(RM) -r $(CLEAN_DIRS)

# create additionally needed directories
$(sort $(EXTRA_DIRS)): ; @mkdir -p $@

# include all dependency files. Suppress error if they don't exist (yet)
$(DEP_FILES):
-include $(wildcard $(DEP_FILES))

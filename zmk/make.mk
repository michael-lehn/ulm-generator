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

variant_list := \
	$(sort $(wildcard *.isa) $(wildcard */*.isa))

$(foreach v,$(variant_list),\
    $(call add_isa,$v))

$(foreach b,$(wildcard */build.mk */*/build.mk),\
    $(call add,$b,BUILD))

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
ifneq ($(words $(BUILD)),1)
	ifeq ($(strip $(build)),)
		install.target := install_select_build
		install.build := $(firstword $(BUILD))
	else
		install.build := $(build)
	endif
else
	install.build := $(BUILD)
endif

ifneq ($(words $(VARIANT)),1)
	ifeq ($(strip $(isa)),)
		install.target := install_select_isa
	else
		install.variant := $(notdir $(basename $(isa)))
	endif
else
	install.variant := $(VARIANT)
endif

ifeq ($(install.target),)
	install.target := install-$(install.variant)
endif

ifeq ($(strip $(prefix)),)
	install.prefix := /usr/local/bin
endif

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

install: $(install.target)
	

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

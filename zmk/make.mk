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
	$(sort \
		$(dir $(wildcard */variant.mk */*/variant.mk)) \
		$(dir $(wildcard */variant.mk */*/isa.txt)))

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

.DEFAULT_GOAL := all
.PHONY: all
all: $(TARGET)

opt: $(OPT_TARGET)

info:
	@echo "VARIANT=$(VARIANT)"
	@echo "BUILD=$(BUILD)"
	@echo "MODULE=$(MODULE)"
	@echo "dep files:"
	@echo "DEP_FILES=$(DEP_FILES)"
	@echo "EXTRA_DIRS=$(EXTRA_DIRS)"
	@echo "CLEAN_DIRS=$(CLEAN_DIRS)"

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

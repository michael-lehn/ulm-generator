ifeq ($(strip $(shell uname)),Linux)
ARFLAGS := crU
else
ARFLAGS := cr
endif

DEP_DIR := dep
CFLAGS := -Wall -I.
CPPFLAGS := -Wall -I.

include zmk/make.mk

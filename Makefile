CFLAGS := -Wall -I.
CPPFLAGS := -Wall -I.

include config/ar
include config/nfs

include zmk/make.mk

refman: $(REFMAN)

refman-theon: $(REFMAN.theon)


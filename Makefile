CFLAGS += -Wextra -Wall -I.
CPPFLAGS += -Wextra -Wall -I.

#
CLEAN_LIST :=

include config/ar
include config/nfs

include zmk/make.mk

refman: $(REFMAN)

refman-theon: $(REFMAN.theon)

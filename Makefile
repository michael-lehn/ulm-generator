CFLAGS += -Wextra -Wall -I.
CPPFLAGS += -Wextra -Wall -I.

include config/ar
include config/nfs

include zmk/make.mk

refman: $(REFMAN)

refman-theon: $(REFMAN.theon)

SUBDIRS := preprocessor transpiler
CFLAGS = -g -std=c99 -D_XOPEN_SOURCE=700
ifeq ($(PROD), 1)
CFLAGS += -Werror -Wall
endif

export CFLAGS

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
	$(CC) $(CFLAGS) $@/$@.o -o program
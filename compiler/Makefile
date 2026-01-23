SUBDIRS := preprocessor transpiler
LIBS := $(foreach dir,$(SUBDIRS),$(dir)/lib$(dir).a)

CFLAGS = -g -std=c99 -D_XOPEN_SOURCE=700
ifeq ($(PROD), 1)
CFLAGS += -Werror -Wall
endif



export CFLAGS

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS) program

program: $(LIBS)
	$(CC) $(CFLAGS) $(LIBS) -o program

clean:
	@for dir in $(SUBDIRS); do \
	$(MAKE) clean -C $$dir; \
	done
	@-rm -f program

$(SUBDIRS):
	$(MAKE) -C $@
	
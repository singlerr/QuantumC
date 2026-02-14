LIB     = libdata.a
SUBDIRS := btree hamt vec
LIBS := $(foreach dir,$(SUBDIRS),$(dir)/lib$(dir).a)

.PHONY: all clean $(SUBDIRS)

all: $(LIB)

$(LIB): $(LIBS)
	$(AR) rcs $@ $^

$(LIBS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
	$(MAKE) clean -C $$dir; \
	done
	@-rm -f $(LIB)

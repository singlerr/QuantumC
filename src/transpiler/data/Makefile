LIB     = libdata.a
SUBDIRS := btree hamt
LIBS := $(foreach dir,$(SUBDIRS),$(dir)/lib$(dir).a)

.PHONY: all clean $(SUBDIRS)

all: $(LIB)

$(LIB): $(LIBS)
	@tmpdir=$$(mktemp -d) && \
	for lib in $^; do (cd $$tmpdir && $(AR) x $$(cd $(CURDIR) && pwd)/$$lib); done && \
	$(AR) rcs $@ $$tmpdir/*.o && \
	rm -rf $$tmpdir

$(LIBS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	for dir in $(SUBDIRS); do \
	$(MAKE) clean -C $$dir; \
	done
	@-rm -f $(LIB)

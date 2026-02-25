LIB := libhamt.a
SRCDIR := src
INCFLAGS := -Iinclude -Isrc
SRCS := $(SRCDIR)/cache.c \
        $(SRCDIR)/hamt.c \
        $(SRCDIR)/murmur3.c \
        $(SRCDIR)/uh.c
OBJS := $(SRCS:.c=.o)

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $@ $<

clean:
	@-rm -f $(OBJS) $(LIB)

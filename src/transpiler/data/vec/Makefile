LIB = libvec.a
OBJS = vec.o

.PHONY: all clean

all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	@-rm -f $(OBJS) $(LIB)

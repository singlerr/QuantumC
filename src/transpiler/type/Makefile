OBJS = btype.o typesize.o tytab.o deco.o base.o intern.o check.o affine.o
LIB = libtype.a

.PHONY: all clean

all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -I.. -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	@-rm -f $(OBJS) $(LIB)

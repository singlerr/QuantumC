OBJS = hir.o
LIB = libhir.a

.PHONY: all clean

all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -I.. -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	@-rm -f $(OBJS) $(LIB)

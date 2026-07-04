OBJS = undo_log.o snapshot_vec.o unify_table.o
LIB = libena.a

.PHONY: all clean

all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

clean:
	@-rm -f $(OBJS) $(LIB)

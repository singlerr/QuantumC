OBJS = preprocessor.o filebuf.o

.PHONY: all
all: preprocessor

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

preprocessor: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@

clean:
	@-rm -f *.o

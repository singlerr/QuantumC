OBJS = preprocessor.o filebuf.o
LIB = libpreprocessor.a

.PHONY: all
all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS) -o $@

clean:
	@-rm -f *.o $(LIB)

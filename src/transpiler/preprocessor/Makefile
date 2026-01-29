OBJS = filebuf.o preprocessor.lexer.o preprocessor.parser.o preprocessor.o stringbuilder.o
LIB = libpreprocessor.a

.PHONY: all
all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

preprocessor.parser.c: parser.y
	$(YACC) --debug --defines=preprocessor.parser.h $< -o $@

preprocessor.lexer.c: scanner.l
	$(LEX) --header-file=preprocessor.lexer.h -o $@ $<

preprocessor.parser.o: preprocessor.lexer.c preprocessor.parser.c preprocessor.c
preprocessor.lexer.o: preprocessor.parser.c

$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS) -o $@

clean:
	@-rm -f *.o $(LIB)
	@-rm -f preprocessor.parser.c
	@-rm -f preprocessor.parser.h
	@-rm -f preprocessor.lexer.c
	@-rm -f preprocessor.lexer.h
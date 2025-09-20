OBJS = ast.o parser.o lexer.o fe.o
CC = gcc
CFLAGS = -g -Wall
LEX = flex
YACC = bison

all: fe

fe: $(OBJS)
	gcc $(CFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

yacc_parser.c: parser.y
	$(YACC) --defines=yacc_parser.h $< -o $@

lex_lexer.c: scanner.l
	$(LEX) --header-file=lex_lexer.h -o $@ $<
ast.o: ast.h ast.c
parser.o : lexer.o ast.o yacc_parser.c yacc_parser.h
lexer.o: lex_lexer.c yacc_parser.h lex_lexer.h
clean:
	@-rm -f yacc_parser.c
	@-rm -f lex_lexer.c
	@-rm -f parser.tab.c
	@-rm -f *.o
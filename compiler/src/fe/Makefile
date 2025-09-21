OBJS = c.parser.o c.lexer.o fe.o
CC = gcc
CFLAGS = -g -Wall
LEX = flex
YACC = bison

all: fe

fe: $(OBJS)
	gcc $(CFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

c.parser.c: parser.y
	$(YACC) --defines=c.parser.h $< -o $@

c.lexer.c: scanner.l
	$(LEX) --header-file=c.lexer.h -o $@ $<

c.parser.o : ast.h c.lexer.c c.parser.c
c.lexer.o: c.parser.c ast.h

clean:
	@-rm -f c.parser.c
	@-rm -f c.parser.h
	@-rm -f c.lexer.c
	@-rm -f c.lexer.h
	@-rm -f *.o
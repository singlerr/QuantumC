CC=cc
LDFLAGS=-lfl -lm
CFLAGS=
ifeq ($(debug), 1)
CFLAGS = -g
endif
ifeq ($(shell uname),Darwin)
	LDFLAGS=-ll
endif

FLEX ?= flex
BISON ?= bison

all: lexer parser-lalr

lexer: lexer_main.o lexer.o tokens.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

lexer_main.o: lexer_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

tokens.o: tokens.c
	$(CC) $(CFLAGS) -std=c99 -c -o $@ $<

lex.yy.c: lexer.l
	$(FLEX) $<

lexer.o: lex.yy.c tokens.h
	$(CC) $(CFLAGS) -include tokens.h -c -o $@ $<

lexer-lalr.o: lex.yy.c parser-lalr.tab.h
	$(CC) $(CFLAGS) -include parser-lalr.tab.h -c -o $@ $<

parser-lalr.o: parser-lalr.tab.c
	$(CC) $(CFLAGS) -c -o $@ $<

parser-lalr-main.o: parser-lalr-main.c
	$(CC) $(CFLAGS) -std=c99 -c -o $@ $<

parser-lalr: parser-lalr.o parser-lalr-main.o lexer-lalr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

parser-lalr.tab.c parser-lalr.tab.h: parser-lalr.y
	$(BISON) $< -d -p rs -v --report=all --warnings=error=all

rlex: rlex.rs
	rustc $<

rparse: rparse.rs
	rustc $<

check: lexer parser parser-lalr
	./testparser.sh

clean:
	rm -rf *.o lexer parser rlex rparse lex.yy.c \
                   parser.c parser.h \
                   parser-lalr.tab.c parser-lalr.tab.h \
                   parser-lalr.output parser-lalr

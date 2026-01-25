OBJS = c.parser.o c.lexer.o main.o ast.o ast_sqz.o symrec.o type.o stringlib.o diagnostics.o ast_sem.o ast_typing.o codegen.o builtin_func.o builtin_measure.o builtin_gate.o
LEX = flex
YACC = bison
SUBDIRS := preprocessor
LIBS := $(foreach dir,$(SUBDIRS),$(dir)/lib$(dir).a)

CFLAGS = -g -std=c99 -D_XOPEN_SOURCE=700
ifeq ($(PROD), 1)
CFLAGS += -Werror -Wall
endif

export CFLAGS

.PHONY: all clean $(SUBDIRS)

all: program

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

c.parser.c: parser.y
	$(YACC) --debug --defines=c.parser.h $< -o $@

c.lexer.c: scanner.l
	$(LEX) --header-file=c.lexer.h -o $@ $<

c.parser.o: ast.h c.lexer.c c.parser.c

c.lexer.o: c.parser.c ast.h

transpiler.o: main.c ast.h

program: $(LIBS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o program

$(LIBS): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	@-rm -f c.parser.c
	@-rm -f c.parser.h
	@-rm -f c.lexer.c
	@-rm -f c.lexer.h
	@-rm -f parser.tab.c
	@-rm -f parser.tab.h
	@-rm -f *.o
	@-rm -f main
	@-rm -f *.yy.c
	@for dir in $(SUBDIRS); do \
	$(MAKE) clean -C $$dir; \
	done
	@-rm -f program

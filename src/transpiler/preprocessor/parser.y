%{

#include "preprocessor.h"
#include "../preprocessor_link.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NEW_OPERAND
#define NEW_OPERAND(__kind, __value, __ret)                                      \
    do                                                                           \
    {                                                                            \
        struct operand *__op = (struct operand *)malloc(sizeof(struct operand)); \
        __op->kind = __kind;                                                     \
        switch (__kind)                                                          \
        {                                                                        \
        case OP_INTEGER:                                                         \
            __op->value.i = __value;                                             \
            break;                                                               \
        case OP_FLOAT:                                                           \
            __op->value.f = __value;                                             \
            break;                                                               \
        default:                                                                 \
            perror("Error!");                                                    \
            break;                                                               \
        }                                                                        \
        __ret = __op;                                                            \
    } while (0);
#endif

extern int prlex();
void prerror(const char *str);

extern FILE* prin;
struct string_builder* ctx = NULL;

%}

%define parse.error detailed
%define api.prefix pr
%union {
    char* str;
    int i;
    float f;
    int b;
    struct dir_openqasm* openqasm;
    struct dir_define* define;
    struct macro_args* macro_args;
    struct placeholder* placeholder;
    struct operand* operand;
}

%token IDENTIFIER;
%token DEFINE
%token IF ELIF IFDEF IFNDEF ENDIF UNDEF ELSE
%token DEFINED
%token OPENQASM
%token LPAREN RPAREN
%token COMMA NEWLINE
%token NUM
%token PLACEHOLDER STRINGIFIED
%token TEXT
%token AND_OP OR_OP EQ_OP NE_OP GE_OP LE_OP G_OP L_OP

%token INTEGER FLOAT

%type<openqasm> openqasm;
%type<define> define;
%type<define> define_id;
%type<macro_args> define_args;
%type<placeholder> define_body;
%type<placeholder> define_body_item;
%type<operand> primary_expression;
%type<operand> relational_expression;
%type<operand> equality_expression;
%type<operand> logical_and_expression;
%type<operand> logical_or_expression;
%type<operand> if_expression;
%type<b> if_condition;

%start program
%%

program
    : directive_list
    | %empty
    ;

directive_list
    : directive
    | directive_list directive
    ;

directive
    : if_directive
    | ifdef_directive
    | ifndef_directive
    | undef
    | elif
    | define
    | openqasm
    ;

if_directive
    : IF if_condition program ENDIF { if (!$2) { } }
    | IF if_condition program ELSE program ENDIF { }
    ;

if_condition
    : if_expression { $$ = ($1->kind == OP_INTEGER) ? $1->value.i : (int)$1->value.f; set_skip_mode(!$$); }
    ;

ifdef_directive
    : IFDEF IDENTIFIER { set_skip_mode(find_macro(yylval.str) == NULL); } program ENDIF { set_skip_mode(0); }
    | IFDEF IDENTIFIER { int cond = find_macro(yylval.str) != NULL; set_skip_mode(!cond); } program ELSE { set_skip_mode($<b>3); } program ENDIF { set_skip_mode(0); }
    ;

ifndef_directive
    : IFNDEF IDENTIFIER { set_skip_mode(find_macro(yylval.str) != NULL); } program ENDIF { set_skip_mode(0); }
    | IFNDEF IDENTIFIER { int cond = find_macro(yylval.str) == NULL; set_skip_mode(!cond); } program ELSE { set_skip_mode($<b>3); } program ENDIF { set_skip_mode(0); }
    ;

elif
    : ELIF
    ;

undef
    : UNDEF IDENTIFIER
    ;

define
    : define_id LPAREN define_args RPAREN define_body NEWLINE { $1->args = $3; $1->content = $5; push_define($1); $$ = $1; }
    | define_id define_body NEWLINE { $1->content = $2; push_define($1); $$ = $1; }
    | define_id NEWLINE { $1->content = NULL; push_define($1); $$ = $1; }
    ;

define_id
    : DEFINE IDENTIFIER { $$ = new_define((const char*) yylval.str); }
    ;

define_args
    : IDENTIFIER { $$ = args_builder_begin((const char*) yylval.str); }
    | define_args COMMA IDENTIFIER { $$ = args_builder_append($1, (const char*) yylval.str); }
    ;


openqasm
    : OPENQASM NUM { $$ = openqasm_new(yylval.i); }
    ;

define_body
    : define_body_item { $$ = $1; }
    | define_body define_body_item { $$ = ph_builder_concat($1, $2); }
    ;

define_body_item
    : TEXT { $$ = ph_builder_begin(is_define_arg(top_define()->args, (const char*) yylval.str) ? PH_PLACEHOLDER : PH_TEXT, yylval.str); }
    | PLACEHOLDER { $$ = ph_builder_begin(PH_PLACEHOLDER, yylval.str); }
    | STRINGIFIED { $$ = ph_builder_begin(PH_STRINGIFIED, yylval.str); }
    ;


primary_expression
    : INTEGER { NEW_OPERAND(OP_INTEGER, yylval.i, $$); }
    | FLOAT { NEW_OPERAND(OP_FLOAT, yylval.f, $$); }
    | DEFINED LPAREN IDENTIFIER RPAREN { NEW_OPERAND(OP_INTEGER,find_macro(yylval.str) != NULL, $$); }
    ;

relational_expression
	: primary_expression { $$ = $1; }
	| relational_expression L_OP primary_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_L, $1, $3), $$); }
	| relational_expression G_OP primary_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_G, $1, $3), $$); }
	| relational_expression LE_OP primary_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_LE, $1, $3), $$); }
	| relational_expression GE_OP primary_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_GE, $1, $3), $$); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_EQ, $1, $3), $$); }
	| equality_expression NE_OP relational_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_NE, $1, $3), $$); }
	;

logical_and_expression
	: equality_expression { $$ = $1; }
	| logical_and_expression AND_OP equality_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_AND, $1, $3), $$); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { NEW_OPERAND(OP_INTEGER, validate_expr(IF_OR, $1, $3), $$); }
	;

if_expression
    : logical_or_expression { $$ = $1; }
    ;

%%

int init_ctx(struct string_builder* sb, FILE* in){
    ctx = sb;
    prin = in;
}

void prerror(const char *str)
{
    fprintf(stderr, "[preprocessor] %s\n", str);
}

int preprocessor_lex(){
    return prparse();
}

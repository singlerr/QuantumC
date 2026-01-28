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

extern int yylex();
void yyerror(const char *str);
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
%token IF ELIF IFDEF IFNDEF ENDIF UNDEF
%token DEFINED
%token OPENQASM
%token LPAREN RPAREN
%token COMMA
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
%type<operand> primary_expression;
%type<operand> relational_expression;
%type<operand> equality_expression;
%type<operand> logical_and_expression;
%type<operand> logical_or_expression;
%type<operand> if_expression;

%%

program
    : if
    | ifdef
    | ifndef
    | undef
    | elif
    | define
    | openqasm
    ;

if
    : IF if_args ENDIF {  } // body is passed in flex
    ;


if_args
    : if_expression
    ;

ifndef
    : IFNDEF IDENTIFIER
    ;

ifdef
    : IFDEF IDENTIFIER  
    ;

elif
    : ELIF
    ;

undef
    : UNDEF IDENTIFIER
    ;

define
    : define_id { push_define($1); } define_body { $1->content = $3; $$ = $1; }
    | define_id { push_define($1); } LPAREN define_args RPAREN define_body { $1->args = $4; $1->content = $6; $$ = $1; }  
    ;

define_id
    : DEFINE IDENTIFIER { $$ = new_define((const char*) prlval.str); }

define_args
    : define_args COMMA IDENTIFIER { $$ = args_builder_append($1, (const char*) prlval.str); }
    | IDENTIFIER { $$ = args_builder_begin((const char*) prlval.str); } 
    ;


openqasm
    : OPENQASM NUM { $$ = openqasm_new(prlval.i); } 
    ;

define_body
    : define_body TEXT { $$ = ph_builder_append($1, is_define_arg(top_define()->args, (const char*) prlval.str) ? PH_PLACEHOLDER : PH_TEXT, prlval.str); }
    | define_body PLACEHOLDER { $$ = ph_builder_append($1, PH_PLACEHOLDER, prlval.str); }
    | define_body STRINGIFIED { $$ = ph_builder_append($1, PH_STRINGIFIED, prlval.str); }
    | TEXT { $$ = ph_builder_begin(is_define_arg(top_define()->args, (const char*) prlval.str) ? PH_PLACEHOLDER : PH_TEXT, prlval.str); }
    | PLACEHOLDER { $$ = ph_builder_begin(PH_PLACEHOLDER, prlval.str); }
    | STRINGIFIED { $$ = ph_builder_begin(PH_STRINGIFIED, prlval.str); }
    ;


primary_expression
    : INTEGER { NEW_OPERAND(OP_INTEGER, prlval.i, $$); }
    | FLOAT { NEW_OPERAND(OP_FLOAT, prlval.f, $$); }
    | DEFINED LPAREN IDENTIFIER RPAREN { NEW_OPERAND(OP_INTEGER,find_macro(prlval.str) != NULL, $$); }
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



void yyerror(const char *str)
{
    fprintf(stderr, "%s\n", str);
}

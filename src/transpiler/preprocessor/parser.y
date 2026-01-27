%{

%}

%define parse.error detailed

%union {

}


%token IDENTIFIER;
%token DEFINE
%token IF ELIF IFDEF IFNDEF ENDIF
%token DEFINED
%token EXTEND
%token INCLUDE OPENQASM
%token LPAREN, RPAREN
%token COMMA
%token NUM
%token INCLUDE_TARGET
%token PLACEHOLDER STRINGIFIED
%token TEXT

%token AND_OP OR_OP EQ_OP NE_OP GE_OP LE_OP G_OP L_OP

%%

program
    : if
    | ifdef
    | ifndef
    | endif
    | define
    | include
    | openqasm
    | body
    ;

if
    : IF if_args
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

endif
    :
    ;

elif
    :
    ;

define
    : DEFINE IDENTIFIER TEXT
    | DEFINE IDENTIFIER LPAREN define_args RPAREN define_body  
    ;

define_args
    : define_args COMMA IDENTIFIER
    | IDENTIFIER
    ;

include
    : INCLUDE INCLUDE_TARGET
    ;

openqasm
    : OPENQASM NUM
    ;

define_body
    : program
    ;

body
    : TEXT
    | PLACEHOLDER
    ;

primary_expression
    : IDENTIFIER
    | DEFINED LPAREN IDENTIFIER RPAREN
    | DEFINE IDENTIFIER
    ;

relational_expression
	: primary_expression { $$ = $1; }
	| relational_expression L_OP primary_expression { $$  = AST_GENERAL_NODE(AST_EXPR_LT, $1, $3, NULL); }
	| relational_expression G_OP primary_expression { $$ = AST_GENERAL_NODE(AST_EXPR_GT, $1, $3, NULL); }
	| relational_expression LE_OP primary_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LEQ, $1, $3, NULL); }
	| relational_expression GE_OP primary_expression { $$ = AST_GENERAL_NODE(AST_EXPR_GEQ, $1, $3, NULL); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = AST_GENERAL_NODE(AST_EXPR_EQ, $1, $3, NULL); }
	| equality_expression NE_OP relational_expression { $$ = AST_GENERAL_NODE(AST_EXPR_NEQ, $1, $3, NULL); }
	;

logical_and_expression
	: equality_expression { $$ = $1; }
	| logical_and_expression AND_OP equality_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LAND, $1, $3, NULL); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LOR, $1, $3, NULL); }
	;

if_expression
    : logical_or_expression
    ;
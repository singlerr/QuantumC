%{
#include "ast.h"
#include "symrec.h"
#include <stdio.h>
#include <stdlib.h> // For free()
#include <string.h>
#include "stringlib.h"

void yyerror(ast_node** root, char const *s);
extern int yylex();
extern int type_size;
extern int yylineno;
extern char* lineptr;
#include "c.parser.h"

%}

%parse-param { ast_node** root}
%define api.prefix tr
%define parse.error detailed
%union {
	int i;
	float f;
	char *str;
	ast_node* node;
	ast_identifier_node* id_node;
}

%token <id_node> IDENTIFIER
%token STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME


%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token BOOL_TRUE BOOL_FALSE BOOL COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token INTCONSTANT FLOATCONSTANT
%start program


%type <node> program
%type <node> primary_expression
%type <node> postfix_expression
%type <node> argument_expression_list
%type <node> unary_expression
%type <node> unary_operator
%type <node> cast_expression
%type <node> multiplicative_expression
%type <node> additive_expression
%type <node> shift_expression
%type <node> relational_expression
%type <node> equality_expression
%type <node> and_expression
%type <node> exclusive_or_expression
%type <node> inclusive_or_expression
%type <node> logical_and_expression
%type <node> logical_or_expression
%type <node> conditional_expression
%type <node> assignment_expression
%type <node> assignment_operator
%type <node> expression
%type <node> constant_expression
%type <node> declaration
%type <node> declaration_specifiers
%type <node> init_declarator_list
%type <node> init_declarator
%type <node> storage_class_specifier
%type <node> type_specifier
%type <node> struct_or_union_specifier
%type <node> struct_or_union
%type <node> struct_declaration_list
%type <node> struct_declaration
%type <node> specifier_qualifier_list
%type <node> struct_declarator_list
%type <node> struct_declarator
%type <node> enum_specifier
%type <node> enumerator_list
%type <node> enumerator
%type <node> type_qualifier
%type <node> declarator
%type <node> direct_declarator
%type <node> parameter_type_list
%type <node> parameter_list
%type <node> parameter_declaration
%type <node> identifier_list
%type <node> type_name
%type <node> abstract_declarator
%type <node> direct_abstract_declarator
%type <node> initializer
%type <node> initializer_list
%type <node> designation
%type <node> designator_list
%type <node> designator
%type <node> statement
%type <node> labeled_statement
%type <node> compound_statement
%type <node> block_item_list
%type <node> block_item
%type <node> expression_statement
%type <node> selection_statement
%type <node> iteration_statement
%type <node> jump_statement
%type <node> translation_unit
%type <node> external_declaration
%type <node> function_definition
%type <node> declaration_list

%%

program: translation_unit { *root = AST_GENERAL_NODE(AST_PROGRAM, NULL, $1, NULL); }

primary_expression
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL); }
	| INTCONSTANT { $$ = AST_CONST_NODE(AST_LITERAL_INTEGER, new_ast_int_const(yylval.i)); }
	| FLOATCONSTANT { $$ = AST_CONST_NODE(AST_LITERAL_FLOAT, new_ast_float_const(yylval.f)); }
	| STRING_LITERAL { $$ = AST_CONST_NODE(AST_LITERAL_STRING, new_ast_str_const(yylval.str)); }
	| BOOL_TRUE { $$ = AST_CONST_NODE(AST_LITERAL_BOOL, new_ast_bool_const(1)); }
	| BOOL_FALSE { $$ = AST_CONST_NODE(AST_LITERAL_BOOL, new_ast_bool_const(0)); }
	| '(' expression ')' { $$ = $2; }
	;

postfix_expression
	: primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']' { $$ = AST_GENERAL_NODE(AST_EXPR_ARRAY_ACCESS, $1, $3, NULL); }
	| postfix_expression '(' ')' { $$ = AST_GENERAL_NODE(AST_EXPR_FUNCTION_CALL, $1, NULL, NULL); }
	| postfix_expression '(' argument_expression_list ')' { $$ = AST_GENERAL_NODE(AST_EXPR_FUNCTION_CALL, $1, $3, NULL); }
	| postfix_expression '.' IDENTIFIER { $$ = AST_GENERAL_NODE(AST_EXPR_MEMBER_ACCESS, AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL), NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	| postfix_expression PTR_OP IDENTIFIER { $$ = AST_GENERAL_NODE(AST_EXPR_POINTER_MEMBER_ACCESS, AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL), NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	| postfix_expression INC_OP { $$ = AST_GENERAL_NODE(AST_EXPR_POST_INC, $1, NULL, NULL); }
	| postfix_expression DEC_OP { $$ = AST_GENERAL_NODE(AST_EXPR_POST_DEC, $1, NULL, NULL); }
	| '(' type_name ')' '{' initializer_list '}' { $$ = AST_GENERAL_NODE(AST_STRUCT_INIT, $2, $5, NULL); }
	| '(' type_name ')' '{' initializer_list ',' '}' { $$ = AST_GENERAL_NODE(AST_STRUCT_INIT, $2, $5, NULL); }
	;

argument_expression_list
	: assignment_expression { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| argument_expression_list ',' assignment_expression { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, NULL, $3, NULL)); }
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| INC_OP unary_expression { $$ = AST_GENERAL_NODE(AST_EXPR_PRE_INC, $2, NULL, NULL); }
	| DEC_OP unary_expression { $$ = AST_GENERAL_NODE(AST_EXPR_PRE_DEC, $2, NULL, NULL); }
	| unary_operator cast_expression { $$ = AST_GENERAL_NODE(AST_EXPR_UNARY, $1, $2, NULL); }
	| SIZEOF unary_expression { $$ = AST_GENERAL_NODE(AST_EXPR_SIZEOF, $2, NULL, NULL); }
	| SIZEOF '(' type_name ')' { $$ = AST_GENERAL_NODE(AST_EXPR_SIZEOF, NULL, $3, NULL); }
	;

unary_operator
	: '&' { $$ = AST_SIMPLE_NODE(AST_UNARY_AMP); }
	| '*' { $$ = AST_SIMPLE_NODE(AST_UNARY_STAR); }
	| '+' { $$ = AST_SIMPLE_NODE(AST_UNARY_PLUS); }
	| '-' { $$ = AST_SIMPLE_NODE(AST_UNARY_MINUS); }
	| '~' { $$ = AST_SIMPLE_NODE(AST_UNARY_TILDE); }
	| '!' { $$ = AST_SIMPLE_NODE(AST_UNARY_EXCL); }
	;

cast_expression
	: unary_expression { $$ = $1; }
	| '(' type_name ')' cast_expression { $$ = AST_GENERAL_NODE(AST_EXPR_TYPE_CAST, $2, $4, NULL); }
	;

multiplicative_expression
	: cast_expression { $$ = $1; }
	| multiplicative_expression '*' cast_expression { $$ = AST_GENERAL_NODE(AST_EXPR_MUL, $1, $3, NULL); }
	| multiplicative_expression '/' cast_expression { $$ = AST_GENERAL_NODE(AST_EXPR_DIV, $1, $3, NULL); }
	| multiplicative_expression '%' cast_expression { $$ = AST_GENERAL_NODE(AST_EXPR_MOD, $1, $3, NULL); }
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = AST_GENERAL_NODE(AST_EXPR_ADD, $1, $3, NULL); }
	| additive_expression '-' multiplicative_expression { $$ = AST_GENERAL_NODE(AST_EXPR_SUB, $1, $3, NULL); }
	;

shift_expression
	: additive_expression { $$ = $1; }
	| shift_expression LEFT_OP additive_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LSHIFT, $1, $3, NULL); }
	| shift_expression RIGHT_OP additive_expression { $$ = AST_GENERAL_NODE(AST_EXPR_RSHIFT, $1, $3, NULL); }
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression { $$  = AST_GENERAL_NODE(AST_EXPR_LT, $1, $3, NULL); }
	| relational_expression '>' shift_expression { $$ = AST_GENERAL_NODE(AST_EXPR_GT, $1, $3, NULL); }
	| relational_expression LE_OP shift_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LEQ, $1, $3, NULL); }
	| relational_expression GE_OP shift_expression { $$ = AST_GENERAL_NODE(AST_EXPR_GEQ, $1, $3, NULL); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = AST_GENERAL_NODE(AST_EXPR_EQ, $1, $3, NULL); }
	| equality_expression NE_OP relational_expression { $$ = AST_GENERAL_NODE(AST_EXPR_NEQ, $1, $3, NULL); }
	;

and_expression
	: equality_expression { $$ = $1; }
	| and_expression '&' equality_expression { $$ = AST_GENERAL_NODE(AST_EXPR_AND, $1, $3, NULL); }
	;

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = AST_GENERAL_NODE(AST_EXPR_XOR, $1, $3, NULL); }
	;

inclusive_or_expression
	: exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = AST_GENERAL_NODE(AST_EXPR_OR, $1, $3, NULL); }
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LAND, $1, $3, NULL); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = AST_GENERAL_NODE(AST_EXPR_LOR, $1, $3, NULL); }
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = AST_GENERAL_NODE(AST_EXPR_COND, $1, $3, $5); }
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = AST_GENERAL_NODE(AST_EXPR_ASSIGN, $1, $2, $3); }
	;

assignment_operator
	: '=' { $$ = AST_SIMPLE_NODE(AST_EXPR_ASSIGN); }
	| MUL_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_MUL_ASSIGN); }
	| DIV_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_DIV_ASSIGN); }
	| MOD_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_MOD_ASSIGN); }
	| ADD_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_ADD_ASSIGN); }
	| SUB_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_SUB_ASSIGN); }
	| LEFT_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_LEFT_ASSIGN); }
	| RIGHT_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_RIGHT_ASSIGN); }
	| AND_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_AND_ASSIGN); }
	| XOR_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_XOR_ASSIGN); }
	| OR_ASSIGN { $$ = AST_SIMPLE_NODE(AST_EXPR_OR_ASSIGN); }
	;

expression
	: assignment_expression { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
	| expression ',' assignment_expression { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, $3, NULL, NULL)); }
	;

constant_expression
	: conditional_expression { $$ = $1; }
	;

declaration
	: declaration_specifiers ';' { $$ = AST_GENERAL_NODE(AST_VARIABLE_DECLARATION, $1, NULL, NULL); }
	| declaration_specifiers init_declarator_list ';' { register_type_if_required($1, $2); $$ = AST_GENERAL_NODE(AST_VARIABLE_DECLARATION, $1, $2, NULL); }
	;

declaration_specifiers
    : storage_class_specifier { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
    | declaration_specifiers storage_class_specifier { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, $2, NULL, NULL)); }
    | type_specifier { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
    | declaration_specifiers type_specifier { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, NULL, $2, NULL)); }
    | type_qualifier { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
    | declaration_specifiers type_qualifier { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, $2, NULL, NULL)); }
    ;

init_declarator_list
	: init_declarator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| init_declarator_list ',' init_declarator { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, NULL, $3, NULL)); }
	;

init_declarator
	: declarator { $$ = AST_GENERAL_NODE(AST_VARIABLE_DECLARATOR, NULL, $1, NULL); }
	| declarator '=' initializer { $$ = AST_GENERAL_NODE(AST_VARIABLE_DECLARATOR, $3, $1, NULL); }
	;

storage_class_specifier
	: TYPEDEF { $$ = AST_SIMPLE_NODE(AST_STG_TYPEDEF); }
	| EXTERN { $$ = AST_SIMPLE_NODE(AST_STG_EXTERN); }
	| STATIC { $$ = AST_SIMPLE_NODE(AST_STG_STATIC); }
	| AUTO { $$ = AST_SIMPLE_NODE(AST_STG_AUTO); }
	| REGISTER { $$ = AST_SIMPLE_NODE(AST_STG_REGISTER); }
	;

type_specifier
	: VOID { $$ = AST_TYPE_NODE(AST_TYPE_VOID, PRIM_VOID, NULL, NULL, NULL); }
	| CHAR { $$ = AST_TYPE_NODE(AST_TYPE_CHAR, PRIM_CHAR, NULL, NULL, NULL); }
	| SHORT { $$ = AST_TYPE_NODE(AST_TYPE_SHORT, PRIM_SHORT, NULL, NULL, NULL); }
	| INT { $$ = AST_TYPE_NODE(AST_TYPE_INT, PRIM_INT, NULL, NULL, NULL); }
	| LONG { $$ = AST_TYPE_NODE(AST_TYPE_LONG, PRIM_LONG, NULL, NULL, NULL); }
	| FLOAT { $$ = AST_TYPE_NODE(AST_TYPE_FLOAT, PRIM_FLOAT, NULL, NULL, NULL); }
	| DOUBLE { $$ = AST_TYPE_NODE(AST_TYPE_DOUBLE, PRIM_DOUBLE, NULL, NULL, NULL); }
	| SIGNED { $$ = AST_TYPE_NODE(AST_TYPE_SIGNED, PRIM_SIGNED, NULL, NULL, NULL); }
	| UNSIGNED { $$ = AST_TYPE_NODE(AST_TYPE_UNSIGNED, PRIM_UNSIGNED, NULL, NULL, NULL); }
	| COMPLEX { $$ = AST_TYPE_NODE(AST_TYPE_COMPLEX, PRIM_COMPLEX, NULL, NULL, NULL); }
	| IMAGINARY { $$ = AST_TYPE_NODE(AST_TYPE_IMAGINARY, PRIM_IMAGINARY, NULL, NULL, NULL); }
	| BOOL { $$ = AST_TYPE_NODE(AST_TYPE_BOOL, PRIM_BOOL, NULL, NULL, NULL); }
	| struct_or_union_specifier { $$ = AST_GENERAL_NODE(AST_TYPE_STRUCT_UNION, NULL, $1, NULL); }
	| enum_specifier { $$ = AST_GENERAL_NODE(AST_TYPE_ENUM, NULL, $1, NULL); }
	| TYPE_NAME { $$ = AST_TYPE_NODE(AST_TYPE_USER, type_size > 0 ? getsizedtype(yylval.str, type_size) : gettype(yylval.str), NULL, NULL , NULL); }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL); } '{' { inc_scope_level(); } struct_declaration_list '}' { dec_scope_level();  $$ = AST_IDENTIFIER_NODE(AST_STRUCT_UNION, $3 ,$1, NULL, $6); }
	| struct_or_union '{' { inc_scope_level(); }  struct_declaration_list '}' { dec_scope_level(); $$ = AST_GENERAL_NODE(AST_STRUCT_UNION, $1, NULL, $4); }
	| struct_or_union IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_STRUCT_UNION, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), $1, NULL, NULL); }
	;

struct_or_union
	: STRUCT { $$ = AST_SIMPLE_NODE(AST_STRUCT); }
	| UNION { $$ = AST_SIMPLE_NODE(AST_UNION); }
	;

struct_declaration_list
	: struct_declaration { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| struct_declaration_list struct_declaration { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), $2); }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATION, $1, $2, NULL); }
	;

specifier_qualifier_list
	: type_specifier { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| specifier_qualifier_list type_specifier { $$ = $1; append_right_child((ast_node*) find_last_left_child($1), AST_GENERAL_NODE(AST_NODE_LIST, NULL, $2, NULL));  }
	| type_qualifier { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
	| specifier_qualifier_list type_qualifier { $$ = $1; append_middle_child((ast_node*) find_last_middle_child($1), AST_GENERAL_NODE(AST_NODE_LIST, $2, NULL, NULL)); }
	;

struct_declarator_list
	: struct_declarator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| struct_declarator_list ',' struct_declarator { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), $3); }
	;

struct_declarator
	: declarator { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATOR, NULL, $1, NULL); }
	| ':' constant_expression { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATOR, NULL, NULL, $2); }
	| declarator ':' constant_expression { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATOR, NULL, $1, $3); }
	;

enum_specifier
	: ENUM '{'  enumerator_list '}' { $$ = AST_GENERAL_NODE(AST_ENUM, NULL, $3, NULL); }
	| ENUM IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL); } '{' enumerator_list '}' { $$ = AST_IDENTIFIER_NODE(AST_ENUM, $2, NULL, NULL, NULL); }
	| ENUM '{' enumerator_list ',' '}' { $$ = AST_GENERAL_NODE(AST_ENUM, NULL, $3, NULL); }
	| ENUM IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL); } '{' enumerator_list ',' '}' { $$ = AST_IDENTIFIER_NODE(AST_ENUM, $3, NULL, $5, NULL); }
	| ENUM IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_ENUM, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL); }
	;

enumerator_list
	: enumerator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| enumerator_list ',' enumerator { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), $3); }
	;

enumerator
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_ENUM_FIELD_DECLARATION, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL); }
	| IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL); } '=' constant_expression { $$ = AST_IDENTIFIER_NODE(AST_ENUM_FIELD_DECLARATION, $2, NULL, $4, NULL); }
	;

type_qualifier
	: CONST { $$ = AST_SIMPLE_NODE(AST_QAL_CONST); }
	| RESTRICT { $$ = AST_SIMPLE_NODE(AST_QAL_RESTRICT); }
	| VOLATILE { $$ = AST_SIMPLE_NODE(AST_QAL_VOLATILE); }
	;

declarator
	: direct_declarator { $$ = $1; }
	;


direct_declarator
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL); }
	| '(' declarator ')' { $$ = $2; }
	/* | direct_declarator '[' type_qualifier_list assignment_expression ']' { $$ = AST_GENERAL_NODE(AST_TYPE_ARRAY, $3, $4, NULL); append_right_child(find_last_right_child($1), $$); $$ = $1; } */
	/* | direct_declarator '[' type_qualifier_list ']' { $$ = AST_GENERAL_NODE(AST_TYPE_ARRAY, $3, NULL, NULL); append_right_child(find_last_right_child($1), $$); $$ = $1; } */
	| direct_declarator '[' assignment_expression ']' { $$ = AST_GENERAL_NODE(AST_TYPE_ARRAY, NULL, $3, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	/* | direct_declarator '[' STATIC type_qualifier_list assignment_expression ']' { $$ = $1; append_right_child(find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, $3, $4, NULL)); } */
	/* | direct_declarator '[' type_qualifier_list STATIC assignment_expression ']' */
	/* | direct_declarator '[' type_qualifier_list '*' ']' { $$ = $1; append_right_child(find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, AST_GENERAL_NODE(AST_TYPE_POINTER, NULL, NULL, $3), NULL, NULL)); } */
	/* | direct_declarator '[' '*' ']' */
	| direct_declarator '[' ']' { $$ = AST_GENERAL_NODE(AST_TYPE_ARRAY, NULL, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	| direct_declarator '(' { inc_scope_level(); }  parameter_type_list ')' { dec_scope_level(); $$ = AST_GENERAL_NODE(AST_TYPE_FUNCTION, $4, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	| direct_declarator '(' identifier_list ')' { $$ = AST_GENERAL_NODE(AST_TYPE_FUNCTION, $3, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	| direct_declarator '(' ')' { $$ = AST_GENERAL_NODE(AST_TYPE_FUNCTION, NULL, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	;


parameter_type_list
	: parameter_list { $$ = $1; }
	| parameter_list ',' ELLIPSIS { perror("Sorry! Currently variadic parameters are not allowed."); }
	;

parameter_list
	: parameter_declaration { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| parameter_list ',' parameter_declaration { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_GENERAL_NODE(AST_NODE_LIST, NULL, $3, NULL)); }
	;

parameter_declaration
	: declaration_specifiers declarator { $$ = AST_GENERAL_NODE(AST_PARAMETER_DECLARATION, $1, $2, NULL); }
	| declaration_specifiers abstract_declarator { $$ = AST_GENERAL_NODE(AST_PARAMETER_DECLARATION, $1, NULL, $2); }
	| declaration_specifiers { $$ = AST_GENERAL_NODE(AST_PARAMETER_DECLARATION, $1, NULL, NULL); }
	;

identifier_list
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL); }
	| identifier_list ',' IDENTIFIER { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL)); }
	;

type_name
	: specifier_qualifier_list { $$ = AST_GENERAL_NODE(AST_NAME_TYPE, $1, NULL, NULL); }
	| specifier_qualifier_list abstract_declarator { $$ = $1; append_right_child((ast_node*) find_last_right_child($1), $2); }
	;

abstract_declarator
	: direct_abstract_declarator { $$ = $1; }
	;

direct_abstract_declarator
    : '(' abstract_declarator ')' { $$ = $2; }
    | '[' ']' { $$ = AST_SIMPLE_NODE(AST_TYPE_ARRAY); }
    | '[' assignment_expression ']' { $$ = AST_GENERAL_NODE(AST_TYPE_ARRAY, NULL, $2, NULL); }
    | direct_abstract_declarator '[' ']' { $$ = AST_SIMPLE_NODE(AST_TYPE_ARRAY); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
    | direct_abstract_declarator '[' assignment_expression ']' { $$ = AST_GENERAL_NODE(AST_TYPE_ARRAY, NULL, $3, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = (ast_node*) $1; }
    /* | '[' '*' ']' { }
    | direct_abstract_declarator '[' '*' ']' */
    | '(' ')' { $$ = AST_SIMPLE_NODE(AST_TYPE_FUNCTION); }
    | '(' parameter_type_list ')' { $$ = AST_GENERAL_NODE(AST_TYPE_FUNCTION, $2, NULL, NULL); }
    | direct_abstract_declarator '(' ')' { $$ = AST_SIMPLE_NODE(AST_TYPE_FUNCTION); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
    | direct_abstract_declarator '(' parameter_type_list ')' { $$ = AST_GENERAL_NODE(AST_TYPE_FUNCTION, $1, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
    ;

initializer
	: assignment_expression { $$ = $1; }
	| '{' initializer_list '}' { $$ = $2; }
	| '{' initializer_list ',' '}' { $$ = $2; }
	;

initializer_list
	: initializer { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| designation initializer { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, $2, NULL); }
	| initializer_list ',' initializer { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $3, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	| initializer_list ',' designation initializer { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $3, $4, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	;

designation
	: designator_list '=' { $$ = $1; }
	;

designator_list
	: designator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
	| designator_list designator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $2, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	;

designator
	: '[' constant_expression ']' { $$ = AST_GENERAL_NODE(AST_ARRAY_ACCESS, $2, NULL, NULL); }
	| '.' IDENTIFIER { $$ = AST_GENERAL_NODE(AST_MEMBER_ACCESS, AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_NONSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL), NULL, NULL); }
	;

statement
	: labeled_statement { $$ = $1; }
	| compound_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	| iteration_statement { $$ = $1; }
	| jump_statement { $$ = $1; }
	;

labeled_statement
	: IDENTIFIER ':' statement { $$ = AST_GENERAL_NODE(AST_STMT_LABEL, AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL), $3, NULL); }
	| CASE constant_expression ':' statement { $$ = AST_GENERAL_NODE(AST_STMT_CASE, $2, $4, NULL); }
	| DEFAULT ':' statement { $$ = AST_GENERAL_NODE(AST_STMT_DEFAULT, NULL, $3, NULL); }
	;

compound_statement
	: '{' '}' { $$ = AST_SIMPLE_NODE(AST_STMT_COMPOUND); }
	| '{' block_item_list '}' { $$ = AST_GENERAL_NODE(AST_STMT_COMPOUND, NULL, $2, NULL); }
	;

block_item_list
	: block_item { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
	| block_item_list block_item { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $2, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	;

block_item
	: declaration { $$ = $1; }
	| statement { $$ = $1; }
	;

expression_statement
	: ';' { $$ = AST_SIMPLE_NODE(AST_STMT_EXPRESSION); }
	| expression ';' { $$ = AST_GENERAL_NODE(AST_STMT_EXPRESSION, $1, NULL, NULL); }
	;

selection_statement
	: IF '(' expression ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_IF, $3, $5, NULL); }
	| IF '(' expression ')' statement ELSE statement { $$ = AST_GENERAL_NODE(AST_STMT_IF_ELSE, $3, $5, $7); }
	| SWITCH '(' expression ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_SWITCH, $3, $5, NULL); }
	;

iteration_statement
	: WHILE '(' expression ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_WHILE, $3, $5, NULL); }
	| DO statement WHILE '(' expression ')' ';' { $$ = AST_GENERAL_NODE(AST_STMT_DO_WHILE, $5, $2, NULL); }
	| FOR '(' expression_statement expression_statement ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_FOR_EXPR, $3, $4, NULL); $$ = AST_GENERAL_NODE(AST_STMT_FOR, $$, $6, NULL); }
	| FOR '(' expression_statement expression_statement expression ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_FOR_EXPR, $3, $4, $5); $$ = AST_GENERAL_NODE(AST_STMT_FOR, $$, $7, NULL); }
	| FOR '(' declaration expression_statement ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_FOR_EXPR, $3, $4, NULL); $$ = AST_GENERAL_NODE(AST_STMT_FOR, $$, $6, NULL); }
	| FOR '(' declaration expression_statement expression ')' statement { $$ = AST_GENERAL_NODE(AST_STMT_FOR_EXPR, $3, $4, $5); $$ = AST_GENERAL_NODE(AST_STMT_FOR, $$, $7, NULL); }
	;

jump_statement
	: GOTO IDENTIFIER ';' { $$ = AST_GENERAL_NODE(AST_STMT_GOTO, AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yylval.str), NULL), NULL, NULL, NULL), NULL, NULL); }
	| CONTINUE ';' { $$ = AST_SIMPLE_NODE(AST_STMT_CONTINUE); }
	| BREAK ';' { $$ = AST_SIMPLE_NODE(AST_STMT_BREAK); }
	| RETURN ';' { $$ = AST_SIMPLE_NODE(AST_STMT_RETURN); }
	| RETURN expression ';' { $$ = AST_GENERAL_NODE(AST_STMT_RETURN, $2, NULL, NULL); }
	;

translation_unit
	: external_declaration { $$ = AST_GENERAL_NODE(AST_TRANSLATION_UNIT, $1, NULL, NULL); }
	| translation_unit external_declaration { $$ = AST_GENERAL_NODE(AST_TRANSLATION_UNIT, $2, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	;

external_declaration
	: function_definition { $$ = $1; }
	| declaration { $$ = $1; }
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement { $$ = AST_GENERAL_NODE(AST_FUNCTION_DECLARATION, $1, $2, AST_GENERAL_NODE(AST_FUNCTION_BODY, $3, $4, NULL)); }
	| declaration_specifiers declarator compound_statement { $$ = AST_GENERAL_NODE(AST_FUNCTION_DECLARATION, $1, $2, AST_GENERAL_NODE(AST_FUNCTION_BODY, NULL, $3, NULL)); }
	;

declaration_list
	: declaration { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $1, NULL, NULL); }
	| declaration_list declaration { $$ = AST_GENERAL_NODE(AST_NODE_LIST, $2, NULL, NULL); append_right_child((ast_node*) find_last_right_child($1), $$); $$ = $1; }
	;


%%


#include <stdio.h>

extern int column;

void yyerror(ast_node** root, const char *str)
{
	fprintf(stderr,"error: %s in line %d, column %d\n", str, yylineno, column);
    fprintf(stderr,"%s", lineptr);
    for(int i = 0; i < column - 1; i++)
        fprintf(stderr,"_");
    fprintf(stderr,"^\n");
}

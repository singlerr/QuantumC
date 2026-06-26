%{
%}
%define api.pure full
%define api.prefix tr
%define parse.error detailed
%define parse.trace
%locations

%param {
	yyscan_t scanner
}

%parse-param {
	ast_t** root
}

%code top {
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "stringlib.h"
	#include "type/deco.h"
	#include "type/tytab.h"
	#include "type/affine.h"
	#include "ast.h"
	#include "ast_internal.h"
	#include "preprocessor_link.h"
	#include "c.parser.h"
}

%code requires {
	typedef void* yyscan_t;
}

%code {
	void yyerror(YYLTYPE* yyllocp, yyscan_t unused, ast_t** root, char const *msg);
	int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
}

%union {
	int i;
	int constr;
	float f;
	char *str;
	ast_tag_t ast_tag;
	ast_t* ast;
	type_t* type;
	ty_deco_t* ty_deco;
	ty_struct_t* ty_struct;
	args_t* args;
	decl_t* decl;
	decl_list_t decl_list;
	arg_list_t arg_list;
	arg_t arg;
	struct_fields_t struct_fields;
	struct_field_t struct_field;
	stmt_compound_t stmt_compound;
	stmt_if_t stmt_if;
	stmt_if_else_t stmt_if_else;
	stmt_switch_t stmt_switch;
	stmt_while_t stmt_while;
	stmt_for_t stmt_for;
	expr_unary_t expr_unary;
	expr_binary_t expr_binary;
	expr_ternary_t expr_ternary;
}

%token <str> IDENTIFIER
%token <i> INTCONSTANT
%token <f> FLOATCONSTANT
%token <type> TYPE_NAME
%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token BOOL_TRUE BOOL_FALSE KW_BOOL COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token STRING_LITERAL

%start program

%type<ast> primary_expression
%type<ast> postfix_expression
%type<ast> argument_expression_list
%type<ast> unary_expression
%type<ast> cast_expression
%type<ast> multiplicative_expression
%type<ast> additive_expression
%type<ast> shift_expression
%type<ast> relational_expression
%type<ast> equality_expression
%type<ast> and_expression
%type<ast> exclusive_or_expression
%type<ast> inclusive_or_expression
%type<ast> logical_or_expression
%type<ast> logical_and_expression
%type<ast> conditional_expression
%type<ast> assignment_expression
%type<ast> expression
%type<ast> constant_expression
%type<ast> initializer
%type<ast> initializer_list
%type<ast> designator
%type<ast> designation

%type<decl_list> declaration
%type<decl_list> init_declarator_list
%type<decl> init_declarator

%type<constr> type_qualifier
%type<constr> type_qualifier_list
%type<constr> storage_class_specifier

%type<type> type_specifier
%type<type> enum_specifier
%type<type> struct_or_union_specifier

%type<ty_struct> struct_or_union

%type<struct_fields> struct_declaration_list
%type<struct_field> struct_declaration

%type<args> parameter_type_list
%type<args> identifier_list
%type<arg_list> parameter_list
%type<arg> parameter_declaration

%type<decl> direct_declarator
%type<decl> declarator

%type<ty_deco> pointer
%type<ty_deco> declaration_specifiers
%type<ty_deco> specifier_qualifier_list
%type<ty_deco> direct_abstract_declarator
%type<ty_deco> abstract_declarator
%type<ty_deco> type_name

%type<ast_tag> unary_operator
%type<ast_tag> assignment_operator

%%

program
	: translation_unit { *root = NULL; }
	;

primary_expression
	: IDENTIFIER { $$ = new_ast_var(search_var($1), search_symbol_type($1, FALSE)); }
	| INTCONSTANT { $$ = new_ast_literal(AST_INT, Int($1), Deco(Type(SIZE_INT, TY_INT), CONSTR_EMPTY)); }
	| FLOATCONSTANT { $$ = new_ast_literal(AST_FLOAT, Float($1), Deco(Type(SIZE_FLOAT, TY_FLOAT), CONSTR_EMPTY)); }
	| BOOL_TRUE { $$ = new_ast_literal(AST_INT, Bool(TRUE), Deco(Type(1, TY_BOOL), CONSTR_EMPTY)); }
	| BOOL_FALSE { $$ = new_ast_literal(AST_INT, Bool(FALSE), Deco(Type(1, TY_BOOL), CONSTR_EMPTY)); }
	| '(' expression ')' { $$ = $2; }
	;

postfix_expression
	: primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']' { $$ = new_ast_arr_access(ArrAccess($1, $3), $1->ty); }
	| postfix_expression '(' ')' { $$ = new_ast_app(App($1, NULL), $1->ty); }
	| postfix_expression '(' argument_expression_list ')' {
		if ($1 && $1->tag == AST_VAR) affine_check_call($1->var.name, $3);
		$$ = new_ast_app(App($1, $3), $1->ty);
	}
	| postfix_expression '.' IDENTIFIER { $$ = from_struct_member($1->ty, $3); }
	| postfix_expression PTR_OP IDENTIFIER { $$ = from_struct_member(ref_pointer($1->ty), $3); }
	| postfix_expression INC_OP { $$ = new_ast_unary_expr(AST_POST_INC, Unary($1), $1->ty); }
	| postfix_expression DEC_OP { $$ = new_ast_unary_expr(AST_POST_DEC, Unary($1), $1->ty); }
	| '(' type_name ')' '{' initializer_list '}' { $$ = NULL; }
	| '(' type_name ')' '{' initializer_list ',' '}' { $$ = NULL; }
	;

argument_expression_list
	: assignment_expression { $$ = $1; }
	| argument_expression_list ',' assignment_expression { $$ = new_ast_expr_list(List($1, $3), $3->ty); }
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| INC_OP unary_expression { $$ = new_ast_unary_expr(AST_PRE_INC, Unary($2), $2->ty); }
	| DEC_OP unary_expression { $$ = new_ast_unary_expr(AST_PRE_DEC, Unary($2), $2->ty); }
	| unary_operator cast_expression { $$ = new_ast_unary_expr($1, Unary($2), $2->ty); }
	| SIZEOF unary_expression { $$ = new_ast_literal(AST_INT, UInt(ast_sizeof($2)), Deco(Type(SIZE_INT, TY_UINT), CONSTR_EMPTY)); }
	| SIZEOF '(' type_name ')' { $$ = new_ast_literal(AST_INT, UInt(type_sizeof($3->ty)), Deco(Type(SIZE_INT, TY_UINT), CONSTR_EMPTY)); }
	;

unary_operator
	: '&' { $$ = AST_UNARY_REF; }
	| '*' { $$ = AST_UNARY_DEREF; }
	| '+' { $$ = AST_UNARY_PLUS; }
	| '-' { $$ = AST_UNARY_MINUS; }
	| '~' { $$ = AST_UNARY_NOT; }
	| '!' { $$ = AST_UNARY_LNOT; }
	;

cast_expression
	: unary_expression { $$ = $1; }
	| '(' type_name ')' cast_expression { $$ = new_ast_cast_expr(Cast($2, $4)); }
	;

multiplicative_expression
	: cast_expression { $$ = $1; }
	| multiplicative_expression '*' cast_expression { $$ = new_ast_binary_expr(AST_MUL, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| multiplicative_expression '/' cast_expression { $$ = new_ast_binary_expr(AST_DIV, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| multiplicative_expression '%' cast_expression { $$ = new_ast_binary_expr(AST_MOD, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = new_ast_binary_expr(AST_ADD, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| additive_expression '-' multiplicative_expression { $$ = new_ast_binary_expr(AST_SUB, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

shift_expression
	: additive_expression { $$ = $1; }
	| shift_expression LEFT_OP additive_expression { $$ = new_ast_binary_expr(AST_LSHIFT, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| shift_expression RIGHT_OP additive_expression { $$ = new_ast_binary_expr(AST_RSHIFT, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression { $$ = new_ast_binary_expr(AST_LT, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| relational_expression '>' shift_expression { $$ = new_ast_binary_expr(AST_GT, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| relational_expression LE_OP shift_expression { $$ = new_ast_binary_expr(AST_LEQ, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| relational_expression GE_OP shift_expression { $$ = new_ast_binary_expr(AST_GEQ, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = new_ast_binary_expr(AST_EQ, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	| equality_expression NE_OP relational_expression { $$ = new_ast_binary_expr(AST_NEQ, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

and_expression
	: equality_expression { $$ = $1; }
	| and_expression '&' equality_expression { $$ = new_ast_binary_expr(AST_AND, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = new_ast_binary_expr(AST_XOR, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

inclusive_or_expression
	: exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = new_ast_binary_expr(AST_OR, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = new_ast_binary_expr(AST_LAND, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = new_ast_binary_expr(AST_LOR, Binary($1, $3), TypeCast($1->ty, $3->ty)); }
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = new_ast_ternary_expr(AST_COND, Ternary($1, $3, $5), TypeCast($3->ty, $5->ty)); }
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = new_ast_binary_expr($2, Binary($1, $3), $1->ty); }
	;

assignment_operator
	: '=' { $$ = AST_ASSIGN; }
	| MUL_ASSIGN { $$ = AST_ASSIGN_MUL; }
	| DIV_ASSIGN { $$ = AST_ASSIGN_DIV; }
	| MOD_ASSIGN { $$ = AST_ASSIGN_MOD; }
	| ADD_ASSIGN { $$ = AST_ASSIGN_ADD; }
	| SUB_ASSIGN { $$ = AST_ASSIGN_SUB; }
	| LEFT_ASSIGN { $$ = AST_ASSIGN_LSHIFT; }
	| RIGHT_ASSIGN { $$ = AST_ASSIGN_RSHIFT; }
	| AND_ASSIGN { $$ = AST_ASSIGN_AND; }
	| XOR_ASSIGN { $$ = AST_ASSIGN_XOR; }
	| OR_ASSIGN  { $$ = AST_ASSIGN_OR; }
	;

expression
	: assignment_expression { $$ = $1; }
	| expression ',' assignment_expression { $$ = new_ast_expr_list(List($1, $3), $1->ty); }
	;

constant_expression
	: conditional_expression { $$ = $1; }
	;

declaration
	: declaration_specifiers ';' { $$ = NULL; }
	| declaration_specifiers init_declarator_list ';' { $$ = deco_init_declarator($2, $1); }
	;

declaration_specifiers
	: storage_class_specifier { $$ = begin_deco_constr($1); }
	| declaration_specifiers storage_class_specifier { $$ = deco_constr($1, $2); }
	| type_specifier { $$ = begin_deco_ty($1); }
	| declaration_specifiers type_specifier { $$ = deco_type($1, $2); }
	| type_qualifier { $$ = begin_deco_constr($1); }
	| declaration_specifiers type_qualifier { $$ = deco_constr($1, $2); }
	;

init_declarator_list
	: init_declarator { $$ = NULL; cvector_push_back($$, *$1); }
	| init_declarator_list ',' init_declarator { cvector_push_back($1, *$3); $$ = $1; }
	;

init_declarator
	: declarator { $$ = $1; }
	| declarator '=' initializer { $1->init = $3; $$ = $1; }
	;

storage_class_specifier
	: TYPEDEF { $$ = CONSTR_TYPEDEF; }
	| EXTERN { $$ = CONSTR_EXTERN; }
	| STATIC { $$ = CONSTR_STATIC; }
	| AUTO { $$ = CONSTR_AUTO; }
	| REGISTER { $$ = CONSTR_REGISTER; }
	;

type_specifier
	: VOID { $$ = Type(0, TY_VOID); }
	| CHAR { $$ = Type(SIZE_CHAR, TY_CHAR); }
	| SHORT { $$ = Type(SIZE_SHORT, TY_SHORT); }
	| INT { $$ = Type(SIZE_INT, TY_INT); }
	| LONG { $$ = Type(SIZE_LONG, TY_LONG); }
	| FLOAT { $$ = Type(SIZE_FLOAT, TY_FLOAT); }
	| DOUBLE { $$ = Type(SIZE_DOUBLE, TY_DOUBLE); }
	| SIGNED { $$ = Type(SIZE_INT, TY_INT); }
	| UNSIGNED { $$ = Type(SIZE_INT, TY_UINT); }
	| COMPLEX { $$ = Type(0, TY_COMPLEX); }
	| IMAGINARY { $$ = Type(0, TY_IMAGINARY); }
	| KW_BOOL { $$ = Type(1, TY_BOOL); }
	| struct_or_union_specifier { $$ = $1; }
	| enum_specifier { $$ = $1; }
	| TYPE_NAME { $$ = $1; }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
		{ set_struct_name($1, $2); $1->fields = $4; $$ = new_struct_type($1); }
	| struct_or_union '{' struct_declaration_list '}'
		{ $1->fields = $3; $$ = new_struct_type($1); }
	| struct_or_union IDENTIFIER
		{ $$ = new_struct_type(set_struct_name($1, $2)); }
	;

struct_or_union
	: STRUCT { $$ = begin_struct(); }
	| UNION { $$ = begin_union(); }
	;

struct_declaration_list
	: struct_declaration { $$ = NULL; cvector_push_back($$, $1); }
	| struct_declaration_list struct_declaration { $$ = $1; cvector_push_back($$, $2); }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' { memset(&$$, 0, sizeof($$)); $$.ty = $1; }
	;

specifier_qualifier_list
	: type_specifier { $$ = begin_deco_ty($1); }
	| specifier_qualifier_list type_specifier { $$ = deco_type($1, $2); }
	| type_qualifier { $$ = begin_deco_constr($1); }
	| specifier_qualifier_list type_qualifier { $$ = deco_constr($1, $2); }
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expression
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}' { $$ = Type(SIZE_INT, TY_INT); }
	| ENUM IDENTIFIER '{' enumerator_list '}' { $$ = Type(SIZE_INT, TY_INT); }
	| ENUM '{' enumerator_list ',' '}' { $$ = Type(SIZE_INT, TY_INT); }
	| ENUM IDENTIFIER '{' enumerator_list ',' '}' { $$ = Type(SIZE_INT, TY_INT); }
	| ENUM IDENTIFIER { $$ = Type(SIZE_INT, TY_INT); }
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER '=' constant_expression
	;

type_qualifier
	: CONST { $$ = CONSTR_CONST; }
	| RESTRICT { $$ = CONSTR_RESTRICT; }
	| VOLATILE { $$ = CONSTR_VOLATILE; }
	;

declarator
	: pointer direct_declarator { pointer_tail(&$1->ty->ty.ty_pointer)->ref = $2->type; $2->type = $1; $$ = $2; }
	| direct_declarator { $$ = $1; }
	;

pointer
	: '*' { $$ = EmptyPointer(); }
	| '*' type_qualifier_list { $$ = Pointer(NULL, $2); }
	| '*' pointer { $$ = Pointer($2, CONSTR_EMPTY); }
	| '*' type_qualifier_list pointer { $$ = Pointer($3, $2); }
	;

type_qualifier_list
	: type_qualifier { $$ = $1; }
	| type_qualifier_list type_qualifier { $$ = $1 | $2; }
	;

direct_declarator
	: IDENTIFIER { $$ = begin_decl($1); }
	| '(' declarator ')' { $$ = $2; }
	| direct_declarator '[' assignment_expression ']' { $$ = decl_array($1, -1); }
	| direct_declarator '[' ']' { $$ = decl_array($1, -1); }
	| direct_declarator '(' parameter_type_list ')' { $$ = $1; }
	| direct_declarator '(' identifier_list ')' { $$ = $1; }
	| direct_declarator '(' ')' { $$ = $1; }
	;

parameter_type_list
	: parameter_list { $$ = new_args($1, FALSE); }
	| parameter_list ',' ELLIPSIS { $$ = new_args($1, TRUE); }
	;

parameter_list
	: parameter_declaration { $$ = NULL; cvector_push_back($$, $1); }
	| parameter_list ',' parameter_declaration { $$ = $1; cvector_push_back($$, $3); }
	;

parameter_declaration
	: declaration_specifiers declarator {
		arg_t a;
		a.has_name = $2->has_name;
		if ($2->has_name)
			strncpy(a.name, $2->name, SYM_MAXLEN);
		else
			a.name[0] = '\0';
		a.ty = $1;
		$$ = a;
	}
	| declaration_specifiers abstract_declarator {
		arg_t a;
		a.has_name = 0;
		a.name[0] = '\0';
		a.ty = $1;
		$$ = a;
	}
	| declaration_specifiers {
		arg_t a;
		a.has_name = 0;
		a.name[0] = '\0';
		a.ty = $1;
		$$ = a;
	}
	;

identifier_list
	: IDENTIFIER { $$ = new_args(NULL, FALSE); }
	| identifier_list ',' IDENTIFIER { $$ = $1; }
	;

type_name
	: specifier_qualifier_list { $$ = $1; }
	| specifier_qualifier_list abstract_declarator { $$ = append_ty($2, $1); }
	;

abstract_declarator
	: pointer { $$ = $1; }
	| direct_abstract_declarator { $$ = $1; }
	| pointer direct_abstract_declarator { pointer_tail(&$1->ty->ty.ty_pointer)->ref = $2; $$ = $1; }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')' { $$ = $2; }
	| '[' ']' { $$ = new_ty_array((ty_array_t){ .ref = NULL, .size = -1 }, CONSTR_EMPTY); }
	| '[' assignment_expression ']' { $$ = new_ty_array((ty_array_t){ .ref = NULL, .size = -1 }, CONSTR_EMPTY); }
	| direct_abstract_declarator '[' ']' { $$ = $1; }
	| direct_abstract_declarator '[' assignment_expression ']' { $$ = $1; }
	| '(' ')' { $$ = new_ty_fun((ty_fun_t){0}, CONSTR_EMPTY); }
	| '(' parameter_type_list ')' { $$ = new_ty_fun((ty_fun_t){0}, CONSTR_EMPTY); }
	| direct_abstract_declarator '(' ')' { $$ = $1; }
	| direct_abstract_declarator '(' parameter_type_list ')' { $$ = $1; }
	;

initializer
	: assignment_expression { $$ = $1; }
	| '{' initializer_list '}' { $$ = $2; }
	| '{' initializer_list ',' '}' { $$ = $2; }
	;

initializer_list
	: initializer { $$ = $1; }
	| designation initializer { $$ = $2; }
	| initializer_list ',' initializer { $$ = $3; }
	| initializer_list ',' designation initializer { $$ = $4; }
	;

designation
	: designator_list '=' { $$ = NULL; }
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']' { $$ = $2; }
	| '.' IDENTIFIER { $$ = NULL; }
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{' block_item_list '}'
	;

block_item_list
	: block_item
	| block_item_list block_item
	;

block_item
	: declaration
	| statement
	;

expression_statement
	: ';'
	| expression ';'
	;

if_head
	: IF '(' expression ')' { affine_snap_push(); }
	;

if_then
	: if_head statement
	;

selection_statement
	: if_then { affine_if_noelse(); }
	| if_then ELSE { affine_prep_else(); } statement { affine_join(); }
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' { affine_loop_push(); } statement { affine_loop_pop(); }
	| DO { affine_loop_push(); } statement WHILE '(' expression ')' ';' { affine_loop_pop(); }
	| FOR '(' expression_statement expression_statement ')' { affine_loop_push(); } statement { affine_loop_pop(); }
	| FOR '(' expression_statement expression_statement expression ')' { affine_loop_push(); } statement { affine_loop_pop(); }
	| FOR '(' declaration expression_statement ')' { affine_loop_push(); } statement { affine_loop_pop(); }
	| FOR '(' declaration expression_statement expression ')' { affine_loop_push(); } statement { affine_loop_pop(); }
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list { affine_fn_enter(); } compound_statement { affine_fn_exit(); }
	| declaration_specifiers declarator { affine_fn_enter(); } compound_statement { affine_fn_exit(); }
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

%%

#include <stdio.h>

extern char *current_line_buffer;

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, ast_t** root, const char* msg)
{
	(void)unused; (void)root;
	fprintf(stderr,"[transpiler]: %s in line %d, column %d\n", msg, yyllocp->first_line, yyllocp->first_column);
	if (current_line_buffer)
		fprintf(stderr, "  %s\n", current_line_buffer);
	fprintf(stderr, "  ");
	for(int i = 1; i < yyllocp->first_column; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "^\n");
}

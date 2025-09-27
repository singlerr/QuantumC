%{
#include "ast.h"
#include <stdlib.h> // For free()
#include <string.h>

void append_child(struct ast_node *node, const struct ast_node *child)
{
    int n = node->child_count + 1;
    node->children = realloc(node->children, sizeof(struct ast_node *) * n);
    node->children[node->child_count] = child;
    node->child_count = n;
}

struct ast_node* empty_node(){
	// return new_ast_node(EMPTY, 0);
	return 0;
}


void yyerror(struct ast_node **root, char const *s);


extern int yylex();

#include "c.parser.h"

%}

%define parse.trace
%define parse.error detailed

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token POSTFIX_ARRAY POSTFIX_CALL POSTFIX_MEMBER POSTFIX_PTR_MEMBER POSTFIX_INC POSTFIX_DEC
%token COMPOUND_LITERAL ARG_LIST PREFIX_INC PREFIX_DEC SIZEOF_EXPR SIZEOF_TYPE CAST
%token AND EXCLUSIVE_OR INCLUSIVE_OR CONDITIONAL COMMA_EXPR DECL_SPECIFIERS INIT_DECL_LIST
%token STRUCT_DECL_LIST SPEC_QUAL_LIST BITFIELD ARRAY_DECLARATOR FUNCTION_DECLARATOR
%token TYPE_QUAL_LIST PARAM_LIST INIT_LIST DESIGNATED_INITITALIZER ARRAY_DESIGNATOR MEMBER_DESIGNATOR
%token CASE_STATEMENT DEFAULT_STATEMENT IF_STATEMENT IF_ELSE_STATEMENT SWITCH_STATEMENT
%token WHILE_STATEMENT DO_WHILE_STATEMENT FOR_STATEMENT

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token BOOL COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELLIPSIS EMPTY

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token PROGRAM PRIMARY_EXPRESSION POSTFIX_EXPRESSION ARGUMENT_EXPRESSION_LIST
%token UNARY_EXPRESSION UNARY_OPERATOR CAST_EXPRESSION MULTIPLICATIVE_EXPRESSION
%token ADDITIVE_EXPRESSION SHIFT_EXPRESSION RELATIONAL_EXPRESSION EQUALITY_EXPRESSION
%token AND_EXPRESSION EXCLUSIVE_OR_EXPRESSION INCLUSIVE_OR_EXPRESSION
%token LOGICAL_AND_EXPRESSION LOGICAL_OR_EXPRESSION CONDITIONAL_EXPRESSION
%token ASSIGNMENT_EXPRESSION ASSIGNMENT_OPERATOR EXPRESSION CONSTANT_EXPRESSION
%token DECLARATION DECLARATION_SPECIFIERS INIT_DECLARATOR_LIST INIT_DECLARATOR INIT_DECLARATOR_ASSIGNED
%token STORAGE_CLASS_SPECIFIER TYPE_SPECIFIER STRUCT_OR_UNION_SPECIFIER
%token STRUCT_OR_UNION STRUCT_DECLARATION_LIST STRUCT_DECLARATION
%token SPECIFIER_QUALIFIER_LIST STRUCT_DECLARATOR_LIST STRUCT_DECLARATOR
%token ENUM_SPECIFIER ENUMERATOR_LIST ENUMERATOR TYPE_QUALIFIER FUNCTION_SPECIFIER
%token DECLARATOR DIRECT_DECLARATOR POINTER PARAMETER_TYPE_LIST
%token PARAMETER_LIST PARAMETER_DECLARATION IDENTIFIER_LIST
%token ABSTRACT_DECLARATOR DIRECT_ABSTRACT_DECLARATOR INITIALIZER INITIALIZER_LIST
%token DESIGNATION DESIGNATOR_LIST DESIGNATOR STATEMENT LABELED_STATEMENT
%token COMPOUND_STATEMENT BLOCK_ITEM_LIST BLOCK_ITEM EXPRESSION_STATEMENT
%token SELECTION_STATEMENT ITERATION_STATEMENT GOTO_STATEMENT CONTINUE_STATEMENT BREAK_STATEMENT RETURN_STATEMENT TRANSLATION_UNIT
%token EXTERNAL_DECLARATION FUNCTION_DEFINITION DECLARATION_LIST PARAM_DECLARATION
%token ENUMERATOR_ASSIGNED POINTER_DECLARATOR

%token INTCONSTANT FLOATCONSTANT
%start program

%parse-param { struct ast_node** root}

%union {
	int i;
	float f;
	char *str;
	struct ast_node* node;
}

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
%type <node> function_specifier
%type <node> declarator
%type <node> direct_declarator
%type <node> pointer
%type <node> type_qualifier_list
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

program: translation_unit { *root = $1;}

primary_expression
	: IDENTIFIER { $$ = new_ast_node_name(IDENTIFIER, yylval.str, 0); }
	| INTCONSTANT { $$ = new_ast_node(INTCONSTANT, 0); $$->data.i = yylval.i; }
	| FLOATCONSTANT { $$ = new_ast_node_name(FLOATCONSTANT, 0); $$->data.f = yylval.f; }
	| STRING_LITERAL { $$ = new_ast_node_name(STRING_LITERAL, yylval.str, 0); }
	| '(' expression ')'{ $$ = $2; }
	;

postfix_expression
	: primary_expression { $$ = $1;}
	| postfix_expression '[' expression ']'	{ $$ = $1; append_child($1, new_ast_node(POSTFIX_ARRAY, $3, 0)); }
	| postfix_expression '(' ')' { $$ = $1; append_child($1, new_ast_node(POSTFIX_CALL, 0)); }
	| postfix_expression '(' argument_expression_list ')' { $$ = $1; append_child($1, new_ast_node(POSTFIX_CALL, $3, 0)); }
	| postfix_expression '.' IDENTIFIER { $$ = $1; append_child($1, new_ast_node(POSTFIX_MEMBER, $1, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0)); }
	| postfix_expression PTR_OP IDENTIFIER { $$ = $1; append_child($1, new_ast_node(POSTFIX_PTR_MEMBER, $1, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0)); }
	| postfix_expression INC_OP { $$ = $1; append_child($1, new_ast_node(POSTFIX_INC, $1, 0)); }
	| postfix_expression DEC_OP { $$ = $1; append_child($1, new_ast_node(POSTFIX_DEC, $1, 0)); }
	| '(' type_name ')' '{' initializer_list '}' { $$ = new_ast_node(COMPOUND_LITERAL, $2, $5, 0); }
	| '(' type_name ')' '{' initializer_list ',' '}' { $$ = new_ast_node(COMPOUND_LITERAL, $2, $5, 0); }
	;

argument_expression_list
	: assignment_expression { $$ = $1; }
	| argument_expression_list ',' assignment_expression { append_child($1, $3); $$ = $1;}
	;

unary_expression
	: postfix_expression { $$ = $1; }
	| INC_OP unary_expression { $$ = new_ast_node(PREFIX_INC, $2, 0);}
	| DEC_OP unary_expression { $$ = new_ast_node(PREFIX_DEC, $2, 0);}
	| unary_operator cast_expression { $$ = new_ast_node($1->code, $2, 0); free($1);}
	| SIZEOF unary_expression { $$ = new_ast_node(SIZEOF_EXPR, $2, 0);}
	| SIZEOF '(' type_name ')' { $$ = new_ast_node(SIZEOF_TYPE, $3, 0);}
	;

unary_operator
	: '&' { $$ = new_ast_node('&', 0); }
	| '*' { $$ = new_ast_node('*', 0); }
	| '+' { $$ = new_ast_node('+', 0); }
	| '-' { $$ = new_ast_node('-', 0); }
	| '~' { $$ = new_ast_node('~', 0); }
	| '!' { $$ = new_ast_node('!', 0); }
	;

cast_expression
	: unary_expression { $$ = $1;}
	| '(' type_name ')' cast_expression { $$ = new_ast_node(CAST, $2, $4, 0); }
	;

multiplicative_expression
	: cast_expression { $$ = $1;}
	| multiplicative_expression '*' cast_expression { $$ = new_ast_node('*', $1, $3, 0); }
	| multiplicative_expression '/' cast_expression { $$ = new_ast_node('/', $1, $3, 0); }
	| multiplicative_expression '%' cast_expression { $$ = new_ast_node('%', $1, $3, 0); }
	;

additive_expression
	: multiplicative_expression { $$ = $1;}
	| additive_expression '+' multiplicative_expression { $$ = new_ast_node('+', $1, $3, 0); }
	| additive_expression '-' multiplicative_expression { $$ = new_ast_node('-', $1, $3, 0); }
	;

shift_expression
	: additive_expression{ $$ = $1;}
	| shift_expression LEFT_OP additive_expression { $$ = new_ast_node(LEFT_OP, $1, $3, 0); }
	| shift_expression RIGHT_OP additive_expression { $$ = new_ast_node(RIGHT_OP, $1, $3, 0); }
	;

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression { $$ = new_ast_node('<', $1, $3, 0); }
	| relational_expression '>' shift_expression { $$ = new_ast_node('>', $1, $3, 0); }
	| relational_expression LE_OP shift_expression { $$ = new_ast_node(LE_OP, $1, $3, 0); }
	| relational_expression GE_OP shift_expression { $$ = new_ast_node(GE_OP, $1, $3, 0); }
	;

equality_expression
	: relational_expression { $$ = $1;}
	| equality_expression EQ_OP relational_expression { $$ = new_ast_node(EQ_OP, $1, $3, 0); }
	| equality_expression NE_OP relational_expression { $$ = new_ast_node(NE_OP, $1, $3, 0); }
	;

and_expression
	: equality_expression { $$ = $1;}
	| and_expression '&' equality_expression { $$ = new_ast_node(AND, $1, $3, 0); }
	;

exclusive_or_expression
	: and_expression{ $$ = $1;}
	| exclusive_or_expression '^' and_expression { $$ = new_ast_node(EXCLUSIVE_OR, $1, $3, 0); }
	;

inclusive_or_expression
	: exclusive_or_expression { $$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression { $$ = new_ast_node(INCLUSIVE_OR, $1, $3, 0); }
	;

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = new_ast_node(AND_OP, $1, $3, 0); }
	;

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression { $$ = new_ast_node(OR_OP, $1, $3, 0); }
	;

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = new_ast_node(CONDITIONAL, $1, $3, $5, 0); }
	;

assignment_expression
	: conditional_expression { $$ = $1; }
	| unary_expression assignment_operator assignment_expression { $$ = new_ast_node($2->code, $1, $3, 0); free($2); }
	;

assignment_operator
	: '=' { $$ = new_ast_node('=', 0); }
	| MUL_ASSIGN { $$ = new_ast_node(MUL_ASSIGN, 0); }
	| DIV_ASSIGN { $$ = new_ast_node(DIV_ASSIGN, 0); }
	| MOD_ASSIGN { $$ = new_ast_node(MOD_ASSIGN, 0); }
	| ADD_ASSIGN { $$ = new_ast_node(ADD_ASSIGN, 0); }
	| SUB_ASSIGN { $$ = new_ast_node(SUB_ASSIGN, 0); }
	| LEFT_ASSIGN { $$ = new_ast_node(LEFT_ASSIGN, 0); }
	| RIGHT_ASSIGN { $$ = new_ast_node(RIGHT_ASSIGN, 0); }
	| AND_ASSIGN { $$ = new_ast_node(AND_ASSIGN, 0); }
	| XOR_ASSIGN { $$ = new_ast_node(XOR_ASSIGN, 0); }
	| OR_ASSIGN { $$ = new_ast_node(OR_ASSIGN, 0); }
	;

expression
	: assignment_expression { $$ = $1; }
	| expression ',' assignment_expression { $$ = $1; append_child($1, $3); }
	;

constant_expression
	: conditional_expression { $$ = $1; }
	;

declaration
	: declaration_specifiers ';' { $$ = new_ast_node(DECLARATION, $1, 0); }
	| declaration_specifiers init_declarator_list ';' { $$ = new_ast_node(DECLARATION, $1, $2, 0); }
	;

declaration_specifiers
	: storage_class_specifier { $$ = $1; }
	| declaration_specifiers storage_class_specifier { append_child($1, $2); $$ = $1; }
	| type_specifier { $$ = $1; }
	| declaration_specifiers type_specifier { append_child($1, $2); $$ = $1; }
	| type_qualifier { $$ = $1; }
	| declaration_specifiers type_qualifier { append_child($1, $2); $$ = $1; }
	| function_specifier { $$ = $1; }
	| declaration_specifiers function_specifier { append_child($1, $2); $$ = $1; }
	;

init_declarator_list
	: init_declarator { $$ = $1; }
	| init_declarator_list ',' init_declarator { append_child($1, $3); $$ = $1; }
	;

init_declarator
	: declarator { $$ = new_ast_node(INIT_DECLARATOR, $1, 0);}
	| declarator '=' initializer { $$ = new_ast_node(INIT_DECLARATOR_ASSIGNED, $1, $3, 0);}
	;

storage_class_specifier
	: TYPEDEF { $$ = new_ast_node(TYPEDEF, 0); }
	| EXTERN { $$ = new_ast_node(EXTERN, 0); }
	| STATIC { $$ = new_ast_node(STATIC, 0); }
	| AUTO { $$ = new_ast_node(AUTO, 0); }
	| REGISTER { $$ = new_ast_node(REGISTER, 0); }
	;

type_specifier
	: VOID { $$ = new_ast_node(VOID, 0); }
	| CHAR { $$ = new_ast_node(CHAR, 0); }
	| SHORT { $$ = new_ast_node(SHORT, 0); }
	| INT { $$ = new_ast_node(INT, 0); }
	| LONG { $$ = new_ast_node(LONG, 0); }
	| FLOAT { $$ = new_ast_node(FLOAT, 0); }
	| DOUBLE { $$ = new_ast_node(DOUBLE, 0); }
	| SIGNED { $$ = new_ast_node(SIGNED, 0); }
	| UNSIGNED { $$ = new_ast_node(UNSIGNED, 0); }
	| BOOL { $$ = new_ast_node(BOOL, 0); }
	| COMPLEX { $$ = new_ast_node(COMPLEX, 0); }
	| IMAGINARY { $$ = new_ast_node(IMAGINARY, 0); }
	| struct_or_union_specifier { $$ = $1; }
	| enum_specifier { $$ = $1; }
	| TYPE_NAME { $$ = new_ast_node_name(TYPE_NAME, yylval.str, 0); }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}' { $$ = new_ast_node($1->code, $4, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); free($1); }
	| struct_or_union '{' struct_declaration_list '}' { $$ = new_ast_node($1->code, $3, 0); free($1); }
	| struct_or_union IDENTIFIER { $$ = new_ast_node($1->code, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); free($1); }
	;

struct_or_union
	: STRUCT { $$ = new_ast_node(STRUCT, 0); }
	| UNION { $$ = new_ast_node(UNION, 0); }
	;

struct_declaration_list
	: struct_declaration { $$ = $1;}
	| struct_declaration_list struct_declaration { append_child($1, $2); $$ = $1; }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' { $$ = new_ast_node(STRUCT_DECLARATION, $1, $2, 0); }
	;

specifier_qualifier_list
	: type_specifier { $$ = $1; }
	| specifier_qualifier_list type_specifier { append_child($1, $2); $$ = $1; }
	| type_qualifier { $$ = $1; }
	| specifier_qualifier_list type_qualifier { append_child($1, $2); $$ = $1; }
	;

struct_declarator_list
	: struct_declarator { $$ = $1; }
	| struct_declarator_list ',' struct_declarator { append_child($1, $3); $$ = $1;}
	;

struct_declarator
	: declarator { $$ = $1; }
	| ':' constant_expression { $$ = new_ast_node(BITFIELD, $2, 0); }
	| declarator ':' constant_expression { $$ = new_ast_node(BITFIELD, $1, $3, 0); }
	;

enum_specifier
	: ENUM '{' enumerator_list '}' { $$ = new_ast_node(ENUM_SPECIFIER, $3, 0); }
	| ENUM IDENTIFIER '{' enumerator_list '}' { $$ = new_ast_node(ENUM_SPECIFIER, new_ast_node_name(IDENTIFIER, yylval.str, 0), $4, 0); }
	| ENUM '{' enumerator_list ',' '}' { $$ = new_ast_node(ENUM_SPECIFIER, $3, 0); }
	| ENUM IDENTIFIER '{' enumerator_list ',' '}' { $$ = new_ast_node(ENUM_SPECIFIER, new_ast_node_name(IDENTIFIER, yylval.str, 0) ,$4, 0); }
	| ENUM IDENTIFIER { $$ = new_ast_node(ENUM_SPECIFIER, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); }
	;

enumerator_list
	: enumerator { $$ = $1; }
	| enumerator_list ',' enumerator { append_child($1, $3); $$ = $1; }
	;

enumerator
	: IDENTIFIER { $$ = new_ast_node(ENUMERATOR, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); }
	| IDENTIFIER '=' constant_expression { $$ = new_ast_node(ENUMERATOR_ASSIGNED, new_ast_node_name(IDENTIFIER, yylval.str, 0), $3, 0); }
	;

type_qualifier
	: CONST { $$ = new_ast_node(CONST, 0); }
	| RESTRICT { $$ = new_ast_node(RESTRICT, 0); }
	| VOLATILE { $$ = new_ast_node(VOLATILE, 0); }
	;

function_specifier
	: INLINE { $$ = new_ast_node(INLINE, 0); }
	;

declarator
	: pointer direct_declarator { $$ = new_ast_node(DECLARATOR, $1, $2, 0); }
	| direct_declarator { $$ = new_ast_node(DECLARATOR, $1, 0); }
	;


direct_declarator
	: IDENTIFIER { $$ = new_ast_node(DIRECT_DECLARATOR, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); }
	| '(' declarator ')'{ $$ = $2;}
	| direct_declarator '[' type_qualifier_list assignment_expression ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, $3, $4, 0);}
	| direct_declarator '[' type_qualifier_list ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, $3, 0);}
	| direct_declarator '[' assignment_expression ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, $3, 0);}
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, new_ast_node(STATIC, 0), $4, $5, 0);}
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, $3, new_ast_node(STATIC, 0), $5, 0);}
	| direct_declarator '[' type_qualifier_list '*' ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, $3, new_ast_node('*', 0), 0);}
	| direct_declarator '[' '*' ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, new_ast_node('*', 0), 0);}
	| direct_declarator '[' ']'	{ $$ = new_ast_node(ARRAY_DECLARATOR, $1, 0);}
	| direct_declarator '(' parameter_type_list ')'{ $$ = new_ast_node(FUNCTION_DECLARATOR, $1, $3, 0);}
	| direct_declarator '(' identifier_list ')'{ $$ = new_ast_node(FUNCTION_DECLARATOR, $1, $3, 0);}
	| direct_declarator '(' ')'{ $$ = new_ast_node(FUNCTION_DECLARATOR, $1, 0);}
	;

pointer
	: '*' { $$ = new_ast_node(POINTER, 0); }
	| '*' type_qualifier_list { $$ = new_ast_node(POINTER, $2, 0); }
	| '*' pointer { append_child($2, new_ast_node(POINTER, 0)); $$ = $2; }
	| '*' type_qualifier_list pointer { append_child($3, new_ast_node(POINTER, $2, 0)); $$ = $3; }
	;

type_qualifier_list
	: type_qualifier { $$ = $1; }
	| type_qualifier_list type_qualifier { append_child($1, $2); $$ = $1; }
	;


parameter_type_list
	: parameter_list { $$ = $1; }
	| parameter_list ',' ELLIPSIS { append_child($1, new_ast_node(ELLIPSIS, 0)); $$ = $1;}
	;

parameter_list
	: parameter_declaration { $$ = $1; }
	| parameter_list ',' parameter_declaration { append_child($1, $3); $$ = $1; }
	;

parameter_declaration
	: declaration_specifiers declarator { $$ = new_ast_node(PARAM_DECLARATION, $1, $2, 0); }
	| declaration_specifiers abstract_declarator { $$ = new_ast_node(PARAM_DECLARATION, $1, $2, 0); }
	| declaration_specifiers { $$ = new_ast_node(PARAM_DECLARATION, $1, 0); }
	;

identifier_list
	: IDENTIFIER { $$ = new_ast_node_name(IDENTIFIER, yylval.str, 0); }
	| identifier_list ',' IDENTIFIER { append_child($1, new_ast_node_name(IDENTIFIER, yylval.str, 0)); $$ = $1;}
	;

type_name
	: specifier_qualifier_list { $$ = new_ast_node(TYPE_NAME, $1, 0); }
	| specifier_qualifier_list abstract_declarator { $$ = new_ast_node(TYPE_NAME, $1, $2, 0); }
	;

abstract_declarator
	: pointer { $$ = new_ast_node(ABSTRACT_DECLARATOR, $1, 0);}
	| direct_abstract_declarator { $$ = new_ast_node(ABSTRACT_DECLARATOR, $1, 0); }
	| pointer direct_abstract_declarator { $$ = new_ast_node(ABSTRACT_DECLARATOR, $1, $2, 0); }
	;

direct_abstract_declarator
    : '(' abstract_declarator ')' { $$ = new_ast_node(DECLARATOR, $2, 0); }
    | '[' ']' { $$ = new_ast_node(ARRAY_DECLARATOR, 0); }
    | '[' assignment_expression ']' { $$ = new_ast_node(ARRAY_DECLARATOR, $2, 0); }
    | direct_abstract_declarator '[' ']' { $$ = new_ast_node(ARRAY_DECLARATOR, $1, 0); }
    | direct_abstract_declarator '[' assignment_expression ']' { $$ = new_ast_node(ARRAY_DECLARATOR, $1, $3); }
    | '[' '*' ']' { $$ = new_ast_node(POINTER_DECLARATOR, 0); }
    | direct_abstract_declarator '[' '*' ']' { $$ = new_ast_node(POINTER_DECLARATOR, $1, 0); }
    | '(' ')' { $$ = new_ast_node(DECLARATOR, 0); }
    | '(' parameter_type_list ')' { $$ = new_ast_node(FUNCTION_DECLARATOR, $2, 0); }
    | direct_abstract_declarator '(' ')' { $$ = new_ast_node(FUNCTION_DECLARATOR, $1, 0); }
    | direct_abstract_declarator '(' parameter_type_list ')' { $$ = new_ast_node(FUNCTION_DECLARATOR, $1, $3); }
    ;

initializer
	: assignment_expression { $$ = $1; }
	| '{' initializer_list '}'{ $$ = $2; }
	| '{' initializer_list ',' '}'{ $$ = $2; }
	;

initializer_list
	: initializer { $$ = $1; }
	| designation initializer { $$ = new_ast_node(DESIGNATED_INITITALIZER, $1, $2, 0); }
	| initializer_list ',' initializer { append_child($1, $3); $$ = $1;}
	| initializer_list ',' designation initializer { append_child($1, new_ast_node(DESIGNATED_INITITALIZER, $3, $4, 0)); $$ = $1;}
	;

designation
	: designator_list '=' { $$ = $1; }
	;

designator_list
	: designator { $$ = $1; }
	| designator_list designator { append_child($1, $2); $$ = $1; }
	;

designator
	: '[' constant_expression ']' { $$ = new_ast_node(ARRAY_DESIGNATOR, $2, 0); }
	| '.' IDENTIFIER { $$ = new_ast_node(MEMBER_DESIGNATOR, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); }
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
	: IDENTIFIER ':' statement { $$ = new_ast_node(LABELED_STATEMENT, new_ast_node_name(IDENTIFIER, yylval.str, 0), $3, 0); }
	| CASE constant_expression ':' statement { $$ = new_ast_node(CASE_STATEMENT, $2, $4, 0); }
	| DEFAULT ':' statement { $$ = new_ast_node(DEFAULT_STATEMENT, $3, 0); }
	;

compound_statement
	: '{' '}' { $$ = new_ast_node(COMPOUND_STATEMENT, 0); }
	| '{' block_item_list '}' { $$ = new_ast_node(COMPOUND_STATEMENT, $2, 0); }
	;

block_item_list
	: block_item { $$ = $1; }
	| block_item_list block_item { append_child($1, $2); $$ = $1; }
	;

block_item
	: declaration { $$ = $1; }
	| statement { $$ = $1; }
	;

expression_statement
	: ';' { $$ = empty_node(); }
	| expression ';' { $$ = $1; }
	;

selection_statement
	: IF '(' expression ')' statement { $$ = new_ast_node(IF_STATEMENT, $3, $5, 0); }
	| IF '(' expression ')' statement ELSE statement { $$ = new_ast_node(IF_ELSE_STATEMENT, $3, $5, $7, 0); }
	| SWITCH '(' expression ')' statement { $$ = new_ast_node(SWITCH_STATEMENT, $3, $5, 0); }
	;

iteration_statement
	: WHILE '(' expression ')' statement { $$ = new_ast_node(WHILE_STATEMENT, $3, $5, 0); }
	| DO statement WHILE '(' expression ')' ';' { $$ = new_ast_node(DO_WHILE_STATEMENT, $2, $5, 0); }
	| FOR '(' expression_statement expression_statement ')' statement { $$ = new_ast_node(FOR_STATEMENT, $3, $4, empty_node(), $6, 0); }
	| FOR '(' expression_statement expression_statement expression ')' statement { $$ = new_ast_node(FOR_STATEMENT, $3, $4, $5, $7, 0); }
	| FOR '(' declaration expression_statement ')' statement { $$ = new_ast_node(FOR_STATEMENT, $3, $4, empty_node(), $6, 0); }
	| FOR '(' declaration expression_statement expression ')' statement { $$ = new_ast_node(FOR_STATEMENT, $3, $4, $5, $7, 0); }
	;

jump_statement
	: GOTO IDENTIFIER ';' { $$ = new_ast_node(GOTO_STATEMENT, new_ast_node_name(IDENTIFIER, yylval.str, 0), 0); }
	| CONTINUE ';' { $$ = new_ast_node(CONTINUE_STATEMENT, 0); }
	| BREAK ';' { $$ = new_ast_node(BREAK_STATEMENT, 0); }
	| RETURN ';' { $$ = new_ast_node(RETURN_STATEMENT, 0); }
	| RETURN expression ';'{ $$ = new_ast_node(RETURN_STATEMENT, $2, 0);}
	;

translation_unit
	: external_declaration { $$ = $1; }
	| translation_unit external_declaration { append_child($1, $2); $$ = $1; }
	;

external_declaration
	: function_definition { $$ = $1; }
	| declaration { $$ = $1; }
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement { $$ = new_ast_node(FUNCTION_DEFINITION, $1, $2, $3, $4, 0); }
	| declaration_specifiers declarator compound_statement { $$ = new_ast_node(FUNCTION_DEFINITION, $1, $2, $3, 0); }
	;

declaration_list
	: declaration { $$ = new_ast_node(DECLARATION_LIST, $1, 0); }
	| declaration_list declaration { append_child($1, $2); $$ = $1; }
	;


%%


#include <stdio.h>

extern int column;

void yyerror(struct ast_node **root, char const *s)
{
	fflush(stdout);
	printf("%*s\n%*s\n", column, "^", column, s);
}
const char* to_ast_string(int code){
	return yysymbol_name(YYTRANSLATE(code));
}
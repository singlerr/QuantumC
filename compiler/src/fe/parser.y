%{
#include "ast.h"
#include <string.h>
void setstr(struct ast_node* n, const char* str){
	int len = strlen(str);
	n->data.str = (const char*) malloc(sizeof(char) * (len + 1));
	strcpy(n->data.str, str);
}
%}


%define parse.trace
%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE RESTRICT
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token BOOL COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%start program

%parse-param { struct ast_node** root }
%define api.value.type union

%type <struct ast_node *> program
%type <struct ast_node *> primary_expression
%type <struct ast_node *> postfix_expression
%type <struct ast_node *> argument_expression_list
%type <struct ast_node *> unary_expression
%type <struct ast_node *> unary_operator
%type <struct ast_node *> cast_expression
%type <struct ast_node *> multiplicative_expression
%type <struct ast_node *> additive_expression
%type <struct ast_node *> shift_expression
%type <struct ast_node *> relational_expression
%type <struct ast_node *> equality_expression
%type <struct ast_node *> and_expression
%type <struct ast_node *> exclusive_or_expression
%type <struct ast_node *> inclusive_or_expression
%type <struct ast_node *> logical_and_expression
%type <struct ast_node *> logical_or_expression
%type <struct ast_node *> conditional_expression
%type <struct ast_node *> assignment_expression
%type <struct ast_node *> assignment_operator
%type <struct ast_node *> expression
%type <struct ast_node *> constant_expression
%type <struct ast_node *> declaration
%type <struct ast_node *> declaration_specifiers
%type <struct ast_node *> init_declarator_list
%type <struct ast_node *> init_declarator
%type <struct ast_node *> storage_class_specifier
%type <struct ast_node *> type_specifier
%type <struct ast_node *> struct_or_union_specifier
%type <struct ast_node *> struct_or_union
%type <struct ast_node *> struct_declaration_list
%type <struct ast_node *> struct_declaration
%type <struct ast_node *> specifier_qualifier_list
%type <struct ast_node *> struct_declarator_list
%type <struct ast_node *> struct_declarator
%type <struct ast_node *> enum_specifier
%type <struct ast_node *> enumerator_list
%type <struct ast_node *> enumerator
%type <struct ast_node *> type_qualifier
%type <struct ast_node *> function_specifier
%type <struct ast_node *> declarator
%type <struct ast_node *> direct_declarator
%type <struct ast_node *> pointer
%type <struct ast_node *> type_qualifier_list
%type <struct ast_node *> parameter_type_list
%type <struct ast_node *> parameter_list
%type <struct ast_node *> parameter_declaration
%type <struct ast_node *> identifier_list
%type <struct ast_node *> type_name
%type <struct ast_node *> abstract_declarator
%type <struct ast_node *> direct_abstract_declarator
%type <struct ast_node *> initializer
%type <struct ast_node *> initializer_list
%type <struct ast_node *> designation
%type <struct ast_node *> designator_list
%type <struct ast_node *> designator
%type <struct ast_node *> statement
%type <struct ast_node *> labeled_statement
%type <struct ast_node *> compound_statement
%type <struct ast_node *> block_item_list
%type <struct ast_node *> block_item
%type <struct ast_node *> expression_statement
%type <struct ast_node *> selection_statement
%type <struct ast_node *> iteration_statement
%type <struct ast_node *> jump_statement
%type <struct ast_node *> translation_unit
%type <struct ast_node *> external_declaration
%type <struct ast_node *> function_definition
%type <struct ast_node *> declaration_list

%%

program: translation_unit { *root = $1; };

primary_expression
	: IDENTIFIER
	| CONSTANT
	| STRING_LITERAL
	| '(' expression ')'
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'
	| postfix_expression '(' ')'
	| postfix_expression '(' argument_expression_list ')'
	| postfix_expression '.' IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP
	| postfix_expression DEC_OP
	| '(' type_name ')' '{' initializer_list '}'
	| '(' type_name ')' '{' initializer_list ',' '}'
	;

argument_expression_list
	: assignment_expression
	| argument_expression_list ',' assignment_expression
	;

unary_expression
	: postfix_expression
	| INC_OP unary_expression
	| DEC_OP unary_expression
	| unary_operator cast_expression
	| SIZEOF unary_expression
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&'
	| '*'
	| '+'
	| '-'
	| '~'
	| '!'
	;

cast_expression
	: unary_expression
	| '(' type_name ')' cast_expression
	;

multiplicative_expression
	: cast_expression
	| multiplicative_expression '*' cast_expression
	| multiplicative_expression '/' cast_expression
	| multiplicative_expression '%' cast_expression
	;

additive_expression
	: multiplicative_expression
	| additive_expression '+' multiplicative_expression
	| additive_expression '-' multiplicative_expression
	;

shift_expression
	: additive_expression
	| shift_expression LEFT_OP additive_expression
	| shift_expression RIGHT_OP additive_expression
	;

relational_expression
	: shift_expression
	| relational_expression '<' shift_expression
	| relational_expression '>' shift_expression
	| relational_expression LE_OP shift_expression
	| relational_expression GE_OP shift_expression
	;

equality_expression
	: relational_expression
	| equality_expression EQ_OP relational_expression
	| equality_expression NE_OP relational_expression
	;

and_expression
	: equality_expression
	| and_expression '&' equality_expression
	;

exclusive_or_expression
	: and_expression
	| exclusive_or_expression '^' and_expression
	;

inclusive_or_expression
	: exclusive_or_expression
	| inclusive_or_expression '|' exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression
	| logical_and_expression AND_OP inclusive_or_expression
	;

logical_or_expression
	: logical_and_expression
	| logical_or_expression OR_OP logical_and_expression
	;

conditional_expression
	: logical_or_expression
	| logical_or_expression '?' expression ':' conditional_expression
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression
	;

assignment_operator
	: '='
	| MUL_ASSIGN
	| DIV_ASSIGN
	| MOD_ASSIGN
	| ADD_ASSIGN
	| SUB_ASSIGN
	| LEFT_ASSIGN
	| RIGHT_ASSIGN
	| AND_ASSIGN
	| XOR_ASSIGN
	| OR_ASSIGN
	;

expression
	: assignment_expression
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression
	;

declaration
	: declaration_specifiers ';' { $$ = new_ast_node(DECLARATION_SPECIFIERS, $1, 0); }
	| declaration_specifiers init_declarator_list ';' { $$ = new_ast_node(DECLARATION_SPECIFIERS, $1, $2, 0); }
	;

declaration_specifiers
	: storage_class_specifier { $$ = new_ast_node(STORAGE_CLASS_SPECIFIER, $1, 0); }
	| storage_class_specifier declaration_specifiers { append_child($1, $2); $$ = $2; }
	| type_specifier { $$ = new_ast_node(TYPE_SPECIFIER, $1, 0); }
	| type_specifier declaration_specifiers { append_child($1, $2); $$ = $2; }
	| type_qualifier { $$ = new_ast_node(TYPE_QUALIFIER, $1), 0; }
	| type_qualifier declaration_specifiers { append_child($1, $2); $$ = $2; }
	| function_specifier { $$ = new_ast_node(FUNCTION_SPECIFIER, $1, 0); }
	| function_specifier declaration_specifiers { append_child($1, $2); $$ = $2; }
	;

init_declarator_list
	: init_declarator { $$ = $1; }
	| init_declarator_list ',' init_declarator { append_child($3, $1); $$ = $1; }
	;

init_declarator
	: declarator { $$ = new_ast_node(DECLARATOR, $1, 0); }
	| declarator '=' initializer { $$ = new_ast_node(DECLARATOR, $1, $3, 0); }
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
	| TYPE_NAME { $$ = new_ast_node(TYPE_NAME, 0); setstar($$, yytext); }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}' { $$ = new_ast_node($1->code, $4, 0); setstar($$, yytext); }
	| struct_or_union '{' struct_declaration_list '}' { $$ = new_ast_node($1->code, $3, 0); }
	| struct_or_union IDENTIFIER { $$ = new_ast_node($1->code, 0); setstar($$, yytext); }
	;

struct_or_union
	: STRUCT { $$ = new_ast_node(STRUCT, 0); }
	| UNION { $$ = new_ast_node(UNION, 0); }
	;

struct_declaration_list
	: struct_declaration { $$ = $1; }
	| struct_declaration_list struct_declaration { append_child($2, $1); $$ = $1; }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' { $$ = new_ast_node(STRUCT_DECLARATION, $1, $2, 0); }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list { append_child($1, $2); $$ = $2; }
	| type_specifier { $$ = new_ast_node(TYPE_SPECIFIER, $1, 0); }
	| type_qualifier specifier_qualifier_list { append_child($1, $2); $$ = $2; }
	| type_qualifier { $$ = new_ast_node(TYPE_QUALIFIER, $1, 0); }
	;

struct_declarator_list
	: struct_declarator { $$ = new_ast_node(STRUCT_DECLARATOR, $1, 0); }
	| struct_declarator_list ',' struct_declarator { append_child($3, $1); $$ = $1; }
	;

struct_declarator
	: declarator { $$ = $1; }
	| ':' constant_expression { $$ = $2; }
	| declarator ':' constant_expression { append_child($1, $3); $$ = $3; }
	;

enum_specifier
	: ENUM '{' enumerator_list '}' { $$ = new_ast_node(ENUM_SPECIFIER, $3, 0); }
	| ENUM IDENTIFIER '{' enumerator_list '}' { $$ = new_ast_node(ENUM_SPECIFIER, $4, 0); setstr($$, yytext); }
	| ENUM '{' enumerator_list ',' '}' { $$ = new_ast_node(ENUM_SPECIFIER, $3, 0); setstr($$, yytext); }
	| ENUM IDENTIFIER '{' enumerator_list ',' '}' { $$ = new_ast_node(ENUM_SPECIFIER, $4, 0); setstr($$, yytext); }
	| ENUM IDENTIFIER { $$ = new_ast_node(ENUM_SPECIFIER, 0); setstr($$, yytext); }
	;

enumerator_list
	: enumerator { $$ = $1; }
	| enumerator_list ',' enumerator { append_child($3, $1); $$ = $1; }
	;

enumerator
	: IDENTIFIER { $$ = new_ast_node(ENUMERATOR, 0); setstr($$, yytext); }
	| IDENTIFIER '=' constant_expression { $$ = new_ast_node(ENUMERATOR, $3, 0); setstr($$, yytext); }
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
	| direct_declarator { $$ = new_ast_node(DELCARATOR, $1, 0); }
	;


direct_declarator
	: IDENTIFIER { $$ = new_ast_node(DIRECT_DECLARATOR, 0); setstr($$, yytext); }
	| '(' declarator ')' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, 0); }
	| direct_declarator '[' type_qualifier_list assignment_expression ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, $4, 0); }
	| direct_declarator '[' type_qualifier_list ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, 0); }
	| direct_declarator '[' assignment_expression ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, 0); }
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, $4, $5, 0); }
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, $4, $5, 0); }
	| direct_declarator '[' type_qualifier_list '*' ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, 0); }
	| direct_declarator '[' '*' ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, 0); }
	| direct_declarator '[' ']' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, 0); }
	| direct_declarator '(' parameter_type_list ')' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, 0); }
	| direct_declarator '(' identifier_list ')' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, $3, 0); }
	| direct_declarator '(' ')' { $$ = new_ast_node(DIRECT_DECLARATOR, $1, 0); }
	;

pointer
	: '*' { $$ = new_ast_node(POINTER, 0); }
	| '*' type_qualifier_list { $$ = new_ast_node(POINTER, $2, 0); }
	| '*' pointer { $$ = new_ast_node(POINTER, $2, 0); }
	| '*' type_qualifier_list pointer { $$ = new_ast_node(POINTER, $2, $3, 0); }
	;

type_qualifier_list
	: type_qualifier { $$ = new_ast_node(TYPE_QUALIFIER, $1, 0); }
	| type_qualifier_list type_qualifier { append_child($2, $1); $$ = $1; }
	;


parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer direct_abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' assignment_expression ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' assignment_expression ']'
	| '[' '*' ']'
	| direct_abstract_declarator '[' '*' ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expression
	| '{' initializer_list '}'
	| '{' initializer_list ',' '}'
	;

initializer_list
	: initializer
	| designation initializer
	| initializer_list ',' initializer
	| initializer_list ',' designation initializer
	;

designation
	: designator_list '='
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: '[' constant_expression ']'
	| '.' IDENTIFIER
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

selection_statement
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	| FOR '(' declaration expression_statement ')' statement
	| FOR '(' declaration expression_statement expression ')' statement
	;

jump_statement
	: GOTO IDENTIFIER ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN ';'
	| RETURN expression ';'
	;

translation_unit
	: external_declaration { $$ = $1; }
	| translation_unit external_declaration { append_child($1, $2); $$ = $2; }
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
	: declaration { $$ = $1; }
	| declaration_list declaration { append_child($1, $2); $$ = $2; }
	;


%%
#include <stdio.h>

extern char yytext[];
extern int column;

void yyerror(char const *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}

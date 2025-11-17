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

struct ast_node* empty_node();

void yyerror(struct ast_node **root, char const *s);


extern int yylex();

#include "c.parser.h"

%}

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
%token ENUMERATOR_ASSIGNED POINTER_DECLARATOR END

%token INTCONSTANT FLOATCONSTANT
%start program

%parse-param 

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

program: translation_unit { *root = $1; }

primary_expression
	: IDENTIFIER 
	| INTCONSTANT 
	| FLOATCONSTANT 
	| STRING_LITERAL 
	| '(' expression ')' { $$ = $1; }
	;

postfix_expression
	: primary_expression { $$ = $1; }
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
	: declaration_specifiers ';' 
	| declaration_specifiers init_declarator_list ';' 
	;

declaration_specifiers
	: storage_class_specifier 
	| declaration_specifiers storage_class_specifier 
	| type_specifier 
	| declaration_specifiers type_specifier 
	| type_qualifier 
	| declaration_specifiers type_qualifier 
	| function_specifier 
	| declaration_specifiers function_specifier 
	;

init_declarator_list
	: init_declarator 
	| init_declarator_list ',' init_declarator 
	;

init_declarator
	: declarator 
	| declarator '=' initializer 
	;

storage_class_specifier
	: TYPEDEF 
	| EXTERN 
	| STATIC 
	| AUTO 
	| REGISTER 
	;

type_specifier
	: VOID 
	| CHAR 
	| SHORT 
	| INT 
	| LONG 
	| FLOAT 
	| DOUBLE 
	| SIGNED 
	| UNSIGNED 
	| BOOL 
	| COMPLEX 
	| IMAGINARY 
	| struct_or_union_specifier 
	| enum_specifier 
	| TYPE_NAME 
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}' 
	| struct_or_union '{' struct_declaration_list '}' 
	| struct_or_union IDENTIFIER 
	;

struct_or_union
	: STRUCT 
	| UNION 
	;

struct_declaration_list
	: struct_declaration 
	| struct_declaration_list struct_declaration 
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' 
	;

specifier_qualifier_list
	: type_specifier 
	| specifier_qualifier_list type_specifier 
	| type_qualifier 
	| specifier_qualifier_list type_qualifier 
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
	: ENUM '{' enumerator_list '}' 
	| ENUM IDENTIFIER '{' enumerator_list '}' 
	| ENUM '{' enumerator_list ',' '}' 
	| ENUM IDENTIFIER '{' enumerator_list ',' '}' 
	| ENUM IDENTIFIER 
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
	: CONST 
	| RESTRICT 
	| VOLATILE 
	;

function_specifier
	: INLINE 
	;

declarator
	: pointer direct_declarator 
	| direct_declarator 
	;


direct_declarator
	: IDENTIFIER 
	| '(' declarator ')'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'	
	| direct_declarator '[' type_qualifier_list ']'	
	| direct_declarator '[' assignment_expression ']'	
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'	
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'	
	| direct_declarator '[' type_qualifier_list '*' ']'	
	| direct_declarator '[' '*' ']'	
	| direct_declarator '[' ']'	
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
	;

pointer
	: '*' 
	| '*' type_qualifier_list 
	| '*' pointer 
	| '*' type_qualifier_list pointer 
	;

type_qualifier_list
	: type_qualifier 
	| type_qualifier_list type_qualifier 
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
	: external_declaration 
	| translation_unit external_declaration 
	;

external_declaration
	: function_definition 
	| declaration 
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement 
	| declaration_specifiers declarator compound_statement 
	;

declaration_list
	: declaration 
	| declaration_list declaration 
	;


%%


#include <stdio.h>

extern int column;

struct ast_node* empty_node(){
	return new_ast_node(END, 0);
}

void yyerror(struct ast_node **root, char const *s)
{
	fflush(stdout);
	printf("%*s\n%*s\n", column, "^", column, s);
}

const char* to_ast_string(int code){
	return yysymbol_name(YYTRANSLATE(code));
}
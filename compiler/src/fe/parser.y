%{
#include "ast.h"
#include "symrec.h"
#include <stdlib.h> // For free()
#include <string.h>

void yyerror(ast_node **root, char const *s);
extern int yylex();
#include "c.parser.h"

%}


%parse-param { ast_node** root}

%union {
	int i;
	float f;
	char *str;
	ast_node* node;
	ast_identifier_node* id_node;
}


%token <id_node> IDENTIFIER 
%token CONSTANT STRING_LITERAL SIZEOF
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

program: translation_unit

primary_expression
	: IDENTIFIER
	| INTCONSTANT
	| FLOATCONSTANT
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
	: declaration_specifiers ';' { $$ = AST_GENERAL_NODE(AST_VARIABLE_DECLARATION, $1, NULL, NULL); }
	| declaration_specifiers init_declarator_list ';' { $$ = AST_GENERAL_NODE(AST_VARIABLE_DECLARATION, $1, $2, NULL); }
	;

declaration_specifiers
	: storage_class_specifier { $$ = $1; }
	| declaration_specifiers storage_class_specifier { $$ = $1; append_left_child(find_last_left_child($1), $2); } 
	| type_specifier { $$ = $1; }
	| declaration_specifiers type_specifier { $$ = $1; append_middle_child(find_last_middle_child($1), $2); } 
	| type_qualifier { $$ = $1; }
	| declaration_specifiers type_qualifier { $$ = $1; append__child(find_last_left_child($1), $2); } 
	;

init_declarator_list
	: init_declarator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| init_declarator_list ',' init_declarator { $$ = $1; append_right_child(find_last_right_child($1), $3); }
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
	: VOID { $$ = AST_SIMPLE_NODE(AST_TYPE_VOID); }
	| CHAR { $$ = AST_SIMPLE_NODE(AST_TYPE_CHAR); }
	| SHORT { $$ = AST_SIMPLE_NODE(AST_TYPE_SHORT); }
	| INT { $$ = AST_SIMPLE_NODE(AST_TYPE_INT); }
	| LONG { $$ = AST_SIMPLE_NODE(AST_TYPE_LONG); }
	| FLOAT { $$ = AST_SIMPLE_NODE(AST_TYPE_FLOAT); } 
	| DOUBLE { $$ = AST_SIMPLE_NODE(AST_TYPE_DOUBLE); }
	| SIGNED { $$ = AST_SIMPLE_NODE(AST_TYPE_SIGNED); }
	| UNSIGNED { $$ = AST_SIMPLE_NODE(AST_TYPE_UNSIGNED); }
	| COMPLEX { $$ = AST_SIMPLE_NODE(AST_TYPE_COMPLEX); }
	| IMAGINARY { $$ = AST_SIMPLE_NODE(AST_TYPE_IMAGINARY); }
	| struct_or_union_specifier { $$ = AST_GENERAL_NODE(AST_TYPE_STRUCT_UNION, NULL, $1, NULL); }
	| enum_specifier { $$ = AST_GENERAL_NODE(AST_TYPE_ENUM, NULL, $1, NULL); }
	| TYPE_NAME { $$ = AST_TYPE_NODE(AST_TYPE_USER, gettype(yytext) ,NULL, NULL , NULL); }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yytext), NULL); } '{' { inc_scope_level(); } struct_declaration_list '}' { dec_scope_level();  $$ = AST_IDENTIFIER_NODE(AST_STRUCT_UNION, $2 ,$1, NULL, $3); }
	| struct_or_union '{' { inc_scope_level(); }  struct_declaration_list '}' { dec_scope_level(); $$ = AST_GENERAL_NODE(AST_STRUCT_UNION, $1, NULL, $4); }
	| struct_or_union IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_STRUCT_UNION, AST_ID_CURSCOPE(getorcreatesym(yytext), NULL), $1, NULL, NULL); }
	;

struct_or_union
	: STRUCT { $$ = AST_SIMPLE_NODE(AST_STRUCT); }
	| UNION { $$ = AST_SIMPLE_NODE(AST_UNION); }
	;

struct_declaration_list
	: struct_declaration { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| struct_declaration_list struct_declaration { $$ = $1; append_right_child(find_last_right_child($1), $2); }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';' { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATION, $1, $2, NULL); }
	;

specifier_qualifier_list
	: type_specifier { $$ = $1; }
	| specifier_qualifier_list type_specifier { $$ = $1; append_left_child(find_last_left_child($1), $2);  }
	| type_qualifier { $$ = $1; }
	| specifier_qualifier_list type_qualifier { $$ = $1; append_middle_child(find_last_middle_child($1), $2); }
	;

struct_declarator_list
	: struct_declarator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| struct_declarator_list ',' struct_declarator { $$ = $1; append_rught_child(find_last_right_child($1), $3); }
	;

struct_declarator
	: declarator { $$ = AST_GENERAL_NOPDE(AST_STRUCT_FIELD_DECLARATOR, NULL, $1, NULL); }
	| ':' constant_expression { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATOR, NULL, NULL, $2); }
	| declarator ':' constant_expression { $$ = AST_GENERAL_NODE(AST_STRUCT_FIELD_DECLARATOR, NULL, $1, $3); }
	;

enum_specifier
	: ENUM '{'  enumerator_list '}' { $$ = AST_GENERAL_NODE(AST_ENUM, NULL, $3, NULL); }
	| ENUM IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yytext), NULL); } '{' enumerator_list '}' { $$ = AST_IDENTIFIER_NODE(AST_ENUM, $2, NULL, NULL, NULL); }
	| ENUM '{' enumerator_list ',' '}' { $$ = AST_GENERAL_NODE(AST_ENUM, NULL, $3, NULL); }
	| ENUM IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yytext), NULL); } '{' enumerator_list ',' '}' { $$ = AST_IDENTIFIER_NODE(AST_ENUM, $2, NULL, $4, NULL); }
	| ENUM IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_ENUM, AST_ID_CURSCOPE(getorcreatesym(yytext), NULL), NULL, NULL, NULL); }
	;

enumerator_list
	: enumerator { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| enumerator_list ',' enumerator { $$ = $1; append_right_child(find_last_right_child($1), $3); }
	;

enumerator
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_ENUN_FIELD_DECLARATION, AST_ID_CURSCOPE(getorcreatesym(yytext), NULL), NULL, NULL, NULL); }
	| IDENTIFIER <id_node> { $$ = AST_ID_CURSCOPE(getorcreatesym(yytext), NULL); } '=' constant_expression { $$ = AST_IDENTIFIER_NODE(AST_ENUM_FIELD_DECLARATION, $1, NULL, $3, NULL); }
	;

type_qualifier
	: CONST { $$ = AST_SIMPLE_NODE(AST_QAL_CONST); }
	| RESTRICT { $$ = AST_SIMPLE_NODE(AST_QAL_RESTRICT); }
	| VOLATILE { $$ = AST_SIMPLE_NODE(AST_QAL_VOLATILE); }
	;

declarator
	: pointer direct_declarator { $$ = $1; append_middle_child(find_last_middle_child($1), $2); }
	| direct_declarator { $$ = $1; }
	;


direct_declarator
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(getorcreatesym(yytext), NULL), NULL, NULL, NULL); }
	| '(' declarator ')' { $$ = $1; }
	| direct_declarator '[' type_qualifier_list assignment_expression ']' {  }
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
	: '*' { $$ = AST_SIMPLE_NODE(AST_TYPE_POINTER); }
	| '*' type_qualifier_list { $$ = AST_GENERAL_NODE(AST_TYPE_POINTER, $2, NULL, NULL); }
	| '*' pointer { $$ = AST_GENERAL_NODE(AST_TYPE_POINTER, NULL, NULL, $2);}
	| '*' type_qualifier_list pointer { $$ = AST_GENERAL_NODE(AST_TYPE_POINTER, $2, NULL, $3); }
	;

type_qualifier_list
	: type_qualifier { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| type_qualifier_list type_qualifier { $$ = $1; append_right_child(find_last_right_child($1), $2); }
	;


parameter_type_list
	: parameter_list { $$ = $1; }
	| parameter_list ',' ELLIPSIS { perror("Sorry! Currently variadic parameters are not allowed."); }
	;

parameter_list
	: parameter_declaration { $$ = AST_GENERAL_NODE(AST_NODE_LIST, NULL, $1, NULL); }
	| parameter_list ',' parameter_declaration { $$ = $1; append_right_child(find_last_right_child($1), $3); }
	;

parameter_declaration
	: declaration_specifiers declarator { $$ = AST_GENERAL_NODE(AST_PARAMETER_DECLARATION, $2, NULL, NULL); }
	| declaration_specifiers abstract_declarator { $$ = AST_GENERAL_NODE(AST_PARAMETER_DECLARATION, NULL, $2, NULL); }
	| declaration_specifiers{ $$ = AST_GENERAL_NODE(AST_PARAMETER_DECLARATION, NULL, NULL, NULL); }
	;

identifier_list
	: IDENTIFIER { $$ = AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(yytext, NULL), NULL, NULL, NULL); }
	| identifier_list ',' IDENTIFIER { $$ = $1; append_right_child(find_last_right_child($1), AST_IDENTIFIER_NODE(AST_IDENTIFIER, AST_ID_CURSCOPE(yytext, NULL), NULL, NULL, NULL)); }
	;

type_name
	: specifier_qualifier_list { $$ = AST_GENERAL_NODE(AST_NAME_TYPE, $1, NULL, NULL); }
	| specifier_qualifier_list abstract_declarator { $$ = $1; append_right_child(find_last_right_child($1), $2); }
	;

abstract_declarator
	: pointer { $$ = $1; }
	| direct_abstract_declarator { $$ = $1; }
	| pointer direct_abstract_declarator { $$ = $1; append_right_child(find_last_right_child($1), $2); }
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

void yyerror(ast_node **root, char const *s)
{
	fflush(stdout);
	printf("%*s\n%*s\n", column, "^", column, s);
}

const char* to_ast_string(int code){
	return yysymbol_name(YYTRANSLATE(code));
}

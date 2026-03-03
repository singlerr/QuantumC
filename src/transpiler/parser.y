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
	ast_node** root
}

%code top {
	#include <stdio.h>
	#include <stdlib.h> 
	#include <string.h>
	#include "stringlib.h"
	#include "ast.h"
	#include "ast_internal.h"
	#include "type/deco.h"
	#include "preprocessor_link.h"
	#include "c.parser.h"
}

%code requires {
	typedef void* yyscan_t;
}

%code {
	void yyerror(YYLTYPE* yyllocp, yyscan_t unused, ast_node** root, char const *msg);
	int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
}

%code {
	
	ast_node* compile(FILE* input);
	int feed_and_parse(const char* content,  ast_node** out);
}

%union {
	int i;
	int constr;
	float f;
	char *str;
	ast_t* ast;
	type_t* type;
	ty_deco_t* ty_deco;
	ty_struct_t* ty_struct;
	pointer_t ptr;
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

%type<ast> primary_expression
%type<constr> type_qualifier
%type<constr> type_qualifier_list
%type<type> type_specifier
%type<struct_fields> struct_declaration_list
%type<struct_field> struct_declaration
%type<ast> declarator
%type<ptr> pointer;

%%

program: translation_unit

primary_expression
	: IDENTIFIER { $$ = new_ast_var(search_var(yylval.str)); }
	| INTCONSTANT { $$ = new_ast_literal(AST_INT, Int(yylval.i)); }
	| FLOATCONSTANT { $$ = new_ast_literal(AST_FLOAT, Float(yylval.f)); }
	/* | STRING_LITERAL  */
	| BOOL_TRUE { $$ = new_ast_literal(AST_BOOL, Bool(TRUE)); }
	| BOOL_FALSE { $$ = new_ast_literal(AST_BOOL, Bool(FALSE)); }
	| '(' expression ')' { $$ = $1; }
	;

postfix_expression 
	: primary_expression { $$ = $1; }
	| postfix_expression '[' expression ']' { $$ = new_ast_arr_access(ArrAccess($1, $2)); }
	| postfix_expression '(' ')' { $$ = new_ast_app(App($1, NULL)); }
	| postfix_expression '(' argument_expression_list ')' { $$ = new_ast_app(App($1, $2)); }
	| postfix_expression '.' IDENTIFIER { $$ = from_struct_member($1, yylval.str); }
	| postfix_expression PTR_OP IDENTIFIER { $$ = from_struct_member(ref_pointer($1), yylval.str); }
	| postfix_expression INC_OP { $$ = new_ast_unary_expr(AST_POST_INC, Unary($1)); }
	| postfix_expression DEC_OP { $$ = new_ast_unary_expr(AST_POST_DEC, Unary($1)); }
	| '(' type_name ')' '{' initializer_list '}' { $$ =  }
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
	: VOID { $$ = Type(TY_VOID); }
	| CHAR { $$ = Type(TY_CHAR); }
	| SHORT { $$ = Type(TY_SHORT); }
	| INT { $$ = Type(TY_INT); }
	| LONG { $$ = Type(TY_LONG); }
	| FLOAT { $$ = Type(TY_FLOAT); }
	| DOUBLE { $$ = Type(TY_DOUBLE); }
	| SIGNED { $$ = Type(TY_INT); }
	| UNSIGNED { $$ = Type(TY_UINT); }
	| COMPLEX  { $$ = Type(TY_COMPLEX); }
	| IMAGINARY { $$ = Type(TY_IMAGINARY); }
	| BOOL { $$ = Type(TY_BOOL); }
	| struct_or_union_specifier { $$ = $1; }
	| enum_specifier { $$ = $1; }
	| TYPE_NAME { $$ = yylval.type; }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER { $$ = set_struct_name($1, yylval.str); } '{' struct_declaration_list '}'
	| struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER { $$ = set_struct_name(yylval.str); }
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
	: specifier_qualifier_list struct_declarator_list ';' {  }
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
	: ENUM '{'  enumerator_list '}'
	| ENUM IDENTIFIER <id_node> '{' enumerator_list '}'
	| ENUM '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER <id_node> '{' enumerator_list ',' '}'
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: IDENTIFIER
	| IDENTIFIER <id_node> '=' constant_expression
	;

type_qualifier
	: CONST { $$ = CONSTR_CONST; }
	| RESTRICT { $$ = CONSTR_RESTRICT; }
	| VOLATILE { $$ = CONSTR_VOLATILE; }
	;

declarator
	: pointer direct_declarator { $$ = find_tail_pointer(&$1); $$->ref = $2; $$ = new_ast_pointer($1); }
	| direct_declarator { $$ = $1; } 
	;

pointer
	: '*' { $$ = EmptyPointer(); } 
	| '*' type_qualifier_list { $$ = Pointer(NULL, $1); }  
	| '*' pointer { $$ = Pointer(new_ast_pointer($1, 0)); }
	| '*' type_qualifier_list pointer { $$ = Pointer(new_ast_pointer($2), $1); }
	;

type_qualifier_list
	: type_qualifier { $$ = $1; }
	| type_qualifier_list type_qualifier { $$ = $1 | $2; }
	;


direct_declarator
	: IDENTIFIER 
	| '(' declarator ')'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '[' ']'
	| direct_declarator '(' parameter_type_list ')'
	| direct_declarator '(' identifier_list ')'
	| direct_declarator '(' ')'
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
	: direct_abstract_declarator
	;

direct_abstract_declarator
    : '(' abstract_declarator ')'
    | '[' ']'
    | '[' assignment_expression ']'
    | direct_abstract_declarator '[' ']'
    | direct_abstract_declarator '[' assignment_expression ']'
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

extern char *current_line_buffer;

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, ast_node** root, const char* msg)
{
	fprintf(stderr,"[transpiler]: %s in line %d, column %d\n", msg, yyllocp->first_line, yyllocp->first_column);
	if (current_line_buffer)
		fprintf(stderr, "  %s\n", current_line_buffer);
	fprintf(stderr, "  ");
	for(int i = 1; i < yyllocp->first_column; i++)
		fprintf(stderr, " ");
	fprintf(stderr, "^\n");
}

ast_node* compile(FILE* in)
{
    ast_node* root;
    struct string_builder sb;
    char* content;
    init_str_builder(&sb);
    init_ctx(&sb, in);
    preprocessor_lex();
    content = end_str_builder(&sb);
    if(feed_and_parse(content, &root)){
        free(content);
        return NULL;
    }
    free(content);
    return root;
}

int feed_and_parse(const char* content, ast_node **out)
{
    return tr_process(content, out);
}
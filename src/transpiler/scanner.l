D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P                       ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS                      ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%option prefix="tr"
%option nounput noyywrap nodefault reentrant yylineno
%option bison-bridge bison-locations

%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include "stringlib.h"
	#include "symrec.h"
	#include "ast.h"
	#include "preprocessor/preprocessor.h"
	#include "preprocessor_link.h"
	#include "c.parser.h"
	#define MAX_PREP_DEPTH 1024
	#define YYSTYPE TRSTYPE
	#define YYLTYPE TRLTYPE	
	#define yyparse trparse
	int prep_stack_ptr = 0;
	YY_BUFFER_STATE prep_stack[MAX_PREP_DEPTH];

	void count(yyscan_t);
	void comment(yyscan_t);
	int check_type(yyscan_t, YYSTYPE*);
	int check_sized_type(yyscan_t, YYSTYPE*);
	int type_size = -1;
	int lineno = 0;
	static int next_column = 1;
	int column = 1;
	char *lineptr = NULL;
	size_t n = 0;
	size_t consumed = 0;
	size_t available = 0;
	size_t min(size_t a, size_t b);
	static yyscan_t scanner;
	#define YY_USER_ACTION                                                        \
		yylloc->first_line = yylloc->last_line;                                     \
		yylloc->first_column = yylloc->last_column;                                 \
		if (yylloc->last_line == yylineno)                                          \
			yylloc->last_column += yyleng;                                            \
		else                                                                        \
		{                                                                         \
			yylloc->last_line = yylineno;                                           \
			yylloc->last_column = yytext + yyleng - strrchr (yytext, '\n');         \
		}
%}

%%
"/*"			{ comment(yyscanner); }
"//"[^\n]*              { /* consume //-comment */ }


"auto"			{ count(yyscanner); return(AUTO); }
"bool"			{ count(yyscanner); return(BOOL); }
"break"			{ count(yyscanner); return(BREAK); }
"case"			{ count(yyscanner); return(CASE); }
"char"			{ count(yyscanner); return(CHAR); }
"complex"		{ count(yyscanner); return(COMPLEX); }
"const"			{ count(yyscanner); return(CONST); }
"continue"		{ count(yyscanner); return(CONTINUE); }
"default"		{ count(yyscanner); return(DEFAULT); }
"do"			{ count(yyscanner); return(DO); }
"double"		{ count(yyscanner); return(DOUBLE); }
"else"			{ count(yyscanner); return(ELSE); }
"enum"			{ count(yyscanner); return(ENUM); }
"extern"		{ count(yyscanner); return(EXTERN); }
"float"			{ count(yyscanner); return(FLOAT); }
"for"			{ count(yyscanner); return(FOR); }
"goto"			{ count(yyscanner); return(GOTO); }
"if"			{ count(yyscanner); return(IF); }
"imaginary"	{ count(yyscanner); return(IMAGINARY); }
"inline"		{ count(yyscanner); return(INLINE); }
"int"			{ count(yyscanner); return(INT); }
"long"			{ count(yyscanner); return(LONG); }
"register"		{ count(yyscanner); return(REGISTER); }
"restrict"		{ count(yyscanner); return(RESTRICT); }
"return"		{ count(yyscanner); return(RETURN); }
"short"			{ count(yyscanner); return(SHORT); }
"signed"		{ count(yyscanner); return(SIGNED); }
"sizeof"		{ count(yyscanner); return(SIZEOF); }
"static"		{ count(yyscanner); return(STATIC); }
"struct"		{ count(yyscanner); return(STRUCT); }
"switch"		{ count(yyscanner); return(SWITCH); }
"typedef"		{ count(yyscanner); return(TYPEDEF); }
"union"			{ count(yyscanner); return(UNION); }
"unsigned"		{ count(yyscanner); return(UNSIGNED); }
"void"			{ count(yyscanner); return(VOID); }
"volatile"		{ count(yyscanner); return(VOLATILE); }
"while"			{ count(yyscanner); return(WHILE); }

{L}({L}|{D})*({D})+"_t" { count(yyscanner); return(check_sized_type(yyscanner, yylval)); }
{L}({L}|{D})*		{ count(yyscanner); return(check_type(yyscanner, yylval)); }
0[xX]{H}+{IS}?		{ count(yyscanner); yylval->i = strtol(yytext, NULL, 16); return(INTCONSTANT); }
0[0-7]*{IS}?		{ count(yyscanner); yylval->i = strtol(yytext, NULL, 8); return(INTCONSTANT); }
[1-9]{D}*{IS}?		{ count(yyscanner); yylval->i = strtol(yytext, NULL, 10); return(INTCONSTANT); }
L?'(\\.|[^\\'\n])+'	{ count(yyscanner); yylval->str = strdup(yytext); return(STRING_LITERAL); }

{D}+{E}{FS}?		{ count(yyscanner); yylval->f = atof(yytext); return(FLOATCONSTANT); }
{D}*"."{D}+{E}?{FS}?	{ count(yyscanner); yylval->f = atof(yytext); return(FLOATCONSTANT); }
{D}+"."{D}*{E}?{FS}?	{ count(yyscanner); yylval->f = atof(yytext); return(FLOATCONSTANT); }
0[xX]{H}+{P}{FS}?	{ count(yyscanner); yylval->f = strtof(yytext, NULL); return(INTCONSTANT); }
0[xX]{H}*"."{H}+{P}?{FS}?     { count(yyscanner); yylval->f = strtof(yytext, NULL); return(FLOATCONSTANT); }
0[xX]{H}+"."{H}*{P}?{FS}?     { count(yyscanner); yylval->f = strtof(yytext, NULL); return(FLOATCONSTANT); }

"\n"	{ next_column = 1; }
L?\"(\\.|[^\\"\n])*\"	{ count(yyscanner); yylval->str = strdup(yytext); return(STRING_LITERAL); }

"..."			{ count(yyscanner); return(ELLIPSIS); }
">>="			{ count(yyscanner); return(RIGHT_ASSIGN); }
"<<="			{ count(yyscanner); return(LEFT_ASSIGN); }
"+="			{ count(yyscanner); return(ADD_ASSIGN); }
"-="			{ count(yyscanner); return(SUB_ASSIGN); }
"*="			{ count(yyscanner); return(MUL_ASSIGN); }
"/="			{ count(yyscanner); return(DIV_ASSIGN); }
"%="			{ count(yyscanner); return(MOD_ASSIGN); }
"&="			{ count(yyscanner); return(AND_ASSIGN); }
"^="			{ count(yyscanner); return(XOR_ASSIGN); }
"|="			{ count(yyscanner); return(OR_ASSIGN); }
">>"			{ count(yyscanner); return(RIGHT_OP); }
"<<"			{ count(yyscanner); return(LEFT_OP); }
"++"			{ count(yyscanner); return(INC_OP); }
"--"			{ count(yyscanner); return(DEC_OP); }
"->"			{ count(yyscanner); return(PTR_OP); }
"&&"			{ count(yyscanner); return(AND_OP); }
"||"			{ count(yyscanner); return(OR_OP); }
"<="			{ count(yyscanner); return(LE_OP); }
">="			{ count(yyscanner); return(GE_OP); }
"=="			{ count(yyscanner); return(EQ_OP); }
"!="			{ count(yyscanner); return(NE_OP); }
";"			{ count(yyscanner); return(';'); }
("{"|"<%")		{ count(yyscanner); return('{'); }
("}"|"%>")		{ count(yyscanner); return('}'); }
","			{ count(yyscanner); return(','); }
":"			{ count(yyscanner); return(':'); }
"="			{ count(yyscanner); return('='); }
"("			{ count(yyscanner); return('('); }
")"			{ count(yyscanner); return(')'); }
("["|"<:")		{ count(yyscanner); return('['); }
("]"|":>")		{ count(yyscanner); return(']'); }
"."			{ count(yyscanner); return('.'); }
"&"			{ count(yyscanner); return('&'); }
"!"			{ count(yyscanner); return('!'); }
"~"			{ count(yyscanner); return('~'); }
"-"			{ count(yyscanner); return('-'); }
"+"			{ count(yyscanner); return('+'); }
"*"			{ count(yyscanner); return('*'); }
"/"			{ count(yyscanner); return('/'); }
"%"			{ count(yyscanner); return('%'); }
"<"			{ count(yyscanner); return('<'); }
">"			{ count(yyscanner); return('>'); }
"^"			{ count(yyscanner); return('^'); }
"|"			{ count(yyscanner); return('|'); }
"?"			{ count(yyscanner); return('?'); }
"true"		{ count(yyscanner); return(BOOL_TRUE); }
"false"		{ count(yyscanner); return(BOOL_FALSE); }

[ \t\v\n\f]		{ count(yyscanner); }
.			{ count(yyscanner); char* msg = (char*) malloc(yyleng + 1024); fprintf(stderr,  "Unknown symbol \"%s\"", yytext); free(msg);  exit(1); }

<<EOF>> {

	if(--prep_stack_ptr < 0)
    {
        yyterminate();
    }
    else
    {
        yy_delete_buffer(YY_CURRENT_BUFFER, &yyscanner);
        yy_switch_to_buffer(prep_stack[prep_stack_ptr], yyscanner);
    }
}

%%


void comment(yyscan_t scanner)
{
	char c, prev = 0;

	while ((c = input(scanner)) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	perror("unterminated comment");
}

void count(yyscan_t scanner)
{
	lineno = yyget_lineno(scanner);
	column = next_column;
	next_column += yyget_leng(scanner);
}

int check_type(yyscan_t scanner, YYSTYPE* out)
{
	type_size = -1;
	out->str = strdup(yyget_text(scanner));

	if(gettype(yyget_text(scanner))){
		return TYPE_NAME;
	}

	return IDENTIFIER;
}

int check_sized_type(yyscan_t scanner, YYSTYPE* out){
	char namebuf[64];
	int size;
	typerec_t* prim_type;

	int ret = sscanf(yyget_text(scanner), "%[^0-9]%d_t", namebuf, &size);

	if(ret < 2){
		return check_type(scanner, out);
	}

	if(! (prim_type = gettype(namebuf))){
		return check_type(scanner, out);
	}

	if(! getsizedtype(namebuf, size)){
		PUT_SIZED_TYPE(namebuf, prim_type->type_type, size);
	}

	out->str = strdup(namebuf);
	type_size = size;
	return TYPE_NAME;
}

size_t min(size_t a, size_t b) {
    return b < a ? b : a;
}

int tr_process(const char* content, ast_node** out){
    if(yylex_init(&scanner)){
		fprintf(stderr, "Failed to init lex: %d", errno);
		exit(1);
	}
    YY_BUFFER_STATE buffer = yy_scan_string(content, scanner);
	yy_switch_to_buffer(buffer, scanner);
    if(! yyparse(scanner, out)){
        yy_delete_buffer(buffer, scanner);
        return 0;
    }
    yy_delete_buffer(buffer, scanner);
	return 1;
}
D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P                       ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS                      ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringlib.h"
#include "ast.h"
#include "c.parser.h"
#include "symrec.h"

void count(void);
void comment(void);
int check_type(void);
int check_sized_type(void);
void yyerror(ast_node** root, char const *s);
int type_size = -1;

static int next_column = 1;
int column = 1;
char *lineptr = NULL;
size_t n = 0;
size_t consumed = 0;
size_t available = 0;
size_t min(size_t a, size_t b);

#define YY_INPUT(buf,result,max_size) {\
	if(available <= 0) {\
		consumed = 0;\
		available = prep_readline(&lineptr, &n, yyin);\
		if (available < 0) {\
			if (ferror(yyin)) { perror("read error:"); }\
				available = 0;\
		}\
	}\
	result = min(available, max_size);\
	strncpy(buf, lineptr + consumed, result);\
	printf("%s %d\n",buf, result); \
	consumed += result;\
	available -= result;\
}
%}
%option nounput
%option yylineno
%option noyywrap
%%
"/*"			{ comment(); }
"//"[^\n]*              { /* consume //-comment */ }


"auto"			{ count(); return(AUTO); }
"bool"			{ count(); return(BOOL); }
"break"			{ count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); return(CHAR); }
"complex"		{ count(); return(COMPLEX); }
"const"			{ count(); return(CONST); }
"continue"		{ count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"enum"			{ count(); return(ENUM); }
"extern"		{ count(); return(EXTERN); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"imaginary"	{ count(); return(IMAGINARY); }
"inline"		{ count(); return(INLINE); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"restrict"		{ count(); return(RESTRICT); }
"return"		{ count(); return(RETURN); }
"short"			{ count(); return(SHORT); }
"signed"		{ count(); return(SIGNED); }
"sizeof"		{ count(); return(SIZEOF); }
"static"		{ count(); return(STATIC); }
"struct"		{ count(); return(STRUCT); }
"switch"		{ count(); return(SWITCH); }
"typedef"		{ count(); return(TYPEDEF); }
"union"			{ count(); return(UNION); }
"unsigned"		{ count(); return(UNSIGNED); }
"void"			{ count(); return(VOID); }
"volatile"		{ count(); return(VOLATILE); }
"while"			{ count(); return(WHILE); }

{L}({L}|{D})*({D})+"_t" { count(); return(check_sized_type()); }
{L}({L}|{D})*		{ count(); return(check_type()); }
0[xX]{H}+{IS}?		{ count(); yylval.i = strtol(yytext, NULL, 16); return(INTCONSTANT); }
0[0-7]*{IS}?		{ count(); yylval.i = strtol(yytext, NULL, 8); return(INTCONSTANT); }
[1-9]{D}*{IS}?		{ count(); yylval.i = strtol(yytext, NULL, 10); return(INTCONSTANT); }
L?'(\\.|[^\\'\n])+'	{ count(); yylval.str = strdup(yytext); return(STRING_LITERAL); }

{D}+{E}{FS}?		{ count(); yylval.f = atof(yytext); return(FLOATCONSTANT); }
{D}*"."{D}+{E}?{FS}?	{ count(); yylval.f = atof(yytext); return(FLOATCONSTANT); }
{D}+"."{D}*{E}?{FS}?	{ count(); yylval.f = atof(yytext); return(FLOATCONSTANT); }
0[xX]{H}+{P}{FS}?	{ count(); yylval.f = strtof(yytext, NULL); return(INTCONSTANT); }
0[xX]{H}*"."{H}+{P}?{FS}?     { count(); yylval.f = strtof(yytext, NULL); return(FLOATCONSTANT); }
0[xX]{H}+"."{H}*{P}?{FS}?     { count(); yylval.f = strtof(yytext, NULL); return(FLOATCONSTANT); }

"\n"	{ next_column = 1; }
L?\"(\\.|[^\\"\n])*\"	{ count(); yylval.str = strdup(yytext); return(STRING_LITERAL); }

"..."			{ count(); return(ELLIPSIS); }
">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }
"true"		{ count(); return(BOOL_TRUE); }
"false"		{ count(); return(BOOL_FALSE); }

[ \t\v\n\f]		{ count(); }
.			{ count(); char* msg = (char*) malloc(yyleng + 1024); sprintf(msg, "Unknown symbol \"%s\"", yytext); yyerror(NULL, msg); free(msg); }

%%


void comment(void)
{
	char c, prev = 0;
  
	while ((c = input()) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	perror("unterminated comment");
}

void count(void)
{
	column = next_column; 
	next_column += strlen(yytext);
}

int check_type(void)
{
	type_size = -1;
	yylval.str = strdup(yytext);
	
	if(gettype(yytext)){
		return TYPE_NAME;
	}

	return IDENTIFIER;
}

int check_sized_type(void){
	char namebuf[64];
	int size;
	typerec_t* prim_type;

	int ret = sscanf(yytext, "%[^0-9]%d_t", namebuf, &size);

	if(ret < 2){
		return check_type();
	}

	if(! (prim_type = gettype(namebuf))){
		return check_type();
	}

	if(! getsizedtype(namebuf, size)){
		PUT_SIZED_TYPE(namebuf, prim_type->type_type, size);
	}

	yylval.str = strdup(namebuf);
	type_size = size;
	return TYPE_NAME;
}

size_t min(size_t a, size_t b) {
    return b < a ? b : a;
}
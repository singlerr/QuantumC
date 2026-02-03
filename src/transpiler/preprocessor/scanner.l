D			[0-9]
L			[a-zA-Z_]
ID          {L}({L}|{D})*
NL          "\n"
S           [ \t]
STR         \"(\\.|[^\\"\n])*\"

H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P                       ([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS                      ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%{

#include <stdio.h>
#include <stdlib.h>
#include "../ast.h"
#include "../preprocessor_link.h"
#include "preprocessor.parser.h"
#include "preprocessor.h"

extern struct string_builder *ctx;

#define MAX_INCLUDE_DEPTH 10
#define SKIP_INITIAL if(YYSTATE == INITIAL) { forward(try_expand(yytext)); } else 

// preprocessor sections
int include_stack_ptr = 0;
int expand_stack_ptr = 0;

YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
YY_BUFFER_STATE expand_stack[MAX_INCLUDE_DEPTH];

int expand_macro(const char* name, char** out);
void expand_expression(const char* name);
char* try_expand(const char* text);
int forward(const char* str);


%}
%option prefix="pr"
%option nounput
%option yylineno
%option noyywrap
%option stack


%x STATE_DEFINE
%x STATE_BODY
%x STATE_BODY_EXT
%x STATE_INCLUDE
%x STATE_IFDEF
%x STATE_IF
%x STATE_IFNDEF
%x STATE_UNDEF
%x STATE_OPENQASM
%x STATE_EXPAND
%%

^"#""openqasm"                { yy_push_state(STATE_OPENQASM); return(OPENQASM); }
^"#""include"                 { yy_push_state(STATE_INCLUDE); }
^"#""define"                  { yy_push_state(STATE_DEFINE); return(DEFINE); }
^"#""ifdef"                   { yy_push_state(STATE_IFDEF); return(IFDEF); }
^"#""else"                    { return(ELSE); }
^"#""if"                      { yy_push_state(STATE_IF); return(IF); }
^"#""elif"                    { return(ELIF); }
^"#""ifndef"                  { yy_push_state(STATE_IFNDEF); return(IFNDEF); }
^"#""endif"                   { return(ENDIF); }
^"#""undef"                   { yy_push_state(STATE_UNDEF); return(UNDEF); }

"auto"			{ forward(yytext); }
"bool"			{ forward(yytext); }
"break"			{ forward(yytext); }
"case"			{ forward(yytext); }
"char"			{ forward(yytext); }
"complex"		{ forward(yytext); }
"const"			{ forward(yytext); }
"continue"		{ forward(yytext); }
"default"		{ forward(yytext); }
"do"			{ forward(yytext); }
"double"		{ forward(yytext); }
"else"			{ forward(yytext); }
"enum"			{ forward(yytext); }
"extern"		{ forward(yytext); }
"float"			{ forward(yytext); }
"for"			{ forward(yytext); }
"goto"			{ forward(yytext); }
"if"			{ forward(yytext); }
"imaginary"	    { forward(yytext); }
"inline"		{ forward(yytext); }
"int"			{ forward(yytext); }
"long"			{ forward(yytext); }
"register"		{ forward(yytext); }
"restrict"		{ forward(yytext); }
"return"		{ forward(yytext); }
"short"			{ forward(yytext); }
"signed"		{ forward(yytext); }
"sizeof"		{ forward(yytext); }
"static"		{ forward(yytext); }
"struct"		{ forward(yytext); }
"switch"		{ forward(yytext); }
"typedef"		{ forward(yytext); }
"union"			{ forward(yytext); }
"unsigned"		{ forward(yytext); }
"void"			{ forward(yytext); }
"volatile"		{ forward(yytext); }
"while"			{ forward(yytext); }


[[:space:]]+             { forward(yytext); }
{L}({L}|{D})*		{ forward(try_expand(yytext)); }
0[xX]{H}+{IS}?		{ forward(yytext); }
0[0-7]*{IS}?		{ forward(yytext); }
[1-9]{D}*{IS}?		{ forward(yytext); }
L?'(\\.|[^\\'\n])+'	{ forward(yytext); }

{D}+{E}{FS}?		{ forward(yytext); }
{D}*"."{D}+{E}?{FS}?	{ forward(yytext); }
{D}+"."{D}*{E}?{FS}?	{ forward(yytext); }
0[xX]{H}+{P}{FS}?	{ }
0[xX]{H}*"."{H}+{P}?{FS}?     { forward(yytext); }
0[xX]{H}+"."{H}*{P}?{FS}?     { forward(yytext); }

"\n"	{ forward(yytext); }
L?\"(\\.|[^\\"\n])*\"	{ forward(yytext); }

"..."			{ forward(yytext); }
">>="			{ forward(yytext); }
"<<="			{ forward(yytext); }
"+="			{ forward(yytext); }
"-="			{ forward(yytext); }
"*="			{ forward(yytext); }
"/="			{ forward(yytext); }
"%="			{ forward(yytext); }
"&="			{ forward(yytext); }
"^="			{ forward(yytext); }
"|="			{ forward(yytext); }
">>"			{ forward(yytext); }
"<<"			{ forward(yytext); }
"++"			{ forward(yytext); }
"--"			{ forward(yytext); }
"->"			{ forward(yytext); }
"&&"			{ forward(yytext); }
"||"			{ forward(yytext); }
"<="			{ forward(yytext); }
">="			{ forward(yytext); }
"=="			{ forward(yytext); }
"!="			{ forward(yytext); }
";"			{ forward(yytext); }
("{"|"<%")		{ forward(yytext); }
("}"|"%>")		{ forward(yytext); }
","			{ forward(yytext); }
":"			{ forward(yytext); }
"="			{ forward(yytext); }
"("			{ forward(yytext); }
")"			{ forward(yytext); }
("["|"<:")		{ forward(yytext); }
("]"|":>")		{ forward(yytext); }
"."			{ forward(yytext); }
"&"			{ forward(yytext); }
"!"			{ forward(yytext); }
"~"			{ forward(yytext); }
"-"			{ forward(yytext); }
"+"			{ forward(yytext); }
"*"			{ forward(yytext); }
"/"			{ forward(yytext); }
"%"			{ forward(yytext); }
"<"			{ forward(yytext); }
">"			{ forward(yytext); }
"^"			{ forward(yytext); }
"|"			{ forward(yytext); }
"?"			{ forward(yytext); }
"true"		{ forward(yytext); }
"false"		{ forward(yytext); }

<STATE_OPENQASM>{D}+                    { return(NUM); }
<STATE_OPENQASM>{NL}                    { yy_pop_state(); }
<STATE_DEFINE>{ID}                      { prlval.str = strdup(yytext); yy_push_state(STATE_BODY); return(IDENTIFIER); }
<STATE_DEFINE>"("                       { return(LPAREN); }
<STATE_DEFINE>")"                       { return(RPAREN); }
<STATE_DEFINE>","                       { return(COMMA); }
<STATE_DEFINE>{S}+                      { /* skip whitespace in define */ }
<STATE_BODY>"("                         { return(LPAREN); }
<STATE_BODY>")"                         { return(RPAREN); }
<STATE_BODY>","                         { return(COMMA); }
<STATE_BODY>{S}+                        { prlval.str = strdup(yytext); return(TEXT); }
<STATE_BODY>"\\"                        { yy_pop_state(); yy_push_state(STATE_BODY_EXT); }
<STATE_BODY>{NL}                        { yy_pop_state(); yy_pop_state(); return(NEWLINE); }
<STATE_BODY_EXT>{S}+                    { /* skip leading whitespace after backslash */ }
<STATE_BODY_EXT>{NL}                    { yy_pop_state(); yy_push_state(STATE_BODY); }
<STATE_BODY>{ID}                        { prlval.str = strdup(yytext); return(PLACEHOLDER); }
<STATE_BODY>"##"{ID}                    { prlval.str = strdup(yytext + 2); return(PLACEHOLDER); }
<STATE_BODY>"#"{ID}                     { prlval.str = strdup(yytext + 1); return(STRINGIFIED); }
<STATE_BODY>[^ \t\v\n\f\r(),"]+         { prlval.str = strdup(yytext); return(TEXT); }
<STATE_BODY_EXT>{ID}                    { prlval.str = strdup(yytext); return(PLACEHOLDER); }
<STATE_BODY_EXT>"##"{ID}                { prlval.str = strdup(yytext + 2); return(PLACEHOLDER); }
<STATE_BODY_EXT>"#"{ID}                 { prlval.str = strdup(yytext + 1); return(STRINGIFIED); }
<STATE_BODY_EXT>[^ \t\v\n\f\r(),"]+     { prlval.str = strdup(yytext); return(TEXT); }
<STATE_INCLUDE>{STR}                    {
    if (include_stack_ptr >= MAX_INCLUDE_DEPTH)
    {
        fprintf( stderr, "Includes nested too deeply" );
        exit( 1 );
    }

    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
    yyin = fopen(yytext, "r");

    if(! yyin)
    {
        fprintf(stderr, "File as include target could not be found: %s.\n", yytext);
        exit(1);
    }

    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
    }

<STATE_INCLUDE><<EOF>> {
    if(--include_stack_ptr < 0)
    {
        yyterminate();
    }
    else
    {
        yy_delete_buffer(YY_CURRENT_BUFFER);
        yy_switch_to_buffer(include_stack[include_stack_ptr]);
    }

    yy_pop_state();
}
<STATE_EXPAND><<EOF>> {
    if(--expand_stack_ptr < 0)
    {
        yyterminate();
    }
    else
    {
        yy_delete_buffer(YY_CURRENT_BUFFER);
        yy_switch_to_buffer(expand_stack[expand_stack_ptr]);
    }

    yy_pop_state();
}

<STATE_INCLUDE>\<(\\.|[^\\>\n])*\>    { yy_pop_state(); }
<STATE_INCLUDE>{NL}                     { yy_pop_state(); }
<STATE_IF>"defined"                   { return(DEFINED); }
<STATE_IF>"<"                         { return(L_OP); }
<STATE_IF>">"                         { return(G_OP); }
<STATE_IF>"<="                        { return(LE_OP); }
<STATE_IF>">="                        { return(GE_OP); }
<STATE_IF>{NL}                          { yy_pop_state(); }
<STATE_IF>"("                         { return(LPAREN); }
<STATE_IF>")"                         { return(RPAREN); }
<STATE_IF>"&&"                        { return(AND_OP); }
<STATE_IF>"||"                        { return(OR_OP);}
<STATE_IF>{ID}                          { expand_expression(yytext); }
<STATE_IFDEF>{ID}                       { return(IDENTIFIER); }
<STATE_IFDEF>{NL}                       { yy_pop_state(); }
<STATE_IFNDEF>{ID}                      { return(IDENTIFIER); }
<STATE_IFNDEF>{NL}                      { yy_pop_state(); }
<STATE_UNDEF>{ID}                       { return(IDENTIFIER); }
<STATE_UNDEF>{NL}                       { yy_pop_state(); }
%%


char* try_expand(const char* text){
    char* content;

    if(! expand_macro(text, &content)){
        return (char*) text;
    }

    return content;
}

int expand_macro(const char* name, char** out)
{
    struct dir_define* macro = find_macro(name);
    char* content;
    if(! macro)
    {
        return 0;
    }

    if(! expand_placeholder(&content, macro->content))
    {
        fprintf(stderr, "Could not expand macro: %s.\n", name);
        return 0;
    }

    *out = content;
    return 1;
}

void expand_expression(const char* name){
    char* content;
    if(! expand_macro(name, &content)){
        return;
    }

    if (expand_stack_ptr >= MAX_INCLUDE_DEPTH)
    {
        fprintf(stderr, "Expand too deeply");
        exit(1);
    }

    yy_push_state(STATE_EXPAND);
    YY_BUFFER_STATE buffer = pr_scan_string(content);
    expand_stack[expand_stack_ptr++] = YY_CURRENT_BUFFER;
    pr_switch_to_buffer(buffer);
}

int forward(const char* str){
    if (get_skip_mode()) {
        return 0;
    }
    str_append(ctx, str);
    return 0;
}

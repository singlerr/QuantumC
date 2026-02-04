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
extern struct string_builder *if_ctx;

#define yylval prlval
#define MAX_INCLUDE_DEPTH 10

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

"#openqasm"                { if(! should_skip()) { yy_push_state(STATE_OPENQASM); return(OPENQASM); } }
"#include"                 { if(! should_skip()) { yy_push_state(STATE_INCLUDE); } }
"#define"                  { if(! should_skip()) { yy_push_state(STATE_DEFINE); return(DEFINE); } }
"#ifdef"                   { if(! should_skip()) { yy_push_state(STATE_IFDEF); return(IFDEF); } }
"#else"                    { if(! should_skip()) { return(ELSE); } }
"#if"                      { if(! should_skip()) { yy_push_state(STATE_IF); return(IF); } }
"#elif"                    { if(! should_skip()) { return(ELIF); } }
"#ifndef"                  { if(! should_skip()) { yy_push_state(STATE_IFNDEF); return(IFNDEF); } }
"#endif"                   { if(! should_skip()) { return(ENDIF); } }
"#undef"                   { if(! should_skip()) { yy_push_state(STATE_UNDEF); return(UNDEF); } }

"auto"                     { if(! should_skip()) { forward(yytext); } }
"bool"                     { if(! should_skip()) { forward(yytext); } }
"break"                     { if(! should_skip()) { forward(yytext); } }
"case"                     { if(! should_skip()) { forward(yytext); } }
"char"                     { if(! should_skip()) { forward(yytext); } }
"complex"                   { if(! should_skip()) { forward(yytext); } }
"const"                     { if(! should_skip()) { forward(yytext); } }
"continue"                   { if(! should_skip()) { forward(yytext); } }
"default"                   { if(! should_skip()) { forward(yytext); } }
"do"                       { if(! should_skip()) { forward(yytext); } }
"double"                   { if(! should_skip()) { forward(yytext); } }
"else"                     { if(! should_skip()) { forward(yytext); } }
"enum"                     { if(! should_skip()) { forward(yytext); } }
"extern"                   { if(! should_skip()) { forward(yytext); } }
"float"                     { if(! should_skip()) { forward(yytext); } }
"for"                       { if(! should_skip()) { forward(yytext); } }
"goto"                     { if(! should_skip()) { forward(yytext); } }
"if"                       { if(! should_skip()) { forward(yytext); } }
"imaginary"                 { if(! should_skip()) { forward(yytext); } }
"inline"                   { if(! should_skip()) { forward(yytext); } }
"int"                       { if(! should_skip()) { forward(yytext); } }
"long"                       { if(! should_skip()) { forward(yytext); } }
"register"                   { if(! should_skip()) { forward(yytext); } }
"restrict"                   { if(! should_skip()) { forward(yytext); } }
"return"                     { if(! should_skip()) { forward(yytext); } }
"short"                     { if(! should_skip()) { forward(yytext); } }
"signed"                     { if(! should_skip()) { forward(yytext); } }
"sizeof"                     { if(! should_skip()) { forward(yytext); } }
"static"                     { if(! should_skip()) { forward(yytext); } }
"struct"                     { if(! should_skip()) { forward(yytext); } }
"switch"                     { if(! should_skip()) { forward(yytext); } }
"typedef"                   { if(! should_skip()) { forward(yytext); } }
"union"                     { if(! should_skip()) { forward(yytext); } }
"unsigned"                   { if(! should_skip()) { forward(yytext); } }
"void"                       { if(! should_skip()) { forward(yytext); } }
"volatile"                   { if(! should_skip()) { forward(yytext); } }
"while"                     { if(! should_skip()) { forward(yytext); } }

{S}+                       { if(! should_skip()) { forward(yytext); } }
{L}({L}|{D})*              { if(! should_skip()) { forward(try_expand(yytext)); } }
0[xX]{H}+{IS}?             { if(! should_skip()) { forward(yytext); } }
0[0-7]*{IS}?               { if(! should_skip()) { forward(yytext); } }
[1-9]{D}*{IS}?             { if(! should_skip()) { forward(yytext); } }
L?'(\\.|[^\\'\n])+'        { if(! should_skip()) { forward(yytext); } }

{D}+{E}{FS}?               { if(! should_skip()) { forward(yytext); } }
{D}*"."{D}+{E}?{FS}?       { if(! should_skip()) { forward(yytext); } }
{D}+"."{D}*{E}?{FS}?       { if(! should_skip()) { forward(yytext); } }
0[xX]{H}+{P}{FS}?          { if(! should_skip()) { } }
0[xX]{H}*"."{H}+{P}?{FS}?  { if(! should_skip()) { forward(yytext); } }
0[xX]{H}+"."{H}*{P}?{FS}?  { if(! should_skip()) { forward(yytext); } }

"\n"                       { if(! should_skip()) { forward(yytext); } }
L?\"(\\.|[^\\"\n])*\"      { if(! should_skip()) { forward(yytext); } }

"..."                      { if(! should_skip()) { forward(yytext); } }
">>="			{ if(! should_skip()) { forward(yytext); } }
"<<="			{ if(! should_skip()) { forward(yytext); } }
"+="			{ if(! should_skip()) { forward(yytext); } }
"-="			{ if(! should_skip()) { forward(yytext); } }
"*="			{ if(! should_skip()) { forward(yytext); } }
"/="			{ if(! should_skip()) { forward(yytext); } }
"%="			{ if(! should_skip()) { forward(yytext); } }
"&="			{ if(! should_skip()) { forward(yytext); } }
"^="			{ if(! should_skip()) { forward(yytext); } }
"|="			{ if(! should_skip()) { forward(yytext); } }
">>"			{ if(! should_skip()) { forward(yytext); } }
"<<"			{ if(! should_skip()) { forward(yytext); } }
"++"			{ if(! should_skip()) { forward(yytext); } }
"--"			{ if(! should_skip()) { forward(yytext); } }
"->"			{ if(! should_skip()) { forward(yytext); } }
"&&"			{ if(! should_skip()) { forward(yytext); } }
"||"			{ if(! should_skip()) { forward(yytext); } }
"<="			{ if(! should_skip()) { forward(yytext); } }
">="			{ if(! should_skip()) { forward(yytext); } }
"=="			{ if(! should_skip()) { forward(yytext); } }
"!="			{ if(! should_skip()) { forward(yytext); } }
";"			{ if(! should_skip()) { forward(yytext); } }
("{"|"<%")		{ if(! should_skip()) { forward(yytext); } }
("}"|"%>")		{ if(! should_skip()) { forward(yytext); } }
","			{ if(! should_skip()) { forward(yytext); } }
":"			{ if(! should_skip()) { forward(yytext); } }
"="			{ if(! should_skip()) { forward(yytext); } }
"("			{ if(! should_skip()) { forward(yytext); } }
")"			{ if(! should_skip()) { forward(yytext); } }
("["|"<:")		{ if(! should_skip()) { forward(yytext); } }
("]"|":>")		{ if(! should_skip()) { forward(yytext); } }
"."			{ if(! should_skip()) { forward(yytext); } }
"&"			{ if(! should_skip()) { forward(yytext); } }
"!"			{ if(! should_skip()) { forward(yytext); } }
"~"			{ if(! should_skip()) { forward(yytext); } }
"-"			{ if(! should_skip()) { forward(yytext); } }
"+"			{ if(! should_skip()) { forward(yytext); } }
"*"			{ if(! should_skip()) { forward(yytext); } }
"/"			{ if(! should_skip()) { forward(yytext); } }
"%"			{ if(! should_skip()) { forward(yytext); } }
"<"			{ if(! should_skip()) { forward(yytext); } }
">"			{ if(! should_skip()) { forward(yytext); } }
"^"			{ if(! should_skip()) { forward(yytext); } }
"|"			{ if(! should_skip()) { forward(yytext); } }
"?"			{ if(! should_skip()) { forward(yytext); } }
"true"                     { if(! should_skip()) { forward(yytext); } }
"false"                    { if(! should_skip()) { forward(yytext); } }

<STATE_OPENQASM>{D}+        { if(! should_skip()) { return(NUM); } }
<STATE_OPENQASM>{NL}        { if(! should_skip()) { yy_pop_state(); } }
<STATE_DEFINE>{ID}          { if(! should_skip()) { yylval.str = strdup(yytext); yy_push_state(STATE_BODY); return(IDENTIFIER); } }
<STATE_DEFINE>"("           { if(! should_skip()) { return(LPAREN); } }
<STATE_DEFINE>")"           { if(! should_skip()) { return(RPAREN); } }
<STATE_DEFINE>","           { if(! should_skip()) { return(COMMA); } }
<STATE_DEFINE>{S}+          { if(! should_skip()) { /* skip whitespace in define */ } }
<STATE_BODY>"("             { if(! should_skip()) { return(LPAREN); } }
<STATE_BODY>")"             { if(! should_skip()) { return(RPAREN); } }
<STATE_BODY>","             { if(! should_skip()) { return(COMMA); } }
<STATE_BODY>{S}+            { if(! should_skip()) { yylval.str = strdup(yytext); return(TEXT); } }
<STATE_BODY>"\\"            { if(! should_skip()) { yy_pop_state(); yy_push_state(STATE_BODY_EXT); } }
<STATE_BODY>{NL}            { if(! should_skip()) { yy_pop_state(); yy_pop_state(); return(NEWLINE); } }
<STATE_BODY_EXT>{S}+        { if(! should_skip()) { /* skip leading whitespace after backslash */ } }
<STATE_BODY_EXT>{NL}        { if(! should_skip()) { yy_pop_state(); yy_push_state(STATE_BODY); } }
<STATE_BODY>{ID}            { if(! should_skip()) { yylval.str = strdup(yytext); return(PLACEHOLDER); } }
<STATE_BODY>"##"{ID}        { if(! should_skip()) { yylval.str = strdup(yytext + 2); return(PLACEHOLDER); } }
<STATE_BODY>"#"{ID}         { if(! should_skip()) { yylval.str = strdup(yytext + 1); return(STRINGIFIED); } }
<STATE_BODY>[^ \t\v\n\f\r(),"]+     { if(! should_skip()) { yylval.str = strdup(yytext); return(TEXT); } }
<STATE_BODY_EXT>{ID}        { if(! should_skip()) { yylval.str = strdup(yytext); return(PLACEHOLDER); } }
<STATE_BODY_EXT>"##"{ID}    { if(! should_skip()) { yylval.str = strdup(yytext + 2); return(PLACEHOLDER); } }
<STATE_BODY_EXT>"#"{ID}     { if(! should_skip()) { yylval.str = strdup(yytext + 1); return(STRINGIFIED); } }
<STATE_BODY_EXT>[^ \t\v\n\f\r(),"]+ { if(! should_skip()) { yylval.str = strdup(yytext); return(TEXT); } }

<STATE_INCLUDE>{STR}        {
    if(! should_skip()) {
        // ...existing code...
    }
}
<STATE_INCLUDE><<EOF>>      {
    if(! should_skip()) {
        // ...existing code...
    }
}
<STATE_EXPAND><<EOF>>       {
    if(! should_skip()) {
        // ...existing code...
    }
}

<STATE_INCLUDE>\<(\\.|[^\\>\n])*\> { if(! should_skip()) { yy_pop_state(); } }
<STATE_INCLUDE>{NL}          { if(! should_skip()) { yy_pop_state(); } }
<STATE_IF>"defined"          { if(! should_skip()) { return(DEFINED); } }
<STATE_IF>"<"                { if(! should_skip()) { return(L_OP); } }
<STATE_IF>">"                { if(! should_skip()) { return(G_OP); } }
<STATE_IF>"<="               { if(! should_skip()) { return(LE_OP); } }
<STATE_IF>">="               { if(! should_skip()) { return(GE_OP); } }
<STATE_IF>{NL}               { if(! should_skip()) { yy_pop_state(); } }
<STATE_IF>"("                { if(! should_skip()) { return(LPAREN); } }
<STATE_IF>")"                { if(! should_skip()) { return(RPAREN); } }
<STATE_IF>"&&"               { if(! should_skip()) { return(AND_OP); } }
<STATE_IF>"||"               { if(! should_skip()) { return(OR_OP); } }
<STATE_IF>{ID}               { if(! should_skip()) { expand_expression(yytext); } }
<STATE_IFDEF>{ID}            { if(! should_skip()) { yylval.str = strdup(yytext); return(IDENTIFIER); } }
<STATE_IFDEF>{NL}            { if(! should_skip()) { yy_pop_state(); } }
<STATE_IFNDEF>{S}+           { if(! should_skip()) { } }
<STATE_IFNDEF>{ID}           { if(! should_skip()) { yylval.str = strdup(yytext); return(IDENTIFIER); } }
<STATE_IFNDEF>{NL}           { if(! should_skip()) { yy_pop_state(); } }
<STATE_UNDEF>{ID}            { if(! should_skip()) { yylval.str = strdup(yytext); return(IDENTIFIER); } }
<STATE_UNDEF>{NL}            { if(! should_skip()) { yy_pop_state(); } }
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
    if(should_skip()){
        return 0;
    }

    str_append(ctx, str);
    return 0;
}


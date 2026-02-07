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
int handle_text(const char* str);

%}
%option prefix="pr"
%option nounput
%option yylineno
%option noyywrap
%option stack
%option nodefault 

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

"#openqasm"                { yy_push_state(STATE_OPENQASM); return(OPENQASM); }
"#include"                 { yy_push_state(STATE_INCLUDE); }
"#define"                  { yy_push_state(STATE_DEFINE); return(DEFINE); }
"#ifdef"                   { yy_push_state(STATE_IFDEF); return(IFDEF); }
"#else"                    { return(ELSE); }
"#if"                      { yy_push_state(STATE_IF); return(IF); }
"#elif"                    { yy_push_state(STATE_IF); return(ELIF); }
"#ifndef"                  { yy_push_state(STATE_IFNDEF); return(IFNDEF); }
"#endif"                   { return(ENDIF); }
"#undef"                   { yy_push_state(STATE_UNDEF); return(UNDEF); }

"auto"                     { return handle_text(yytext); }
"bool"                     { return handle_text(yytext); }
"break"                     { return handle_text(yytext); }
"case"                     { return handle_text(yytext); }
"char"                     { return handle_text(yytext); }
"complex"                   { return handle_text(yytext); }
"const"                     { return handle_text(yytext); }
"continue"                   { return handle_text(yytext); }
"default"                   { return handle_text(yytext); }
"do"                       { return handle_text(yytext); }
"double"                   { return handle_text(yytext); }
"else"                     { return handle_text(yytext); }
"enum"                     { return handle_text(yytext); }
"extern"                   { return handle_text(yytext); }
"float"                     { return handle_text(yytext); }
"for"                       { return handle_text(yytext); }
"goto"                     { return handle_text(yytext); }
"if"                       { return handle_text(yytext); }
"imaginary"                 { return handle_text(yytext); }
"inline"                   { return handle_text(yytext); }
"int"                       { return handle_text(yytext); }
"long"                       { return handle_text(yytext); }
"register"                   { return handle_text(yytext); }
"restrict"                   { return handle_text(yytext); }
"return"                     { return handle_text(yytext); }
"short"                     { return handle_text(yytext); }
"signed"                     { return handle_text(yytext); }
"sizeof"                     { return handle_text(yytext); }
"static"                     { return handle_text(yytext); }
"struct"                     { return handle_text(yytext); }
"switch"                     { return handle_text(yytext); }
"typedef"                   { return handle_text(yytext); }
"union"                     { return handle_text(yytext); }
"unsigned"                   { return handle_text(yytext); }
"void"                       { return handle_text(yytext); }
"volatile"                   { return handle_text(yytext); }
"while"                     { return handle_text(yytext); }

{S}+                       { return handle_text(yytext); }
{L}({L}|{D})*              { return handle_text(yytext); }
0[xX]{H}+{IS}?             { return handle_text(yytext); }
0[0-7]*{IS}?               { return handle_text(yytext); }
[1-9]{D}*{IS}?             { return handle_text(yytext); }
L?'(\\.|[^\\'\n])+'        { return handle_text(yytext); }

{D}+{E}{FS}?               { return handle_text(yytext); }
{D}*"."{D}+{E}?{FS}?       { return handle_text(yytext); }
{D}+"."{D}*{E}?{FS}?       { return handle_text(yytext); }
0[xX]{H}+{P}{FS}?          { return handle_text(yytext); }
0[xX]{H}*"."{H}+{P}?{FS}?  { return handle_text(yytext); }
0[xX]{H}+"."{H}*{P}?{FS}?  { return handle_text(yytext); }

"\n"                       { return handle_text(yytext); }
L?\"(\\.|[^\\"\n])*\"      { return handle_text(yytext); }

"..."                      { return handle_text(yytext); }
">>="			{ return handle_text(yytext); }
"<<="			{ return handle_text(yytext); }
"+="			{ return handle_text(yytext); }
"-="			{ return handle_text(yytext); }
"*="			{ return handle_text(yytext); }
"/="			{ return handle_text(yytext); }
"%="			{ return handle_text(yytext); }
"&="			{ return handle_text(yytext); }
"^="			{ return handle_text(yytext); }
"|="			{ return handle_text(yytext); }
">>"			{ return handle_text(yytext); }
"<<"			{ return handle_text(yytext); }
"++"			{ return handle_text(yytext); }
"--"			{ return handle_text(yytext); }
"->"			{ return handle_text(yytext); }
"&&"			{ return handle_text(yytext); }
"||"			{ return handle_text(yytext); }
"<="			{ return handle_text(yytext); }
">="			{ return handle_text(yytext); }
"=="			{ return handle_text(yytext); }
"!="			{ return handle_text(yytext); }
";"			{ return handle_text(yytext); }
("{"|"<%")		{ return handle_text(yytext); }
("}"|"%>")		{ return handle_text(yytext); }
","			{ return handle_text(yytext); }
":"			{ return handle_text(yytext); }
"="			{ return handle_text(yytext); }
"("			{ return handle_text(yytext); }
")"			{ return handle_text(yytext); }
("["|"<:")		{ return handle_text(yytext); }
("]"|":>")		{ return handle_text(yytext); }
"."			{ return handle_text(yytext); }
"&"			{ return handle_text(yytext); }
"!"			{ return handle_text(yytext); }
"~"			{ return handle_text(yytext); }
"-"			{ return handle_text(yytext); }
"+"			{ return handle_text(yytext); }
"*"			{ return handle_text(yytext); }
"/"			{ return handle_text(yytext); }
"%"			{ return handle_text(yytext); }
"<"			{ return handle_text(yytext); }
">"			{ return handle_text(yytext); }
"^"			{ return handle_text(yytext); }
"|"			{ return handle_text(yytext); }
"?"			{ return handle_text(yytext); }
"true"                     { return handle_text(yytext); }
"false"                    { return handle_text(yytext); }

<STATE_OPENQASM>{D}+        { return(NUM); }
<STATE_OPENQASM>{NL}        { yy_pop_state(); }
<STATE_DEFINE>{ID}          { yylval.str = strdup(yytext); yy_push_state(STATE_BODY); return(IDENTIFIER); }
<STATE_DEFINE>"("           { return(LPAREN); }
<STATE_DEFINE>")"           { return(RPAREN); }
<STATE_DEFINE>","           { return(COMMA); }
<STATE_DEFINE>{S}+          { /* skip whitespace in define */ }
<STATE_BODY>"("             { return(LPAREN); }
<STATE_BODY>")"             { return(RPAREN); }
<STATE_BODY>","             { return(COMMA); }
<STATE_BODY>{S}+            { yylval.str = strdup(yytext); return(TEXT); }
<STATE_BODY>"\\"            { yy_pop_state(); yy_push_state(STATE_BODY_EXT); }
<STATE_BODY>{NL}            { yy_pop_state(); yy_pop_state(); return(NEWLINE); }
<STATE_BODY_EXT>{S}+        { /* skip leading whitespace after backslash */ }
<STATE_BODY_EXT>{NL}        { yy_pop_state(); yy_push_state(STATE_BODY); }
<STATE_BODY>{ID}            { yylval.str = strdup(yytext); return(PLACEHOLDER); }
<STATE_BODY>"##"{ID}        { yylval.str = strdup(yytext + 2); return(PLACEHOLDER); }
<STATE_BODY>"#"{ID}         { yylval.str = strdup(yytext + 1); return(STRINGIFIED); }
<STATE_BODY>[^ \t\v\n\f\r(),"]+     { yylval.str = strdup(yytext); return(TEXT); }
<STATE_BODY_EXT>{ID}        { yylval.str = strdup(yytext); return(PLACEHOLDER); }
<STATE_BODY_EXT>"##"{ID}    { yylval.str = strdup(yytext + 2); return(PLACEHOLDER); }
<STATE_BODY_EXT>"#"{ID}     { yylval.str = strdup(yytext + 1); return(STRINGIFIED); }
<STATE_BODY_EXT>[^ \t\v\n\f\r(),"]+ { yylval.str = strdup(yytext); return(TEXT); }

<STATE_INCLUDE>{STR}        {
    char* filename = strndup(yytext + 1, strlen(yytext) - 2);
    
    if (include_stack_ptr >= MAX_INCLUDE_DEPTH) {
        fprintf(stderr, "Include nesting too deep\n");
        exit(1);
    }
    
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Cannot open include file: %s\n", filename);
        free(filename);
        yy_pop_state();
        return 0;
    }
    
    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
    yy_switch_to_buffer(yy_create_buffer(f, YY_BUF_SIZE));
    free(filename);
    yy_pop_state();
}

<STATE_INCLUDE><<EOF>>      {
    if (--include_stack_ptr < 0) {
        yyterminate();
    } else {
       
        yy_delete_buffer(YY_CURRENT_BUFFER);
        yy_switch_to_buffer(include_stack[include_stack_ptr]);
        yy_pop_state();
  
    }
}

<STATE_EXPAND,STATE_IF><<EOF>>       {
    if (--expand_stack_ptr < 0) {
        yyterminate();
    } else {

        yy_delete_buffer(YY_CURRENT_BUFFER);
        yy_switch_to_buffer(expand_stack[expand_stack_ptr]);
        yy_pop_state();
    
    }
}

<STATE_INCLUDE>\<(\\.|[^\\>\n])*\> { yy_pop_state(); }
<STATE_INCLUDE>{NL}          { yy_pop_state(); }

<STATE_IF>"defined"          { return(DEFINED); }
<STATE_IF>"<"                { return(L_OP); }
<STATE_IF>">"                { return(G_OP); }
<STATE_IF>"<="               { return(LE_OP); }
<STATE_IF>">="               { return(GE_OP); }
<STATE_IF>{NL}               { yy_pop_state(); return(NEWLINE); }
<STATE_IF>"("                { return(LPAREN); }
<STATE_IF>")"                { return(RPAREN); }
<STATE_IF>"&&"               { return(AND_OP); }
<STATE_IF>"||"               { return(OR_OP); }
<STATE_IF>{ID}               { expand_expression(yytext); }
<STATE_IF>"=="			{ return(EQ_OP); }
<STATE_IF>"!="			{ return(NE_OP); }
<STATE_EXPAND,STATE_IF>0[xX]{H}+{IS}?             { yylval.i = strtol(yytext, NULL, 16); return(INTEGER); }
<STATE_EXPAND,STATE_IF>0[0-7]*{IS}?               { yylval.i = strtol(yytext, NULL, 8); return(INTEGER); }
<STATE_EXPAND,STATE_IF>[1-9]{D}*{IS}?             { yylval.i = strtol(yytext, NULL, 10); return(INTEGER); }
<STATE_EXPAND,STATE_IF>{D}+{E}{FS}?               { yylval.i = strtod(yytext, NULL); return(INTEGER); }
<STATE_EXPAND,STATE_IF>{D}*"."{D}+{E}?{FS}?       { yylval.f = strtof(yytext, NULL); return(FLOAT); }
<STATE_EXPAND,STATE_IF>{D}+"."{D}*{E}?{FS}?       { yylval.f = strtof(yytext, NULL); return(FLOAT); }
<STATE_EXPAND,STATE_IF>0[xX]{H}+{P}{FS}?          { yylval.f = strtof(yytext, NULL); return(FLOAT); }
<STATE_EXPAND,STATE_IF>0[xX]{H}*"."{H}+{P}?{FS}?  { yylval.f = strtof(yytext, NULL); return(FLOAT); }
<STATE_EXPAND,STATE_IF>0[xX]{H}+"."{H}*{P}?{FS}?  { yylval.f = strtof(yytext, NULL); return(FLOAT); }
<STATE_EXPAND,STATE_IF>{ID}               { expand_expression(yytext); }
<STATE_IFDEF>{ID}            { yylval.str = strdup(yytext); return(IDENTIFIER); }
<STATE_IFDEF>{NL}            { yy_pop_state(); return(NEWLINE); }
<STATE_IFNDEF>{ID}           { yylval.str = strdup(yytext); return(IDENTIFIER); }
<STATE_IFNDEF>{NL}           { yy_pop_state(); return(NEWLINE); }
<STATE_UNDEF>{ID}            { yylval.str = strdup(yytext); return(IDENTIFIER); }
<STATE_UNDEF>{NL}            { yy_pop_state(); return(NEWLINE); }
<STATE_EXPAND,STATE_IF,STATE_IFDEF,STATE_IFNDEF>[[:space:]]+                  { }
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

    expand_stack[expand_stack_ptr++] = YY_CURRENT_BUFFER;
    YY_BUFFER_STATE buffer = yy_scan_string(content);
    yy_push_state(STATE_EXPAND);
    yy_switch_to_buffer(buffer);


}

int handle_text(const char* text){
    yylval.str = try_expand(text);
    return(TEXT);
}
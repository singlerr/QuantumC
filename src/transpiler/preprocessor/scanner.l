D			[0-9]
L			[a-zA-Z_]
ID          L(L|D)*
NL          "\n"
S           [ \t]
STR         \"(\\.|[^\\"\n])*\"

%{

#include <stdio.h>
#include <stdlib.h>

%}

%option nounput
%option yylineno
%option noyywrap

%x DIR_DEFINE
%x DIR_BODY
%x DIR_BODY_EXT
%x DIR_INCLUDE
%x DIR_IFDEF
%x DIR_IF
%x DIR_IFNDEF
%x DIR_OPENQASM
%%

ID                            { return(IDENTIFIER); }
^"#""openqasm"                { yy_push_state(DIR_OPENQASM) return(OPENQASM); }
^"#""include"                 { yy_push_state(DIR_INCLUDE); return(INCLUDE); }
^"#""define"                  { yy_push_state(DIR_DEFINE); return(DEFINE); }     
^"#""ifdef"                   { yy_push_state(DIR_IFDEF); return(IFDEF); }
^"#""if"                      { yy_push_state(DIR_IF); return(IF); }
^"#""elif"                    { return(ELIF); }
^"#""ifndef"                  { yy_push_state(DIR_IFNDEF); return(IFNDEF); }
^"#""endif"                   { return(ENDIF); }

<DIR_OPENQASM>D+                    { return(NUM); }
<DIR_OPENQASM>NL                    { yy_pop_state(); }
<DIR_DEFINE>"("                     { return(LPAREN); }
<DIR_DEFINE>")"                     { return(RPAREN); }
<DIR_DEFINE>ID                      { return(IDENTIFIER); }
<DIR_DEFINE>","                     { return(COMMA); }
<DIR_DEFINE>"\\"                    { yy_push_state(DIR_BODY); }
<DIR_BODY>"\\"                      { yy_pop_state(); yy_push_state(DIR_BODY_EXT); }
<DIR_BODY>NL                        { yy_pop_state(); yy_pop_state(); }
<DIR_BODY_EXT>NL                    { yy_pop_state(); yy_push_state(DIR_BODY); } 
<DIR_BODY>ID                        { return(PLACEHOLDER); }
<DIR_BODY>^"##"ID                   { return(PLACEHOLDER); }
<DIR_BODY>^"#"ID                    { return(STRINGIFIED); }
<DIR_BODY>[^:ID:]                   { return(TEXT); }
<DIR_BODY_EXT>ID                    { return(PLACEHOLDER); }
<DIR_BODY_EXT>^"##"ID               { return(PLACEHOLDER); }
<DIR_BODY_EXT>^"#"ID                { return(STRINGIFIED); }
<DIR_BODY_EXT>[^:ID:]               { return(TEXT); }
<DIR_INCLUDE>STR                    { yy_pop_state(); return(INCLUDE_TARGET); }
<DIR_INCLUDE>\<(\\.|[^\\>\n])*\>    { yy_pop_state(); return(INCLUDE_TARGET); }
<DIR_INCLUDE>NL                     { yy_pop_state(); }
<DIR_IF>"defined"                   { return(INCLUDE_TARGET); }
<DIR_IF>"<"                         { return(L_OP); }
<DIR_IF>">"                         { return(G_OP); }
<DIR_IF>"<="                        { return(LE_OP); }
<DIR_IF>">="                        { return(GE_OP); }
<DIR_IF>NL                          { yy_pop_state(); }
<DIR_IF>"("                         { return(LPAREN); }
<DIR_IF>")"                         { return(RPAREN); }
<DIR_IF>"&&"                        { return(AND); }
<DIR_IF>"||"                        { return(OR);}
<DIR_IF>ID                          { return(IDENTIFIER); }
<DIR_IFDEF>ID                       { return(IDENTIFIER); }
<DIR_IFDEF>NL                       { yy_pop_state(); }
<DIR_IFNDEF>ID                      { return(IDENTIFIER); }
<DIR_IFNDEF>NL                      { yy_pop_state(); }

[ \t\n]+                            { ; }
.                                   { return(TEXT); }


%%

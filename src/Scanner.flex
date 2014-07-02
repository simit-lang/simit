%option noyywrap
%option nounput
%option noinput

%{
#include "Tokens.h"
#include "Logger.h"
#include <string>
%}

WHITESPACE    [ \t\n]
DIGIT         [0-9]
LETTER        [a-zA-Z]
ID            {LETTER}({LETTER}|{DIGIT})+

%%
"proc"                  { return PROC; }
"func"                  { return FUNC; }
"end"                   { return END; }

[\[\]\(\):,;]           { return yytext[0]; }

{WHITESPACE}
{DIGIT}+                { return INT_LITERAL; }
{DIGIT}+"."{DIGIT}+     { return FLOAT_LITERAL; }
{ID}                    { return ID; }

.                       { log(std::string("Unknown character [")+yytext[0]+"]");
                          return UNKNOWN;
                        }
%%

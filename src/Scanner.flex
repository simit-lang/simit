%option noyywrap
%option nounput
%option noinput

%{
#include "Tokens.h"
%}

WHITESPACE            [ \t\n]
DIGIT         [0-9]
LETTER        [a-zA-Z]
ID            {LETTER}({LETTER}|{DIGIT})+

%%
{WHITESPACE}                   
{DIGIT}+               { printf("Integer: %s\n", yytext); } 
{DIGIT}+"."{DIGIT}+    { printf("Float:   %s\n", yytext); } 
{ID}                   { printf("Ident:   %s\n", yytext); }

.                      { printf("Unknown character [%c]\n", yytext[0]); /*return UNKNOWN;*/ }
%%

int main() {
	yylex();
}

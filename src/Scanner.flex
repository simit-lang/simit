%option noyywrap
%option nounput
%option noinput

%{
  #include "Frontend.h"
  #include "Tokens.h"
  #include "Logger.h"
  #include <string>
  #include <stdlib.h>
  using namespace std;
%}
%x SLCOMMENT
%x MLCOMMENT


whitespace    [ \t\n]
digit         [0-9]
letter        [a-zA-Z]
ident         {letter}({letter}|{digit})*

%%
"struct"               { return STRUCT;    }
"Tensor"               { return TENSOR;    }
"const"                { return CONST;     }
"extern"               { return EXTERN;    }
"proc"                 { return PROC;      }
"func"                 { return FUNC;      }
"map"                  { return MAP;       }
"to"                   { return TO;        }
"with"                 { return WITH;      }
"reduce"               { return REDUCE;    }
"while"                { return WHILE;     }
"if"                   { return IF;        }
"elif"                 { return ELIF;      }
"else"                 { return ELSE;      }
"end"                  { return END;       }
"->"                   { return RARROW;    }

"int"                  { return INT;       }
"float"                { return FLOAT;     }

[\[\]\(\)\{\}:,;*+-/]  { return yytext[0]; }

{digit}+               { yylval.num  = atoi(yytext);     return INT_LITERAL; }
{digit}+"."{digit}+    { yylval.fnum = atof(yytext);     return FLOAT_LITERAL; }
{ident}                { yylval.string = strdup(yytext); return IDENT; }
{whitespace}           {}

 /* Comments */
"%{"                   { BEGIN(MLCOMMENT); }
<MLCOMMENT>\n          {}
<MLCOMMENT>.           {}
<MLCOMMENT><<EOF>>     { /* TODO: REPORT ERROR */ return UNKNOWN; }
<MLCOMMENT>"%}"        { BEGIN(INITIAL); }

%[^{]                  { BEGIN(SLCOMMENT); }
<SLCOMMENT>.           {}
<SLCOMMENT>\n          { BEGIN(INITIAL); }

.                      { /* TODO: REPORT ERROR */
                         log(string("Unknown character [")+yytext[0]+"]");
                         return UNKNOWN;
                       }
%%

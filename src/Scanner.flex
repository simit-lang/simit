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

  int yycolumn = 1;
  #define YY_USER_ACTION                                                      \
    yylloc->first_line = yylloc->last_line = yylineno;                        \
    yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
    yycolumn += yyleng;
%}
%option bison-bridge
%option bison-locations
%option yylineno

%x SLCOMMENT
%x MLCOMMENT

digit         [0-9]
letter        [a-zA-Z]
ident         ({letter}|_)({letter}|{digit}|_)*

int_literal   -?({digit}+|{digit}+([eE][-+]?{digit}+)?)
float_literal -?({digit}+|{digit}*\.{digit}+([eE][-+]?{digit}+)?)

%%
 /* Keywords and symbols */
"struct"              { return STRUCT;    }
"Tensor"              { return TENSOR;    }
"const"               { return CONST;     }
"extern"              { return EXTERN;    }
"proc"                { return PROC;      }
"func"                { return FUNC;      }
"map"                 { return MAP;       }
"to"                  { return TO;        }
"with"                { return WITH;      }
"reduce"              { return REDUCE;    }
"while"               { return WHILE;     }
"if"                  { return IF;        }
"elif"                { return ELIF;      }
"else"                { return ELSE;      }
"end"                 { return BLOCKEND;  }
"int"                 { return INT;       }
"float"               { return FLOAT;     }
"->"                  { return RARROW;    }
"("                   { return LP;        }
")"                   { return RP;        }
"["                   { return LB;        }
"]"                   { return RB;        }
"{"                   { return LC;        }
"}"                   { return RC;        }
","                   { return COMMA;     }
"."                   { return PERIOD;    }
":"                   { return COL;       }
";"                   { return SEMICOL;   }
"="                   { return ASSIGN;    }
"+"                   { return PLUS;      }
"-"                   { return MINUS;     }
"*"                   { return STAR;      }
"/"                   { return SLASH;     }
"\\"                  { return BACKSLASH; }
"^"                   { return EXP;       }
"'"                   { return TRANSPOSE; }

 /* Multi-line comments */
"%{"                  { BEGIN(MLCOMMENT); }
<MLCOMMENT>\n         { yycolumn = 1; }
<MLCOMMENT>.          {}
<MLCOMMENT><<EOF>>    { /* TODO: REPORT ERROR */ return UNKNOWN; }
<MLCOMMENT>"%}"       { BEGIN(INITIAL); }

 /* Single-line comments */
%[^{]                 { BEGIN(SLCOMMENT); }
<SLCOMMENT>.          {}
<SLCOMMENT>\n         { BEGIN(INITIAL); yycolumn = 1; }

 /* Identifiers */
{ident}               { yylval->string = strdup(yytext); return IDENT; }

 /* Literals */
{int_literal}         { yylval->num  = atoi(yytext); return INT_LITERAL;   }
{float_literal}       { yylval->fnum = atof(yytext); return FLOAT_LITERAL; }

 /* Whitespace */
[ \t]                 {}
\n                    { yycolumn = 1;                }
<<EOF>>               { yycolumn = 1; yyterminate(); }

 /* Unexpected (error) */
.                     { /* TODO: REPORT ERROR */
                        log(string("Unknown character [")+yytext[0]+"]");
                        return UNKNOWN; }
%%

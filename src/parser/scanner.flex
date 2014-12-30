%option c++
%option prefix="Simit"
%option yywrap nounput noinput

%{
#include "scanner.h"
#include <stdlib.h>
#include "parser.h"

#define YY_USER_ACTION yylloc->columns(yyleng);
#define yyterminate() return Parser::token::END
%}

%x SLCOMMENT
%x MLCOMMENT
%s SLTEST
%s MLTEST

digit         [0-9]
letter        [a-zA-Z]
ident         ({letter}|_)({letter}|{digit}|_)*

int_literal   ({digit}+)
float_literal ([-+]?{digit}*\.{digit}+|{digit}*(\.{digit}+)?[eE][-+]?{digit}+)

%%
%{
yylloc->step();
using namespace simit::internal;
%}

 /* Keywords and symbols */
"int"                 { return Parser::token::INT;       }
"float"               { return Parser::token::FLOAT;     }
"tensor"              { return Parser::token::TENSOR;    }
"element"             { return Parser::token::ELEMENT;   }
"set"                 { return Parser::token::SET;       }
"const"               { return Parser::token::CONST;     }
"extern"              { return Parser::token::EXTERN;    }
"proc"                { return Parser::token::PROC;      }
"func"                { return Parser::token::FUNC;      }
"map"                 { return Parser::token::MAP;       }
"to"                  { return Parser::token::TO;        }
"with"                { return Parser::token::WITH;      }
"reduce"              { return Parser::token::REDUCE;    }
"while"               { return Parser::token::WHILE;     }
"if"                  { return Parser::token::IF;        }
"elif"                { return Parser::token::ELIF;      }
"else"                { return Parser::token::ELSE;      }
"for"                 { return Parser::token::FOR;       }
"in"                  { return Parser::token::IN;        }
"end"                 { return Parser::token::BLOCKEND;  }
"return"              { return Parser::token::RETURN;    }
"print"               { return Parser::token::PRINT;     }

"->"                  { return Parser::token::RARROW;    }
"("                   { return Parser::token::LP;        }
")"                   { return Parser::token::RP;        }
"["                   { return Parser::token::LB;        }
"]"                   { return Parser::token::RB;        }
"{"                   { return Parser::token::LC;        }
"}"                   { return Parser::token::RC;        }
"<"                   { return Parser::token::LA;        }
">"                   { return Parser::token::RA;        }
","                   { return Parser::token::COMMA;     }
"."                   { return Parser::token::PERIOD;    }
":"                   { return Parser::token::COL;       }
";"                   { return Parser::token::SEMICOL;   }

"="                   { return Parser::token::ASSIGN;    }
"+"                   { return Parser::token::PLUS;      }
"-"                   { return Parser::token::MINUS;     }
"*"                   { return Parser::token::STAR;      }
"/"                   { return Parser::token::SLASH;     }
".*"                  { return Parser::token::DOTSTAR;   }
"./"                  { return Parser::token::DOTSLASH;  }
"\\"                  { return Parser::token::BACKSLASH; }
"^"                   { return Parser::token::EXP;       }
"'"                   { return Parser::token::TRANSPOSE; }

"=="                  { return Parser::token::EQ;        }
"!="                  { return Parser::token::NE;        }
"<="                  { return Parser::token::LE;        }
">="                  { return Parser::token::GE;        }
"and"                 { return Parser::token::AND;       }
"or"                  { return Parser::token::OR;        }
"not"                 { return Parser::token::NOT;       }
"xor"                 { return Parser::token::XOR;       }
"true"                { return Parser::token::TRUE;      }
"false"               { return Parser::token::FALSE;     }

 /* Tests */
"%!"                  { BEGIN(SLTEST); return Parser::token::TEST; }
<SLTEST>\n            { BEGIN(INITIAL); }
"%{!"                 { BEGIN(MLTEST); return Parser::token::TEST; }
<MLTEST>"%}"          { BEGIN(INITIAL); }

 /* Single-line comments */
%[^{]                 { BEGIN(SLCOMMENT); }
<SLCOMMENT>.          {}
<SLCOMMENT>\n         { BEGIN(INITIAL); yylloc->lines(yyleng); yylloc->step(); }

 /* Multi-line comments */
"%{"                  { BEGIN(MLCOMMENT); }
<MLCOMMENT>\n         { yylloc->lines(yyleng); yylloc->step(); }
<MLCOMMENT>.          {}
<MLCOMMENT><<EOF>>    { /*TODO: REPORT ERROR*/ return Parser::token::UNKNOWN; }
<MLCOMMENT>"%}"       { BEGIN(INITIAL); }

 /* Identifiers */
{ident}               { yylval->string = strdup(yytext);
                        return Parser::token::IDENT; }

 /* Literals */
{int_literal}         { yylval->num  = atoi(yytext);
                        return Parser::token::INT_LITERAL; }
{float_literal}       { yylval->fnum = atof(yytext);
                        return Parser::token::FLOAT_LITERAL; }

 /* Whitespace */
[ \t]                 {}
\n                    { yylloc->lines(yyleng); yylloc->step(); }
<<EOF>>               { yylloc->step(); yyterminate(); }

 /* Unexpected (error) */
.                     { return Parser::token::UNKNOWN; }
%%

namespace simit { namespace internal {

Scanner::Scanner(std::istream* in) : SimitFlexLexer(in) {
}

Scanner::~Scanner() {
}

}} // namespace simit::internal

#ifdef yylex
#undef yylex
#endif

int SimitFlexLexer::yylex() {
    iassert(false) << "SimitFlexLexer::yylex() should never be called~";
    return 0;
}

int SimitFlexLexer::yywrap() {
    return 1;
}

#ifndef __MYSCANNER_HPP__
#define __MYSCANNER_HPP__

//#undef  YY_DECL
//#define YY_DECL int simit::internal::Scanner::yylex()

#ifndef YY_DECL
#define YY_DECL                                         \
    simit::internal::Parser::token_type                 \
    simit::internal::Scanner::lex(                      \
        simit::internal::Parser::semantic_type* yylval, \
        simit::internal::Parser::location_type* yylloc)
#endif

#ifndef __FLEX_LEXER_H
#include "flexscanner.h"
#endif

#include "parser.h"

namespace simit { namespace internal {

class Scanner : public yyFlexLexer{
 public:
  Scanner(std::istream *arg_yyin = 0);
  virtual ~Scanner();

  virtual Parser::token_type lex(Parser::semantic_type* yylval,
                                 Parser::location_type* yylloc);



//  Scanner(std::istream *yyin) : yyFlexLexer(yyin), yylval(NULL){};

//  int yylex(Parser::semantic_type *yylval, Parser::location_type *yylloc) {
//    this->yylval = yylval;
//    return( yylex() );
//  }


 private:
//  Parser::semantic_type *yylval;
//  Parser::location_type *yylloc;
//  int yylex();
};

}} // namespace simit::internal

#endif


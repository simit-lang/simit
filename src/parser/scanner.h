#ifndef __MYSCANNER_HPP__
#define __MYSCANNER_HPP__

#ifndef YY_DECL
#define YY_DECL                                         \
    simit::internal::Parser::token_type                 \
    simit::internal::Scanner::lex(                      \
        simit::internal::Parser::semantic_type* yylval, \
        simit::internal::Parser::location_type* yylloc)
#endif

#ifndef __FLEX_LEXER_H
#include "FlexLexer.h"
#endif

#include "parser.h"

namespace simit { namespace internal {

class Scanner : public yyFlexLexer{
 public:
  Scanner(std::istream *arg_yyin = 0);
  ~Scanner();

  Parser::token_type lex(Parser::semantic_type* yylval,
                         Parser::location_type* yylloc);
};

}} // namespace simit::internal

#endif


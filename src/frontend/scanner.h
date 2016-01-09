#ifndef SCANNER_NEW_H
#define SCANNER_NEW_H

#include <sstream>
#include <string>
#include <vector>

#include "token.h"
#include "error.h"

namespace simit { 
namespace internal {

class Scanner {
public:
  Scanner(std::vector<ParseError> *errors) : errors(errors) {}

  TokenStream lex(std::istream &);

private:
  enum class ScanState {INITIAL, SLTEST, MLTEST};

private:
  static Token::Type getTokenType(const std::string);
  
  void reportError(std::string msg, unsigned line, unsigned col) {
    errors->push_back(ParseError(line, col, line, col, msg));
  }

private:
  std::vector<ParseError> *errors;
};

}
}

#endif

#ifndef SCANNER_NEW_H
#define SCANNER_NEW_H

#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "error.h"

namespace simit { 
namespace internal {

struct Token {
  enum class Type {
    END,
    UNKNOWN,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    IDENT,
    AND,
    OR,
    NEG,
    INT,
    FLOAT,
    BOOL,
    STRING,
    TENSOR,
    ELEMENT,
    SET,
    VAR,
    CONST,
    EXTERN,
    PROC,
    FUNC,
    INOUT,
    MAP,
    TO,
    WITH,
    REDUCE,
    WHILE,
    DO,
    IF,
    ELIF,
    ELSE,
    FOR,
    IN,
    BLOCKEND,
    RETURN,
    TEST,
    PRINT,
    RARROW,
    LP,
    RP,
    LB,
    RB,
    LC,
    RC,
    LA,
    RA,
    COMMA,
    PERIOD,
    COL,
    SEMICOL,
    ASSIGN,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    DOTSTAR,
    DOTSLASH,
    EXP,
    TRANSPOSE,
    BACKSLASH,
    EQ,
    NE,
    LE,
    GE,
    NOT,
    XOR,
    TRUE,
    FALSE
  };

  Type type;
  union {
    int num;
    double fnum;
  };
  std::string str;
  unsigned lineBegin;
  unsigned colBegin;
  unsigned lineEnd;
  unsigned colEnd;
 
  static std::string tokenTypeString(const Token::Type);
  
  std::string toString() const;
  friend std::ostream &operator <<(std::ostream &, const Token &);
};

class TokenStream {
public:
  inline void addToken(Token newToken) { tokens.push_back(newToken); }
  inline void addToken(Token::Type type, unsigned line, 
                       unsigned col, unsigned len = 1) {
    Token newToken;
    newToken.type = type;
    newToken.lineBegin = line;
    newToken.colBegin = col;
    newToken.lineEnd = line;
    newToken.colEnd = col + len - 1;
    tokens.push_back(newToken);
  }

  inline void skip() { tokens.pop_front(); }
  inline bool consume(Token::Type type) {
    if (tokens.front().type == type) {
      tokens.pop_front();
      return true;
    }
    return false;
  }
  inline Token peek(unsigned k) {
    if (k == 0) {
      return tokens.front();
    }

    std::list<Token>::const_iterator it = tokens.cbegin();
    for (unsigned i = 0; i < k && it != tokens.cend(); ++i, ++it) {}

    if (it == tokens.cend()) {
      Token endToken = Token();
      endToken.type = Token::Type::END;
      return endToken;
    }

    return *it;
  }


  friend std::ostream &operator <<(std::ostream &, TokenStream);

private:
  std::list<Token> tokens;
};

class ScannerNew {
public:
  ScannerNew(std::vector<ParseError> *errors) : errors(errors) {}

  TokenStream lex(std::istream &);

private:
  enum class ScanState { INITIAL, SLTEST, MLTEST };
  
  Token::Type getTokenType(const std::string);
  
  void reportError(const std::string msg, unsigned line, unsigned col) {
    errors->push_back(ParseError(line, col, line, col, msg));
  }

  std::vector<ParseError> *errors;
};

}
}

#endif

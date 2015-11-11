#ifndef SCANNER_NEW_H
#define SCANNER_NEW_H

#include <list>
#include <sstream>
#include <string>

namespace simit { 
namespace internal {

enum class TokenType {
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
  UNTIL,
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

struct Token {
  TokenType type;
  union {
    int num;
    double fnum;
    bool boolean;
  };
  std::string str;

  Token() : type(TokenType::UNKNOWN) {}
  Token(TokenType type) : type(type) {}
  Token(int num) : type(TokenType::INT_LITERAL), num(num) {}
  Token(double fnum) : type(TokenType::FLOAT_LITERAL), fnum(fnum) {}

  friend std::ostream &operator <<(std::ostream &, Token);
};

class TokenList {
  public:
    inline void addToken(Token newToken) { tokens.push_back(newToken); }
    inline void addToken(TokenType type) { tokens.push_back(Token(type)); }

    inline bool consume(TokenType type) {
      if (tokens.front().type == type) {
        tokens.pop_front();
        return true;
      }

      return false;
    }
    inline Token peek(unsigned int k) {
      if (k == 0) return tokens.front();

      std::list<Token>::const_iterator it = tokens.cbegin();
      for (unsigned int i = 0; i < k && it != tokens.cend(); ++i, ++it) {}
      return (it == tokens.cend()) ? Token(TokenType::END) : *it;
    }


    friend std::ostream &operator <<(std::ostream &, TokenList);

  private:
    std::list<Token> tokens;
};

class ScannerNew {
public:
  static TokenList lex(std::istream &);

private:
  enum class ScanState {
    INITIAL,
    SLTEST,
    MLTEST
  };
  
  static TokenType getTokenType(std::string);
};

}
}

#endif

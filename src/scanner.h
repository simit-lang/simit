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
  Token() : Token(0, 0) {}
  Token(unsigned lineNum, unsigned colNum) : 
    type(TokenType::UNKNOWN), lineNum(lineNum), colNum(colNum) {}
  Token(TokenType type, unsigned lineNum, unsigned colNum) : 
    type(type), lineNum(lineNum), colNum(colNum) {}
  Token(int num, unsigned lineNum, unsigned colNum) : 
    type(TokenType::INT_LITERAL), num(num), lineNum(lineNum), colNum(colNum) {}
  Token(double fnum, unsigned lineNum, unsigned colNum) : 
    type(TokenType::FLOAT_LITERAL), fnum(fnum), lineNum(lineNum), 
    colNum(colNum) {}

  friend std::ostream &operator <<(std::ostream &, Token);
  
  TokenType type;
  union {
    int num;
    double fnum;
    bool boolean;
  };
  std::string str;
  unsigned lineNum;
  unsigned colNum;
};

class TokenStream {
  public:
    inline void addToken(Token newToken) { tokens.push_back(newToken); }
    inline void addToken(TokenType type, unsigned lineNum, 
                         unsigned colNum) {
      tokens.push_back(Token(type, lineNum, colNum));
    }

    inline void skip() { tokens.pop_front(); }
    inline bool consume(TokenType type) {
      if (tokens.front().type == type) {
        tokens.pop_front();
        return true;
      }

      return false;
    }
    inline Token peek(unsigned k) {
      if (k == 0) return tokens.front();

      std::list<Token>::const_iterator it = tokens.cbegin();
      for (unsigned i = 0; i < k && it != tokens.cend(); ++i, ++it) {}
      return (it == tokens.cend()) ? Token(TokenType::END, -1, -1) : *it;
    }


    friend std::ostream &operator <<(std::ostream &, TokenStream);

  private:
    std::list<Token> tokens;
};

class ScannerNew {
public:
  static TokenStream lex(std::istream &);

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

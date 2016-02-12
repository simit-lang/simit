#ifndef TOKEN_H
#define TOKEN_H

#include <list>
#include <sstream>
#include <string>

namespace simit { 
namespace internal {

struct Token {
public:
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
    COMPLEX,
    STRING,
    TENSOR,
    MATRIX,
    VECTOR,
    ELEMENT,
    SET,
    VAR,
    CONST,
    EXTERN,
    EXPORT,
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

public:
  Type        type;
  union {
    int       num;
    double    fnum;
  };
  std::string str;
  unsigned    lineBegin;
  unsigned    colBegin;
  unsigned    lineEnd;
  unsigned    colEnd;

public:
  static std::string tokenTypeString(Token::Type);
  
  std::string toString() const;
  
  friend std::ostream &operator <<(std::ostream &, const Token &);
};

class TokenStream {
public:
  void addToken(Token newToken) { tokens.push_back(newToken); }
  void addToken(Token::Type, unsigned, unsigned, unsigned = 1);

  Token peek(unsigned) const;
  
  void skip() { tokens.pop_front(); }
  bool consume(Token::Type);

  friend std::ostream &operator <<(std::ostream &, const TokenStream &);

private:
  std::list<Token> tokens;
};

}
}

#endif


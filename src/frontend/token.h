#ifndef TOKEN_H
#define TOKEN_H

#include <list>
#include <sstream>
#include <string>

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
    COMPLEX,
    STRING,
    TENSOR,
    MATRIX,
    VECTOR,
    ELEMENT,
    SET,
    GRID,
    OPAQUE,
    VAR,
    CONST,
    EXTERN,
    EXPORT,
    FUNC,
    INOUT,
    APPLY,
    MAP,
    TO,
    WITH,
    THROUGH,
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
    PRINTLN,
    NEW,
    DELETE,
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
    FALSE,
    IVAR
  };

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

  static std::string tokenTypeString(Token::Type);
  
  std::string toString() const;
  
  friend std::ostream &operator <<(std::ostream &, const Token &);
};

struct TokenStream {
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


#include <cctype>
#include <string>
#include <cstdlib>
#include <iostream>

#include "token.h"
#include "error.h"

namespace simit { 
namespace internal {

std::string Token::tokenTypeString(Token::Type type) {
  switch (type) {
    case Token::Type::END:
      return "end of file";
    case Token::Type::UNKNOWN:
      return "unknown";
    case Token::Type::INT_LITERAL:
      return "an integer literal";
    case Token::Type::FLOAT_LITERAL:
      return "a float literal";
    case Token::Type::STRING_LITERAL:
      return "a string literal";
    case Token::Type::IDENT:
      return "an identifier";
    case Token::Type::AND:
      return "'and'";
    case Token::Type::OR:
      return "'or'";
    case Token::Type::NEG:
      return "'-'";
    case Token::Type::INT:
      return "'int'";
    case Token::Type::FLOAT:
      return "'float'";
    case Token::Type::BOOL:
      return "'bool'";
    case Token::Type::COMPLEX:
      return "'complex'";
    case Token::Type::STRING:
      return "'string'";
    case Token::Type::TENSOR:
      return "'tensor'";
    case Token::Type::MATRIX:
      return "'matrix'";
    case Token::Type::VECTOR:
      return "'vector'";
    case Token::Type::ELEMENT:
      return "'element'";
    case Token::Type::SET:
      return "'set'";
    case Token::Type::LATTICE:
      return "'lattice'";
    case Token::Type::VAR:
      return "'var'";
    case Token::Type::CONST:
      return "'const'";
    case Token::Type::EXTERN:
      return "'extern'";
    case Token::Type::EXPORT:
      return "'export'";
    case Token::Type::PROC:
      return "'proc'";
    case Token::Type::FUNC:
      return "'func'";
    case Token::Type::INOUT:
      return "'inout'";
    case Token::Type::APPLY:
      return "'apply'";
    case Token::Type::MAP:
      return "'map'";
    case Token::Type::TO:
      return "'to'";
    case Token::Type::WITH:
      return "'with'";
    case Token::Type::THROUGH:
      return "'through'";
    case Token::Type::REDUCE:
      return "'reduce'";
    case Token::Type::WHILE:
      return "'while'";
    case Token::Type::DO:
      return "'do'";
    case Token::Type::IF:
      return "'if'";
    case Token::Type::ELIF:
      return "'elif'";
    case Token::Type::ELSE:
      return "'else'";
    case Token::Type::FOR:
      return "'for'";
    case Token::Type::IN:
      return "'in'";
    case Token::Type::BLOCKEND:
      return "'end'";
    case Token::Type::RETURN:
      return "'return'";
    case Token::Type::TEST:
      return "'test'";
    case Token::Type::PRINT:
      return "'print'";
    case Token::Type::PRINTLN:
      return "'println'";
    case Token::Type::RARROW:
      return "'->'";
    case Token::Type::LP:
      return "'('";
    case Token::Type::RP:
      return "')'";
    case Token::Type::LB:
      return "'['";
    case Token::Type::RB:
      return "']'";
    case Token::Type::LC:
      return "'{'";
    case Token::Type::RC:
      return "'}'";
    case Token::Type::LA:
      return "'<'";
    case Token::Type::RA:
      return "'>'";
    case Token::Type::COMMA:
      return "','";
    case Token::Type::PERIOD:
      return "'.'";
    case Token::Type::COL:
      return "':'";
    case Token::Type::SEMICOL:
      return "';'";
    case Token::Type::ASSIGN:
      return "'='";
    case Token::Type::PLUS:
      return "'+'";
    case Token::Type::MINUS:
      return "'-'";
    case Token::Type::STAR:
      return "'*'";
    case Token::Type::SLASH:
      return "'/'";
    case Token::Type::DOTSTAR:
      return "'.*'";
    case Token::Type::DOTSLASH:
      return "'./'";
    case Token::Type::EXP:
      return "'^'";
    case Token::Type::TRANSPOSE:
      return "'''";
    case Token::Type::BACKSLASH:
      return "'\\'";
    case Token::Type::EQ:
      return "'=='";
    case Token::Type::NE:
      return "'!='";
    case Token::Type::LE:
      return "'<='";
    case Token::Type::GE:
      return "'>='";
    case Token::Type::NOT:
      return "'not'";
    case Token::Type::XOR:
      return "'xor'";
    case Token::Type::TRUE:
      return "'true'";
    case Token::Type::FALSE:
      return "'false'";
    default:
      unreachable;
      return "";
  }
}

std::string Token::toString() const {
  switch (type) {
    case Token::Type::INT_LITERAL:
      return "'" + std::to_string(num) + "'";
    case Token::Type::FLOAT_LITERAL:
      return "'" + std::to_string(fnum) + "'";
    case Token::Type::STRING_LITERAL:
      return "'\"" + str + "\"'";
    case Token::Type::IDENT:
      return "'" + str + "'";
    default:
      return tokenTypeString(type);
  }
}

std::ostream &operator <<(std::ostream &out, const Token &token) {
  out << "(" << Token::tokenTypeString(token.type);
  switch (token.type) {
    case Token::Type::INT_LITERAL:
      out << ", " << token.num;
      break;
    case Token::Type::FLOAT_LITERAL:
      out << ", " << token.fnum;
      break;
    case Token::Type::STRING_LITERAL:
      out << ", \"" << token.str << "\"";
      break;
    case Token::Type::IDENT:
      out << ", " << token.str;
      break;
    default:
      break;
  }
  out << ", " << token.lineBegin << ":" << token.colBegin << "-" 
      << token.lineEnd << ":" << token.colEnd << ")";
  return out;
}

void TokenStream::addToken(Token::Type type, unsigned line, 
                           unsigned col, unsigned len) {
  Token newToken;
  
  newToken.type = type;
  newToken.lineBegin = line;
  newToken.colBegin = col;
  newToken.lineEnd = line;
  newToken.colEnd = col + len - 1;
  
  tokens.push_back(newToken);
}
  
bool TokenStream::consume(Token::Type type) {
  if (tokens.front().type == type) {
    tokens.pop_front();
    return true;
  }

  return false;
}
  
Token TokenStream::peek(unsigned k) const {
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

std::ostream &operator <<(std::ostream &out, const TokenStream &tokens) {
  for (auto it = tokens.tokens.cbegin(); it != tokens.tokens.cend(); ++it) {
    out << *it << std::endl;
  }
  return out;
}

}
}


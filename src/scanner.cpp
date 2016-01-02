#include <cctype>
#include <string>
#include <cstdlib>
#include <iostream>

#include "scanner.h"
#include "error.h"

namespace simit { 
namespace internal {

std::string Token::tokenTypeString(const Token::Type type) {
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
    case Token::Type::STRING:
      return "'string'";
    case Token::Type::TENSOR:
      return "'tensor'";
    case Token::Type::ELEMENT:
      return "'element'";
    case Token::Type::SET:
      return "'set'";
    case Token::Type::VAR:
      return "'var'";
    case Token::Type::CONST:
      return "'const'";
    case Token::Type::EXTERN:
      return "'extern'";
    case Token::Type::PROC:
      return "'proc'";
    case Token::Type::FUNC:
      return "'func'";
    case Token::Type::INOUT:
      return "'inout'";
    case Token::Type::MAP:
      return "'map'";
    case Token::Type::TO:
      return "'to'";
    case Token::Type::WITH:
      return "'with'";
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
      return "'\'";
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

std::ostream &operator <<(std::ostream &out, TokenStream tokens) {
  for (std::list<Token>::const_iterator it = tokens.tokens.cbegin();
      it != tokens.tokens.cend(); ++it) {
    out << *it << std::endl;
  }
  return out;
}

Token::Type ScannerNew::getTokenType(const std::string token) {
  if (token == "int") return Token::Type::INT;
  if (token == "float") return Token::Type::FLOAT;
  if (token == "bool") return Token::Type::BOOL;
  if (token == "string") return Token::Type::STRING;
  if (token == "tensor") return Token::Type::TENSOR;
  if (token == "matrix") return Token::Type::TENSOR;
  if (token == "vector") return Token::Type::TENSOR;
  if (token == "element") return Token::Type::ELEMENT;
  if (token == "set") return Token::Type::SET;
  if (token == "var") return Token::Type::VAR;
  if (token == "const") return Token::Type::CONST;
  if (token == "extern") return Token::Type::EXTERN;
  if (token == "proc") return Token::Type::PROC;
  if (token == "func") return Token::Type::FUNC;
  if (token == "inout") return Token::Type::INOUT;
  if (token == "map") return Token::Type::MAP;
  if (token == "to") return Token::Type::TO;
  if (token == "with") return Token::Type::WITH;
  if (token == "reduce") return Token::Type::REDUCE;
  if (token == "while") return Token::Type::WHILE;
  if (token == "do") return Token::Type::DO;
  if (token == "if") return Token::Type::IF;
  if (token == "elif") return Token::Type::ELIF;
  if (token == "else") return Token::Type::ELSE;
  if (token == "for") return Token::Type::FOR;
  if (token == "in") return Token::Type::IN;
  if (token == "end") return Token::Type::BLOCKEND;
  if (token == "return") return Token::Type::RETURN;
  if (token == "print") return Token::Type::PRINT;
  if (token == "and") return Token::Type::AND; 
  if (token == "or") return Token::Type::OR;
  if (token == "not") return Token::Type::NOT;
  if (token == "xor") return Token::Type::XOR;
  if (token == "true") return Token::Type::TRUE;
  if (token == "false") return Token::Type::FALSE;
 
  // If string does not correspond to a keyword, assume it is an identifier.
  return Token::Type::IDENT;
}

TokenStream ScannerNew::lex(std::istream &programStream) {
  TokenStream tokens;
  unsigned line = 1;
  unsigned col = 1;
  ScanState state = ScanState::INITIAL;

  while (programStream.peek() != EOF) {
    if (programStream.peek() == '_' || std::isalpha(programStream.peek())) {
      std::string tokenString(1, programStream.get());

      while (programStream.peek() == '_' || 
          std::isalnum(programStream.peek())) {
        tokenString += programStream.get();
      }

      Token newToken;
      newToken.type = getTokenType(tokenString);
      newToken.lineBegin = line;
      newToken.colBegin = col;
      newToken.lineEnd = line;
      newToken.colEnd = col + tokenString.length() - 1;
      if (newToken.type == Token::Type::IDENT) {
        newToken.str = tokenString;
      }
      tokens.addToken(newToken);

      col += tokenString.length();
    } else {
      switch (programStream.peek()) {
        case '(':
          programStream.get();
          tokens.addToken(Token::Type::LP, line, col++);
          break;
        case ')':
          programStream.get();
          tokens.addToken(Token::Type::RP, line, col++);
          break;
        case '[':
          programStream.get();
          tokens.addToken(Token::Type::LB, line, col++);
          break;
        case ']':
          programStream.get();
          tokens.addToken(Token::Type::RB, line, col++);
          break;
        case '{':
          programStream.get();
          tokens.addToken(Token::Type::LC, line, col++);
          break;
        case '}':
          programStream.get();
          tokens.addToken(Token::Type::RC, line, col++);
          break;
        case '<':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(Token::Type::LE, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(Token::Type::LA, line, col++);
          }
          break;
        case '>':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(Token::Type::GE, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(Token::Type::RA, line, col++);
          }
          break;
        case ',':
          programStream.get();
          tokens.addToken(Token::Type::COMMA, line, col++);
          break;
        case '.':
          programStream.get();
          switch (programStream.peek()) {
            case '*':
              programStream.get();
              tokens.addToken(Token::Type::DOTSTAR, line, col, 2);
              col += 2;
              break;
            case '/':
              programStream.get();
              tokens.addToken(Token::Type::DOTSLASH, line, col, 2);
              col += 2;
              break;
            default:
              tokens.addToken(Token::Type::PERIOD, line, col++);
              break;
          }
          break;
        case ':':
          programStream.get();
          tokens.addToken(Token::Type::COL, line, col++);
          break;
        case ';':
          programStream.get();
          tokens.addToken(Token::Type::SEMICOL, line, col++);
          break;
        case '=':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(Token::Type::EQ, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(Token::Type::ASSIGN, line, col++);
          }
          break;
        case '*':
          programStream.get();
          tokens.addToken(Token::Type::STAR, line, col++);
          break;
        case '/':
          programStream.get();
          tokens.addToken(Token::Type::SLASH, line, col++);
          break;
        case '\\':
          programStream.get();
          tokens.addToken(Token::Type::BACKSLASH, line, col++);
          break;
        case '^':
          programStream.get();
          tokens.addToken(Token::Type::EXP, line, col++);
          break;
        case '\'':
          programStream.get();
          tokens.addToken(Token::Type::TRANSPOSE, line, col++);
          break;
        case '!':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(Token::Type::NE, line, col, 2);
            col += 2;
          } else {
            reportError("unexpected symbol '!'", line, col++);
            while (programStream.peek() != EOF && 
                !std::isspace(programStream.peek())) {
              programStream.get();
              ++col;
            }
          }
          break;
        case '%':
          programStream.get();
          switch (programStream.peek()) {
            case '!':
              programStream.get();
              tokens.addToken(Token::Type::TEST, line, col, 2);
              state = ScanState::SLTEST;
              col += 2;
              break;
            case '{':
              if (programStream.peek() == '!') {
                programStream.get();
                tokens.addToken(Token::Type::TEST, line, col, 2);
                state = ScanState::MLTEST;
                col += 2;
              } else {
                std::string comment;
                while (programStream.peek() != EOF) {
                  if (programStream.peek() == '%') {
                    programStream.get();

                    if (programStream.peek() == '\n') {
                      ++line;
                      col = 1;
                    } else {
                      col += 2;
                    }

                    if (programStream.peek() == '}') {
                      programStream.get();
                      // TODO: emit COMMENT token
                      break;
                    } else {
                      comment += '%';
                      comment += programStream.get();
                    }
                  } else {
                    if (programStream.peek() == '\n') {
                      ++line;
                      col = 1;
                    } else {
                      ++col;
                    }

                    comment += programStream.get();
                  }
                }

                if (programStream.peek() == EOF) {
                  reportError("unclosed comment", line, col);
                }
              }
              break;
            case '}':
            {
              programStream.get();
              if (state == ScanState::MLTEST) {
                state = ScanState::INITIAL;
              } else {
                reportError("could not find corresponding '!%{'", line, col);
              }
              col += 2;
              break;
            }
            default:
            {
              std::string comment;
              while (programStream.peek() != '\n' && 
                  programStream.peek() != EOF) {
                comment += programStream.get();
              }

              col += (comment.length() + 1);
              // TODO: emit COMMENT token
              break;
            }
          }
          break;
        case '\n':
          programStream.get();
          if (state == ScanState::SLTEST) {
            state = ScanState::INITIAL;
          }
          ++line;
          col = 1;
          break;
        case ' ':
        case '\t':
          programStream.get();
          ++col;
          break;
        case '+':
          programStream.get();
          tokens.addToken(Token::Type::PLUS, line, col++);
          break;
        case '-': 
          programStream.get();
          if (programStream.peek() == '>') {
            programStream.get();
            tokens.addToken(Token::Type::RARROW, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(Token::Type::MINUS, line, col++);
          }
          break;
        default: 
        {
          Token newToken;
          newToken.type = Token::Type::INT_LITERAL;
          newToken.lineBegin = line;
          newToken.colBegin = col;

          if (programStream.peek() != '.' && 
              !std::isdigit(programStream.peek())) {
            std::stringstream errMsg;
            errMsg << "unexpected symbol '" 
                   << (char)programStream.peek() << "'";
            reportError(errMsg.str(), line, col);
            
            while (programStream.peek() != EOF && 
                !std::isspace(programStream.peek())) {
              programStream.get();
              ++col;
            }
            break;
          }

          std::string tokenString;
          while (std::isdigit(programStream.peek())) {
            tokenString += programStream.get();
            ++col;
          }

          if (programStream.peek() == '.') {
            newToken.type = Token::Type::FLOAT_LITERAL;
            tokenString += programStream.get();
            ++col;

            if (!std::isdigit(programStream.peek())) {
              std::stringstream errMsg;
              errMsg << "unexpected symbol '" 
                     << (char)programStream.peek() << "'";
              reportError(errMsg.str(), line, col);
              
              while (programStream.peek() != EOF && 
                  !std::isspace(programStream.peek())) {
                programStream.get();
                ++col;
              }
              break;
            }
            tokenString += programStream.get();
            ++col;

            while (std::isdigit(programStream.peek())) {
              tokenString += programStream.get();
              ++col;
            }
          }

          if (programStream.peek() == 'e' || programStream.peek() == 'E') {
            newToken.type = Token::Type::FLOAT_LITERAL;
            tokenString += programStream.get();
            ++col;

            if (programStream.peek() == '+' || programStream.peek() == '-') {
              tokenString += programStream.get();
              ++col;
            }

            if (!std::isdigit(programStream.peek())) {
              std::stringstream errMsg;
              errMsg << "unexpected symbol '" 
                     << (char)programStream.peek() << "'";
              reportError(errMsg.str(), line, col);
              
              while (programStream.peek() != EOF && 
                  !std::isspace(programStream.peek())) {
                programStream.get();
                ++col;
              }
              break;
            }
            tokenString += programStream.get();
            ++col;

            while (std::isdigit(programStream.peek())) {
              tokenString += programStream.get();
              ++col;
            }
          }

          char *end;
          if (newToken.type == Token::Type::INT_LITERAL) {
            newToken.num = std::strtol(tokenString.c_str(), &end, 0);
          } else {
            newToken.fnum = std::strtod(tokenString.c_str(), &end);
          }
          newToken.lineEnd = line;
          newToken.colEnd = col - 1;
          tokens.addToken(newToken);
          break;
        }
      }
    }
  }

  if (state != ScanState::INITIAL) {
    reportError("unclosed test", line, col);
  }

  tokens.addToken(Token::Type::END, line, col);
  return tokens;
}

}
}

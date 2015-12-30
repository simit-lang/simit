#include <cctype>
#include <string>
#include <cstdlib>
#include <iostream>

#include "scanner.h"

namespace simit { 
namespace internal {

std::ostream &operator <<(std::ostream &out, Token token) {
  out << "(";
  switch (token.type) {
    case TokenType::END:
      out << "END";
      break;
    case TokenType::UNKNOWN:
      out << "UNKNOWN";
      break;
    case TokenType::INT_LITERAL:
      out << "INT_LITERAL";
      break;
    case TokenType::FLOAT_LITERAL:
      out << "FLOAT_LITERAL";
      break;
    case TokenType::STRING_LITERAL:
      out << "STRING_LITERAL";
      break;
    case TokenType::IDENT:
      out << "IDENT";
      break;
    case TokenType::AND:
      out << "AND";
      break;
    case TokenType::OR:
      out << "OR";
      break;
    case TokenType::NEG:
      out << "NEG";
      break;
    case TokenType::INT:
      out << "INT";
      break;
    case TokenType::FLOAT:
      out << "FLOAT";
      break;
    case TokenType::BOOL:
      out << "BOOL";
      break;
    case TokenType::STRING:
      out << "STRING";
      break;
    case TokenType::TENSOR:
      out << "TENSOR";
      break;
    case TokenType::ELEMENT:
      out << "ELEMENT";
      break;
    case TokenType::SET:
      out << "SET";
      break;
    case TokenType::VAR:
      out << "VAR";
      break;
    case TokenType::CONST:
      out << "CONST";
      break;
    case TokenType::EXTERN:
      out << "EXTERN";
      break;
    case TokenType::PROC:
      out << "PROC";
      break;
    case TokenType::FUNC:
      out << "FUNC";
      break;
    case TokenType::INOUT:
      out << "INOUT";
      break;
    case TokenType::MAP:
      out << "MAP";
      break;
    case TokenType::TO:
      out << "TO";
      break;
    case TokenType::WITH:
      out << "WITH";
      break;
    case TokenType::REDUCE:
      out << "REDUCE";
      break;
    case TokenType::WHILE:
      out << "WHILE";
      break;
    case TokenType::DO:
      out << "DO";
      break;
    case TokenType::IF:
      out << "IF";
      break;
    case TokenType::ELIF:
      out << "ELIF";
      break;
    case TokenType::ELSE:
      out << "ELSE";
      break;
    case TokenType::FOR:
      out << "FOR";
      break;
    case TokenType::IN:
      out << "IN";
      break;
    case TokenType::BLOCKEND:
      out << "BLOCKEND";
      break;
    case TokenType::RETURN:
      out << "RETURN";
      break;
    case TokenType::TEST:
      out << "TEST";
      break;
    case TokenType::PRINT:
      out << "PRINT";
      break;
    case TokenType::RARROW:
      out << "RARROW";
      break;
    case TokenType::LP:
      out << "LP";
      break;
    case TokenType::RP:
      out << "RP";
      break;
    case TokenType::LB:
      out << "LB";
      break;
    case TokenType::RB:
      out << "RB";
      break;
    case TokenType::LC:
      out << "LC";
      break;
    case TokenType::RC:
      out << "RC";
      break;
    case TokenType::LA:
      out << "LA";
      break;
    case TokenType::RA:
      out << "RA";
      break;
    case TokenType::COMMA:
      out << "COMMA";
      break;
    case TokenType::PERIOD:
      out << "PERIOD";
      break;
    case TokenType::COL:
      out << "COL";
      break;
    case TokenType::SEMICOL:
      out << "SEMICOL";
      break;
    case TokenType::ASSIGN:
      out << "ASSIGN";
      break;
    case TokenType::PLUS:
      out << "PLUS";
      break;
    case TokenType::MINUS:
      out << "MINUS";
      break;
    case TokenType::STAR:
      out << "STAR";
      break;
    case TokenType::SLASH:
      out << "SLASH";
      break;
    case TokenType::DOTSTAR:
      out << "DOTSTAR";
      break;
    case TokenType::DOTSLASH:
      out << "DOTSLASH";
      break;
    case TokenType::EXP:
      out << "EXP";
      break;
    case TokenType::TRANSPOSE:
      out << "TRANSPOSE";
      break;
    case TokenType::BACKSLASH:
      out << "BACKSLASH";
      break;
    case TokenType::EQ:
      out << "EQ";
      break;
    case TokenType::NE:
      out << "NE";
      break;
    case TokenType::LE:
      out << "LE";
      break;
    case TokenType::GE:
      out << "GE";
      break;
    case TokenType::NOT:
      out << "NOT";
      break;
    case TokenType::XOR:
      out << "XOR";
      break;
    case TokenType::TRUE:
      out << "TRUE";
      break;
    case TokenType::FALSE:
      out << "FALSE";
      break;
    default:
      out << "";
      break;
  }
  switch (token.type) {
    case TokenType::INT_LITERAL:
      out << ", " << token.num;
      break;
    case TokenType::FLOAT_LITERAL:
      out << ", " << token.fnum;
      break;
    case TokenType::STRING_LITERAL:
      out << ", \"" << token.str << "\"";
      break;
    case TokenType::IDENT:
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

TokenType ScannerNew::getTokenType(const std::string token) {
  if (token == "int") return TokenType::INT;
  if (token == "float") return TokenType::FLOAT;
  if (token == "bool") return TokenType::BOOL;
  if (token == "string") return TokenType::STRING;
  if (token == "tensor") return TokenType::TENSOR;
  if (token == "matrix") return TokenType::TENSOR;
  if (token == "vector") return TokenType::TENSOR;
  if (token == "element") return TokenType::ELEMENT;
  if (token == "set") return TokenType::SET;
  if (token == "var") return TokenType::VAR;
  if (token == "const") return TokenType::CONST;
  if (token == "extern") return TokenType::EXTERN;
  if (token == "proc") return TokenType::PROC;
  if (token == "func") return TokenType::FUNC;
  if (token == "inout") return TokenType::INOUT;
  if (token == "map") return TokenType::MAP;
  if (token == "to") return TokenType::TO;
  if (token == "with") return TokenType::WITH;
  if (token == "reduce") return TokenType::REDUCE;
  if (token == "while") return TokenType::WHILE;
  if (token == "do") return TokenType::DO;
  if (token == "if") return TokenType::IF;
  if (token == "elif") return TokenType::ELIF;
  if (token == "else") return TokenType::ELSE;
  if (token == "for") return TokenType::FOR;
  if (token == "in") return TokenType::IN;
  if (token == "end") return TokenType::BLOCKEND;
  if (token == "return") return TokenType::RETURN;
  if (token == "print") return TokenType::PRINT;
  if (token == "and") return TokenType::AND; 
  if (token == "or") return TokenType::OR;
  if (token == "not") return TokenType::NOT;
  if (token == "xor") return TokenType::XOR;
  if (token == "true") return TokenType::TRUE;
  if (token == "false") return TokenType::FALSE;
 
  // If string does not correspond to a keyword, assume it is an identifier.
  return TokenType::IDENT;
}

TokenStream ScannerNew::lex(std::istream &programStream) {
  TokenStream tokens;
  ScanState state = ScanState::INITIAL;
  unsigned line = 1;
  unsigned col = 1;

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
      if (newToken.type == TokenType::IDENT) {
        newToken.str = tokenString;
      }
      tokens.addToken(newToken);

      col += tokenString.length();
    } else {
      switch (programStream.peek()) {
        case '(':
          programStream.get();
          tokens.addToken(TokenType::LP, line, col++);
          break;
        case ')':
          programStream.get();
          tokens.addToken(TokenType::RP, line, col++);
          break;
        case '[':
          programStream.get();
          tokens.addToken(TokenType::LB, line, col++);
          break;
        case ']':
          programStream.get();
          tokens.addToken(TokenType::RB, line, col++);
          break;
        case '{':
          programStream.get();
          tokens.addToken(TokenType::LC, line, col++);
          break;
        case '}':
          programStream.get();
          tokens.addToken(TokenType::RC, line, col++);
          break;
        case '<':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::LE, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(TokenType::LA, line, col++);
          }
          break;
        case '>':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::GE, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(TokenType::RA, line, col++);
          }
          break;
        case ',':
          programStream.get();
          tokens.addToken(TokenType::COMMA, line, col++);
          break;
        case '.':
          programStream.get();
          switch (programStream.peek()) {
            case '*':
              programStream.get();
              tokens.addToken(TokenType::DOTSTAR, line, col, 2);
              col += 2;
              break;
            case '/':
              programStream.get();
              tokens.addToken(TokenType::DOTSLASH, line, col, 2);
              col += 2;
              break;
            default:
              tokens.addToken(TokenType::PERIOD, line, col++);
              break;
          }
          break;
        case ':':
          programStream.get();
          tokens.addToken(TokenType::COL, line, col++);
          break;
        case ';':
          programStream.get();
          tokens.addToken(TokenType::SEMICOL, line, col++);
          break;
        case '=':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::EQ, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(TokenType::ASSIGN, line, col++);
          }
          break;
        case '*':
          programStream.get();
          tokens.addToken(TokenType::STAR, line, col++);
          break;
        case '/':
          programStream.get();
          tokens.addToken(TokenType::SLASH, line, col++);
          break;
        case '\\':
          programStream.get();
          tokens.addToken(TokenType::BACKSLASH, line, col++);
          break;
        case '^':
          programStream.get();
          tokens.addToken(TokenType::EXP, line, col++);
          break;
        case '\'':
          programStream.get();
          tokens.addToken(TokenType::TRANSPOSE, line, col++);
          break;
        case '!':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::NE, line, col, 2);
            col += 2;
          } else {
            reportError("unexpected symbol \'!\'", line, col++);
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
              tokens.addToken(TokenType::TEST, line, col, 2);
              state = ScanState::SLTEST;
              col += 2;
              break;
            case '{':
              if (programStream.peek() == '!') {
                programStream.get();
                tokens.addToken(TokenType::TEST, line, col, 2);
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
                reportError("could not find corresponding \'!%{\'", line, col);
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
          tokens.addToken(TokenType::PLUS, line, col++);
          break;
        case '-': 
          programStream.get();
          if (programStream.peek() == '>') {
            programStream.get();
            tokens.addToken(TokenType::RARROW, line, col, 2);
            col += 2;
          } else {
            tokens.addToken(TokenType::MINUS, line, col++);
          }
          break;
        default: 
        {
          Token newToken;
          newToken.type = TokenType::INT_LITERAL;
          newToken.lineBegin = line;
          newToken.colBegin = col;

          if (programStream.peek() != '.' && 
              !std::isdigit(programStream.peek())) {
            std::stringstream errMsg;
            errMsg << "unexpected symbol \'" 
                   << (char)programStream.peek() << "\'";
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
            newToken.type = TokenType::FLOAT_LITERAL;
            tokenString += programStream.get();
            ++col;

            if (!std::isdigit(programStream.peek())) {
              std::stringstream errMsg;
              errMsg << "unexpected symbol \'" 
                     << (char)programStream.peek() << "\'";
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
            newToken.type = TokenType::FLOAT_LITERAL;
            tokenString += programStream.get();
            ++col;

            if (programStream.peek() == '+' || programStream.peek() == '-') {
              tokenString += programStream.get();
              ++col;
            }

            if (!std::isdigit(programStream.peek())) {
              std::stringstream errMsg;
              errMsg << "unexpected symbol \'" 
                     << (char)programStream.peek() << "\'";
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
          if (newToken.type == TokenType::INT_LITERAL) {
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

  tokens.addToken(TokenType::END, line, col);
  return tokens;
}

}
}

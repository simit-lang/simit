#include <cctype>
#include <string>
#include <cstdlib>
#include <iostream>

#include "scanner.h"
#include "error.h"

namespace simit { 
namespace internal {

Token::Type Scanner::getTokenType(const std::string token) {
  if (token == "int") return Token::Type::INT;
  if (token == "float") return Token::Type::FLOAT;
  if (token == "bool") return Token::Type::BOOL;
  if (token == "complex") return Token::Type::COMPLEX;
  if (token == "string") return Token::Type::STRING;
  if (token == "tensor") return Token::Type::TENSOR;
  if (token == "matrix") return Token::Type::MATRIX;
  if (token == "vector") return Token::Type::VECTOR;
  if (token == "element") return Token::Type::ELEMENT;
  if (token == "set") return Token::Type::SET;
  if (token == "lattice") return Token::Type::LATTICE;
  if (token == "opaque") return Token::Type::OPAQUE;
  if (token == "var") return Token::Type::VAR;
  if (token == "const") return Token::Type::CONST;
  if (token == "extern") return Token::Type::EXTERN;
  if (token == "export") return Token::Type::EXPORT;
  if (token == "proc") return Token::Type::PROC;
  if (token == "func") return Token::Type::FUNC;
  if (token == "inout") return Token::Type::INOUT;
  if (token == "apply") return Token::Type::APPLY;
  if (token == "map") return Token::Type::MAP;
  if (token == "to") return Token::Type::TO;
  if (token == "with") return Token::Type::WITH;
  if (token == "reduce") return Token::Type::REDUCE;
  if (token == "through") return Token::Type::THROUGH;
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
  if (token == "println") return Token::Type::PRINTLN;
  if (token == "and") return Token::Type::AND; 
  if (token == "or") return Token::Type::OR;
  if (token == "not") return Token::Type::NOT;
  if (token == "xor") return Token::Type::XOR;
  if (token == "true") return Token::Type::TRUE;
  if (token == "false") return Token::Type::FALSE;
 
  // If string does not correspond to a keyword, assume it is an identifier.
  return Token::Type::IDENT;
}

TokenStream Scanner::lex(std::istream &programStream) {
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
                ++col;

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
        case '"':
        {
          Token newToken;
          newToken.type = Token::Type::STRING_LITERAL;
          newToken.lineBegin = line;
          newToken.colBegin = col;
          
          programStream.get();
          ++col;

          while (programStream.peek() != EOF && programStream.peek() != '"') {
            if (programStream.peek() == '\\') {
              programStream.get();

              std::string escapedChar = "";
              switch (programStream.peek()) {
                case 'a':
                  escapedChar = "\a";
                  break;
                case 'b':
                  escapedChar = "\b";
                  break;
                case 'f':
                  escapedChar = "\f";
                  break;
                case 'n':
                  escapedChar = "\n";
                  break;
                case 'r':
                  escapedChar = "\r";
                  break;
                case 't':
                  escapedChar = "\t";
                  break;
                case 'v':
                  escapedChar = "\v";
                  break;
                case '\\':
                  escapedChar = "\\";
                  break;
                case '\'':
                  escapedChar = "\'";
                  break;
                case '"':
                  escapedChar = "\"";
                  break;
                case '?':
                  escapedChar = "\?";
                  break;
                default:
                  reportError("unrecognized escape sequence", line, col);
                  ++col;
                  break;
              }

              if (escapedChar != "") {
                newToken.str += escapedChar;
                programStream.get();
                col += 2;
              }
            } else {
              newToken.str += programStream.get();
              ++col;
            }
          }
          
          newToken.lineEnd = line;
          newToken.colEnd = col;
          tokens.addToken(newToken);

          if (programStream.peek() == '"') {
            programStream.get();
            ++col;
          } else {
            reportError("unclosed string literal", line, col);
          }
          break;
        }
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

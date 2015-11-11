#include <cctype>
#include <string>
#include <cstdlib>

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
    case TokenType::UNTIL:
      out << "UNTIL";
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
  out << ")";
  return out;
}

std::ostream &operator <<(std::ostream &out, TokenList tokens) {
  for (std::list<Token>::const_iterator it = tokens.tokens.cbegin();
      it != tokens.tokens.cend(); ++it) {
    out << *it << std::endl;
  }
  return out;
}

TokenType ScannerNew::getTokenType(std::string token) {
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
  if (token == "until") return TokenType::UNTIL;
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

TokenList ScannerNew::lex(std::istream &programStream) {
  TokenList tokens;
  ScanState state = ScanState::INITIAL;
  unsigned int lineNum = 1;
  unsigned int colNum = 1;

  while (programStream.peek() != EOF) {
    if (programStream.peek() == '_' || std::isalpha(programStream.peek())) {
      std::string tokenString(1, programStream.get());

      while (programStream.peek() == '_' || 
          std::isalnum(programStream.peek())) {
        tokenString += programStream.get();
      }

      Token newToken;
      newToken.type = getTokenType(tokenString);
      if (newToken.type == TokenType::IDENT) {
        newToken.str = tokenString;
      }
      tokens.addToken(newToken);

      colNum += tokenString.length();
    } else {
      switch (programStream.peek()) {
        case '(':
          programStream.get();
          tokens.addToken(TokenType::LP);
          break;
        case ')':
          programStream.get();
          tokens.addToken(TokenType::RP);
          break;
        case '[':
          programStream.get();
          tokens.addToken(TokenType::LB);
          break;
        case ']':
          programStream.get();
          tokens.addToken(TokenType::RB);
          break;
        case '{':
          programStream.get();
          tokens.addToken(TokenType::LC);
          break;
        case '}':
          programStream.get();
          tokens.addToken(TokenType::RC);
          break;
        case '<':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::LE);
          } else {
            tokens.addToken(TokenType::LA);
          }
          break;
        case '>':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::GE);
          } else {
            tokens.addToken(TokenType::RA);
          }
          break;
        case ',':
          programStream.get();
          tokens.addToken(TokenType::COMMA);
          break;
        case '.':
          programStream.get();
          switch (programStream.peek()) {
            case '*':
              programStream.get();
              tokens.addToken(TokenType::DOTSTAR);
              break;
            case '/':
              programStream.get();
              tokens.addToken(TokenType::DOTSLASH);
              break;
            default:
              tokens.addToken(TokenType::PERIOD);
              break;
          }
          break;
        case ':':
          programStream.get();
          tokens.addToken(TokenType::COL);
          break;
        case ';':
          programStream.get();
          tokens.addToken(TokenType::SEMICOL);
          break;
        case '=':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::EQ);
          } else {
            tokens.addToken(TokenType::ASSIGN);
          }
          break;
        case '*':
          programStream.get();
          tokens.addToken(TokenType::STAR);
          break;
        case '/':
          programStream.get();
          tokens.addToken(TokenType::SLASH);
          break;
        case '\\':
          programStream.get();
          tokens.addToken(TokenType::BACKSLASH);
          break;
        case '^':
          programStream.get();
          tokens.addToken(TokenType::EXP);
          break;
        case '\'':
          programStream.get();
          tokens.addToken(TokenType::TRANSPOSE);
          break;
        case '!':
          programStream.get();
          if (programStream.peek() == '=') {
            programStream.get();
            tokens.addToken(TokenType::NE);
          } else {
            // TODO: raise error
          }
          break;
        case '%':
          programStream.get();
          switch (programStream.peek()) {
            case '!':
              programStream.get();
              tokens.addToken(TokenType::TEST);
              state = ScanState::SLTEST;
              break;
            case '{':
              if (programStream.peek() == '!') {
                programStream.get();
                tokens.addToken(TokenType::TEST);
                state = ScanState::MLTEST;
              } else {
                std::string comment;

                while (programStream.peek() != EOF) {
                  if (programStream.peek() == '%') {
                    programStream.get();
                    if (programStream.peek() == '}') {
                      // TODO: emit COMMENT token
                      break;
                    } else {
                      comment += '%';
                      comment += programStream.get();
                    }
                  } else {
                    comment += programStream.get();
                  }
                }

                if (programStream.peek() == EOF) {
                  // TODO: raise error
                }
              }
              break;
            case '}':
              if (state == ScanState::MLTEST) {
                programStream.get();
                state = ScanState::INITIAL;
              } else {
                // TODO: raise error
              }
              break;
            default:
            {
              std::string comment;
              while (programStream.peek() != '\n' && 
                  programStream.peek() != EOF) {
                comment += programStream.get();
              }
              // TODO: emit COMMENT token
              break;
            }
          }
          break;
        case '\n':
          ++lineNum;
          colNum = 0;
        case ' ':
        case '\t':
          programStream.get();
          ++colNum;
          break;
#if 0
        case '+':
        case '-': 
        default: 
        {
          if (programStream.peek() != '+' && programStream.peek() != '-' && 
              programStream.peek() != '.' && 
              !std::isdigit(programStream.peek())) {
            // TODO: raise error
            programStream.get();
            break;
          }

          std::string tokenString;

          if (programStream.peek() == '+' || programStream.peek() == '-') {
            const char ch = programStream.get();
            
            if (ch == '+' && !std::isdigit(programStream.peek()) && 
                programStream.peek() != '.') {
              tokens.addToken(TokenType::PLUS);
              break;
            } else if (ch == '-' && !std::isdigit(programStream.peek()) &&
                programStream.peek() != '.') {
              if (programStream.peek() == '>') {
                programStream.get();
                tokens.addToken(TokenType::RARROW);
              } else {
                tokens.addToken(TokenType::MINUS);
              }
              break;
            }

            tokenString += ch;
          }
#endif
        case '+':
          programStream.get();
          tokens.addToken(TokenType::PLUS);
          break;
        case '-': 
          programStream.get();
          if (programStream.peek() == '>') {
            programStream.get();
            tokens.addToken(TokenType::RARROW);
          } else {
            tokens.addToken(TokenType::MINUS);
          }
          break;
        default: 
        {
          if (programStream.peek() != '.' && 
              !std::isdigit(programStream.peek())) {
            // TODO: raise error
            programStream.get();
            break;
          }

          std::string tokenString;
          bool isInt = true;
          
          //iassert (std::isdigit(programStream.peek()) || 
          //  programStream.peek() == '.');
          while (std::isdigit(programStream.peek())) {
            tokenString += programStream.get();
          }

          if (programStream.peek() == '.') {
            isInt = false;
            tokenString += programStream.get();

            if (!std::isdigit(programStream.peek())) {
              // TODO: raise error
              break;
            }
            tokenString += programStream.get();

            while (std::isdigit(programStream.peek())) {
              tokenString += programStream.get();
            }
          }

          if (programStream.peek() == 'e' || programStream.peek() == 'E') {
            isInt = false;
            tokenString += programStream.get();

            if (programStream.peek() == '+' || programStream.peek() == '-') {
              tokenString += programStream.get();
            }

            if (!std::isdigit(programStream.peek())) {
              // TODO: raise error
              break;
            }
            tokenString += programStream.get();

            while (std::isdigit(programStream.peek())) {
              tokenString += programStream.get();
            }
          }

          char *end;
          if (isInt) {
            int num = std::strtol(tokenString.c_str(), &end, 0);
            // iassert(end == nullptr);
            tokens.addToken(Token(num));
          } else {
            double fnum = std::strtod(tokenString.c_str(), &end);
            // iassert(end == nullptr);
            tokens.addToken(Token(fnum));
          }

          break;
        }
      }
    }
  }

  if (state != ScanState::INITIAL) {
    // TODO: raise error
  }

  tokens.addToken(Token(TokenType::END));
  return tokens;
}

}
}

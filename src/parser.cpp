#include "parser.h"
#include "scanner.h"

namespace simit { 
namespace internal {

#if 1
void ParserNew::parse(const TokenList &tokens, ProgramContext *ctx, 
                      std::vector<ParseError> *errors) {
  this->tokens = tokens;
  this->ctx = ctx;
  this->errors = errors;
  parseProgram();
}

void ParserNew::parseProgram() {
  while (peek().type != TokenType::END) {
    parseProgramElement();
  }
}

void ParserNew::parseProgramElement() {
  switch (peek().type) {
    case TokenType::TEST:
      parseTest();
      break;
    case TokenType::FUNC:
      parseFunction();
      break;
    case TokenType::PROC:
      parseProcedure();
      break;
    case TokenType::ELEMENT:
      parseElementTypeDecl();
      break;
    case TokenType::EXTERN:
      parseExternDecl();
      break;
    case TokenType::CONST:
      parseConstDecl();
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseElementTypeDecl() {
  consume(TokenType::ELEMENT);
  consume(TokenType::IDENT);
  while (peek().type == TokenType::IDENT) {
    parseFieldDecl();
  }
  consume(TokenType::BLOCKEND);
}

void ParserNew::parseFieldDecl() {
  consume(TokenType::IDENT);
  consume(TokenType::COL);
  parseTensorType();
  consume(TokenType::SEMICOL);
}

void ParserNew::parseExternDecl() {
  consume(TokenType::EXTERN);
  parseArgumentDecl();
  consume(TokenType::SEMICOL);
}

void ParserNew::parseFunction() {
  consume(TokenType::FUNC);
  consume(TokenType::IDENT);
  parseArgsAndResults();
  parseStmtBlock();
  consume(TokenType::BLOCKEND);
}

void ParserNew::parseProcedure() {
  consume(TokenType::PROC);
  consume(TokenType::IDENT);
  if (peek().type == TokenType::LP) {
    switch (peek(1).type) {
      case TokenType::VAR:
      case TokenType::INOUT:
        parseArgsAndResults();
        break;
      default:
        break;
    }
  }
  parseStmtBlock();
  consume(TokenType::BLOCKEND);
}

void ParserNew::parseArgsAndResults() {
  consume(TokenType::LP);
  parseArguments();
  consume(TokenType::RP);
  parseResults();
}

void ParserNew::parseArguments() {
  parseArgumentDecl();
  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    parseArgumentDecl();
  }
}

void ParserNew::parseArgumentDecl() {
  if (peek().type == TokenType::INOUT) {
    consume(TokenType::INOUT);
  }
  parseIdentDecl();
}

void ParserNew::parseResults() {
  if (peek().type == TokenType::RARROW) {
    consume(TokenType::RARROW);
    consume(TokenType::LP);
    parseResultList();
    consume(TokenType::RP);
  }
}

void ParserNew::parseResultList() {
  parseIdentDecl();
  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    parseIdentDecl();
  }
}

void ParserNew::parseStmtBlock() {
  while (true) {
    switch (peek().type) {
      case TokenType::BLOCKEND:
      case TokenType::UNTIL:
      case TokenType::ELIF:
      case TokenType::ELSE:
      case TokenType::END:
        return;
      default:
        parseStmt();
        break;
    }
  }
}

void ParserNew::parseStmt() {
  switch (peek().type) {
    case TokenType::VAR:
      parseVarDecl();
      break;
    case TokenType::CONST:
      parseConstDecl();
      break;
    case TokenType::IF:
      parseIfStmt();
      break;
    case TokenType::WHILE:
      parseWhileStmt();
      break;
    case TokenType::DO:
      parseDoUntilStmt();
      break;
    case TokenType::FOR:
      parseForStmt();
      break;
    case TokenType::PRINT:
      parsePrintStmt();
      break;
    default:
      parseExprOrAssignStmt();
      break;
  }
}

void ParserNew::parseVarDecl() {
  consume(TokenType::VAR);
  parseIdentDecl();
  if (peek().type == TokenType::ASSIGN) {
    consume(TokenType::ASSIGN);
    parseExpr();
  }
  consume(TokenType::SEMICOL);
}

void ParserNew::parseConstDecl() {
  consume(TokenType::CONST);
  parseIdentDecl();
  consume(TokenType::ASSIGN);
  parseExpr();
  consume(TokenType::SEMICOL);
}

void ParserNew::parseIdentDecl() {
  consume(TokenType::IDENT);
  consume(TokenType::COL);
  parseType();
}

void ParserNew::parseWhileStmt() {
  consume(TokenType::WHILE);
  parseOrExpr();
  parseStmtBlock();
  consume(TokenType::BLOCKEND);
}

void ParserNew::parseDoUntilStmt() {
  consume(TokenType::DO);
  parseStmtBlock();
  consume(TokenType::UNTIL);
  parseOrExpr();
}

void ParserNew::parseIfStmt() {
  consume(TokenType::IF);
  parseOrExpr();
  parseStmtBlock();
  parseElseClause();
  consume(TokenType::BLOCKEND);
}

void ParserNew::parseElseClause() {
  switch (peek().type) {
    case TokenType::ELSE:
      consume(TokenType::ELSE);
      parseStmtBlock();
      return;
    case TokenType::ELIF:
      consume(TokenType::ELIF);
      parseOrExpr();
      parseStmtBlock();
      parseElseClause();
      return;
    default:
      break;
  }
  // TODO: return Pass
}

void ParserNew::parseForStmt() {
  consume(TokenType::FOR);
  consume(TokenType::IDENT);
  consume(TokenType::IN);
  parseForStmtRange();
  parseStmtBlock();
  consume(TokenType::BLOCKEND);
}

void ParserNew::parseForStmtRange() {
  // index_set
  switch (peek().type) {
    case TokenType::INT_LITERAL:
    case TokenType::IDENT:
      if (peek(1).type == TokenType::COL) {
        break;
      }
    case TokenType::STAR:
      parseIndexSet();
      return;
    default:
      break;
  }

  parseExpr();
  consume(TokenType::COL);
  parseExpr();
  // TODO: check range is valid?
}

void ParserNew::parsePrintStmt() {
  consume(TokenType::PRINT);
  parseExpr();
}

void ParserNew::parseExprOrAssignStmt() {
  if (peek().type != TokenType::SEMICOL) {
    parseExpr();
    switch (peek().type) {
      case TokenType::COMMA:
      case TokenType::ASSIGN:
        while (peek().type == TokenType::COMMA) {
          consume(TokenType::COMMA);
          parseExpr();
        }
        consume(TokenType::ASSIGN);
        parseExpr();
        break;
      default:
        break;
    }
    // TODO: handle map statement
  }
  consume(TokenType::SEMICOL);
}

void ParserNew::parseExpr() {
  if (peek().type == TokenType::MAP) {
    parseMapExpr();
    return;
  }
  parseOrExpr();
}

void ParserNew::parseMapExpr() {
  consume(TokenType::MAP);
  consume(TokenType::IDENT);
  if (peek().type == TokenType::LP) {
    parseOptionalExprList(); 
  }
  consume(TokenType::TO);
  consume(TokenType::IDENT);
  if (peek().type == TokenType::REDUCE) {
    consume(TokenType::REDUCE);
    consume(TokenType::PLUS);
  }
}

void ParserNew::parseOrExpr() {
  parseAndExpr();
  while (peek().type == TokenType::OR) {
    consume(TokenType::OR);
    parseAndExpr();
  }
}

void ParserNew::parseAndExpr() {
  parseXorExpr();
  while (peek().type == TokenType::AND) {
    consume(TokenType::AND);
    parseXorExpr();
  }
}

void ParserNew::parseXorExpr() {
  parseEqExpr();
  while (peek().type == TokenType::XOR) {
    consume(TokenType::XOR);
    parseEqExpr();
  }
}

void ParserNew::parseEqExpr() {
  parseIneqExpr();
  switch (peek().type) {
    case TokenType::EQ:
      consume(TokenType::EQ);
      parseIneqExpr();
      break;
    case TokenType::NE:
      consume(TokenType::NE);
      parseIneqExpr();
      break;
    default:
      break;
  }
}

void ParserNew::parseIneqExpr() {
  parseBooleanFactor();
  switch (peek().type) {
    case TokenType::RA:
      consume(TokenType::RA);
      parseBooleanFactor();
      break;
    case TokenType::LA:
      consume(TokenType::LA);
      parseBooleanFactor();
      break;
    case TokenType::LE:
      consume(TokenType::LE);
      parseBooleanFactor();
      break;
    case TokenType::GE:
      consume(TokenType::GE);
      parseBooleanFactor();
      break;
    default:
      break;
  }
}

void ParserNew::parseBooleanFactor() {
  if (peek().type == TokenType::NOT) {
    consume(TokenType::NOT);
    parseBooleanFactor();
    return;
  }
  parseSolveExpr();
}

void ParserNew::parseSolveExpr() {
  parseAddExpr();
  while (peek().type == TokenType::BACKSLASH) {
    consume(TokenType::BACKSLASH);
    parseAddExpr();
  }
}

void ParserNew::parseAddExpr() {
  parseMulExpr();
  while (true) {
    switch (peek().type) {
      case TokenType::PLUS:
        consume(TokenType::PLUS);
        parseMulExpr();
        break;
      case TokenType::MINUS:
        consume(TokenType::MINUS);
        parseMulExpr();
        break;
      default:
        return;
    }
  }
}

void ParserNew::parseMulExpr() {
  parseNegExpr();
  while (true) {
    switch (peek().type) {
      case TokenType::STAR:
        consume(TokenType::STAR);
        parseNegExpr();
        break;
      case TokenType::SLASH:
        consume(TokenType::SLASH);
        parseNegExpr();
        break;
      case TokenType::DOTSTAR:
        consume(TokenType::DOTSTAR);
        parseNegExpr();
        break;
      case TokenType::DOTSLASH:
        consume(TokenType::DOTSLASH);
        parseNegExpr();
        break;
      default:
        return;
    }
  }
}

void ParserNew::parseNegExpr() {
  while (true) {
    switch (peek().type) {
      case TokenType::MINUS:
        consume(TokenType::MINUS);
        break;
      case TokenType::PLUS:
        consume(TokenType::PLUS);
        break;
      default:
        parseExpExpr();
        return;
    }
  }
}

void ParserNew::parseExpExpr() {
  parseTransposeExpr();
  while (peek().type == TokenType::EXP) {
    consume(TokenType::EXP);
    parseExpExpr();
  }
}

void ParserNew::parseTransposeExpr() {
  parseCallOrReadExpr();
  if (peek().type == TokenType::TRANSPOSE) {
    consume(TokenType::TRANSPOSE);
  }
}

void ParserNew::parseCallOrReadExpr() {
  parseFactor();
  while (true) {
    switch (peek().type) {
      case TokenType::LP:
        parseOptionalExprList();
        break;
      case TokenType::PERIOD:
        consume(TokenType::PERIOD);
        consume(TokenType::IDENT);
        break;
      default:
        return;
    }
  }
}

void ParserNew::parseFactor() {
  switch (peek().type) {
    case TokenType::LP:
      consume(TokenType::LP);
      parseOrExpr();
      consume(TokenType::RP);
      break;
    case TokenType::IDENT:
      consume(TokenType::IDENT);
      break;
    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::STRING_LITERAL:
    case TokenType::TRUE:
    case TokenType::FALSE:
    case TokenType::LB:
      parseTensorLiteral();
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseNonemptyExprList() {
  consume(TokenType::LP);
  parseExprList();
  consume(TokenType::RP);
}

void ParserNew::parseOptionalExprList() {
  if (peek(1).type == TokenType::RP) {
    consume(TokenType::LP);
    consume(TokenType::RP);
    return;
  }
  parseNonemptyExprList();
}

void ParserNew::parseExprList() {
  parseExprListElement();
  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    parseExprListElement();
  }
}

void ParserNew::parseExprListElement() {
  if (peek().type == TokenType::COL) {
    consume(TokenType::COL);
    return;
  }
  parseOrExpr();
}

void ParserNew::parseType() {
  switch (peek().type) {
    case TokenType::IDENT:
      parseElementType();
      break;
    case TokenType::SET:
      parseSetType();
      break;
    case TokenType::LP:
      parseTupleType();
      break;
    case TokenType::INT:
    case TokenType::FLOAT:
    case TokenType::BOOL:
    case TokenType::STRING:
    case TokenType::TENSOR:
      parseTensorType();
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseElementType() {
  consume(TokenType::IDENT);
}

void ParserNew::parseSetType() {
  consume(TokenType::SET);
  consume(TokenType::LC);
  parseElementType();
  consume(TokenType::RC);
  if (peek().type == TokenType::LP) {
    consume(TokenType::LP);
    parseEndpoints();
    consume(TokenType::RP);
  }
}

void ParserNew::parseEndpoints() {
  consume(TokenType::IDENT);
  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    consume(TokenType::IDENT);
  }
}

void ParserNew::parseTupleType() {
  consume(TokenType::LP);
  parseElementType();
  consume(TokenType::STAR);
  consume(TokenType::INT_LITERAL);
  consume(TokenType::RP);
}

void ParserNew::parseTensorType() {
  parseTensorTypeStart();
  if (peek().type == TokenType::TRANSPOSE) {
    consume(TokenType::TRANSPOSE);
  }
}

void ParserNew::parseTensorTypeStart() {
  switch (peek().type) {
    case TokenType::INT:
      consume(TokenType::INT);
      break;
    case TokenType::FLOAT:
      consume(TokenType::FLOAT);
      break;
    case TokenType::BOOL:
      consume(TokenType::BOOL);
      break;
    case TokenType::STRING:
      consume(TokenType::STRING);
      break;
    case TokenType::TENSOR:
      consume(TokenType::TENSOR);
      if (peek().type == TokenType::LB) {
        consume(TokenType::LB);
        parseIndexSets();
        consume(TokenType::RB);
      }
      consume(TokenType::LP);
      parseTensorType();
      consume(TokenType::RP);
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseIndexSets() {
  parseIndexSet();
  while (peek().type == TokenType::COMMA) {
    consume(TokenType::COMMA);
    parseIndexSet();
  }
}

void ParserNew::parseIndexSet() {
  switch (peek().type) {
    case TokenType::INT_LITERAL:
      consume(TokenType::INT_LITERAL);
      break;
    case TokenType::IDENT:
      consume(TokenType::IDENT);
      break;
    case TokenType::STAR:
      consume(TokenType::STAR);
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseTensorLiteral() {
  switch (peek().type) {
    case TokenType::INT_LITERAL:
      consume(TokenType::INT_LITERAL);
      break;
    case TokenType::FLOAT_LITERAL:
      consume(TokenType::FLOAT_LITERAL);
      break;
    case TokenType::STRING_LITERAL:
      consume(TokenType::STRING_LITERAL);
      break;
    case TokenType::TRUE:
      consume(TokenType::TRUE);
      break;
    case TokenType::FALSE:
      consume(TokenType::FALSE);
      break;
    case TokenType::LB:
      parseDenseTensorLiteral();
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseDenseTensorLiteral() {
  consume(TokenType::LB);
  parseDenseTensorLiteralInner();
  consume(TokenType::RB);
  if (peek().type == TokenType::TRANSPOSE) {
    consume(TokenType::TRANSPOSE);
  }
}

void ParserNew::parseDenseTensorLiteralInner() {
  if (peek().type == TokenType::LB) {
    parseDenseTensorLiteral();
    while (true) {
      switch (peek().type) {
        case TokenType::COMMA:
          consume(TokenType::COMMA);
        case TokenType::LB:
          parseDenseTensorLiteral();
          break;
        default:
          return;
      }
    }
  }
  parseDenseMatrixLiteral();
}

void ParserNew::parseDenseMatrixLiteral() {
  parseDenseVectorLiteral();
  while (peek().type == TokenType::SEMICOL) {
    consume(TokenType::SEMICOL);
    parseDenseVectorLiteral();
  }
}

void ParserNew::parseDenseVectorLiteral() {
  switch (peek().type) {
    case TokenType::INT_LITERAL:
      parseDenseIntVectorLiteral();
      break;
    case TokenType::FLOAT_LITERAL:
      parseDenseFloatVectorLiteral();
      break;
    case TokenType::PLUS:
    case TokenType::MINUS:
      switch (peek(1).type) {
        case TokenType::INT_LITERAL:
          parseDenseIntVectorLiteral();
          break;
        case TokenType::FLOAT_LITERAL:
          parseDenseFloatVectorLiteral();
          break;
        default:
          throw ParseException();
          break;
      }
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseDenseIntVectorLiteral() {
  parseSignedIntLiteral();
  while (true) {
    switch (peek().type) {
      case TokenType::COMMA:
        consume(TokenType::COMMA);
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::INT_LITERAL:
        parseSignedIntLiteral();
        break;
      default:
        return;
    }
  }
}

void ParserNew::parseDenseFloatVectorLiteral() {
  parseSignedFloatLiteral();
  while (true) {
    switch (peek().type) {
      case TokenType::COMMA:
        consume(TokenType::COMMA);
      case TokenType::PLUS:
      case TokenType::MINUS:
      case TokenType::FLOAT_LITERAL:
        parseSignedFloatLiteral();
        break;
      default:
        return;
    }
  }
}

void ParserNew::parseSignedIntLiteral() {
  switch (peek().type) {
    case TokenType::PLUS:
      consume(TokenType::PLUS);
      break;
    case TokenType::MINUS:
      consume(TokenType::MINUS);
      break;
    default:
      break;
  }
  consume(TokenType::INT_LITERAL);
}

void ParserNew::parseSignedFloatLiteral() {
  switch (peek().type) {
    case TokenType::PLUS:
      consume(TokenType::PLUS);
      break;
    case TokenType::MINUS:
      consume(TokenType::MINUS);
      break;
    default:
      break;
  }
  consume(TokenType::FLOAT_LITERAL);
}

void ParserNew::parseTest() {
  consume(TokenType::TEST);
  consume(TokenType::IDENT);
  switch (peek().type) {
    case TokenType::LP:
      parseOptionalExprList();
      consume(TokenType::EQ);
      parseTensorLiteral();
      consume(TokenType::SEMICOL);
      break;
    case TokenType::ASSIGN:
      consume(TokenType::ASSIGN);
      parseSystemGenerator();
      consume(TokenType::COL);
      parseExternAssert();
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseSystemGenerator() {
  switch (peek().type) {
    case TokenType::IDENT:
      consume(TokenType::IDENT);
      break;
    case TokenType::INT_LITERAL:
      consume(TokenType::INT_LITERAL);
      break;
    default:
      throw ParseException();
      break;
  }
}

void ParserNew::parseExternAssert() {
  consume(TokenType::IDENT);
  consume(TokenType::PERIOD);
  consume(TokenType::IDENT);
  consume(TokenType::ASSIGN);
  parseTensorLiteral();
  consume(TokenType::RARROW);
  parseTensorLiteral();
}
#endif

}
}


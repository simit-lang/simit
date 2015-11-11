#ifndef PARSER_NEW_H
#define PARSER_NEW_H

#include <exception>
#include <vector>

#include "scanner.h"
#include "program_context.h"
#include "error.h"

namespace simit { 
namespace internal {

class ParseException : public std::exception {};

class ParserNew {
public:
  void parse(const TokenList &, ProgramContext *, std::vector<ParseError> *);

private:
  void parseProgram();
  void parseProgramElement();
  void parseElementTypeDecl();
  void parseFieldDecl();
  void parseExternDecl();
  void parseFunction();
  void parseProcedure();
  void parseArgsAndResults();
  void parseArguments();
  void parseArgumentDecl();
  void parseResults();
  void parseResultList();
  void parseStmtBlock();
  void parseStmt();
  void parseVarDecl();
  void parseConstDecl();
  void parseIdentDecl();
  void parseWhileStmt();
  void parseDoUntilStmt();
  void parseIfStmt();
  void parseElseClause();
  void parseForStmt();
  void parseForStmtRange();
  void parsePrintStmt();
  void parseExprOrAssignStmt();
  void parseExpr();
  void parseMapExpr();
  void parseOrExpr();
  void parseAndExpr();
  void parseXorExpr();
  void parseEqExpr();
  void parseIneqExpr();
  void parseBooleanFactor();
  void parseSolveExpr();
  void parseAddExpr();
  void parseMulExpr();
  void parseNegExpr();
  void parseExpExpr();
  void parseTransposeExpr();
  void parseCallOrReadExpr();
  void parseFactor();
  void parseNonemptyExprList();
  void parseOptionalExprList();
  void parseExprList();
  void parseExprListElement();
  void parseType();
  void parseElementType();
  void parseSetType();
  void parseEndpoints();
  void parseTupleType();
  void parseTensorType();
  void parseTensorTypeStart();
  void parseIndexSets();
  void parseIndexSet();
  void parseTensorLiteral();
  void parseDenseTensorLiteral();
  void parseDenseTensorLiteralInner();
  void parseDenseMatrixLiteral();
  void parseDenseVectorLiteral();
  void parseDenseIntVectorLiteral();
  void parseDenseFloatVectorLiteral();
  void parseSignedIntLiteral();
  void parseSignedFloatLiteral();
  void parseTest();
  void parseSystemGenerator();
  void parseExternAssert();

  inline Token consume(TokenType type) { 
    Token token = peek();
    std::cout << token << " " << Token(type) << std::endl;
    if (!tokens.consume(type)) {
      throw ParseException();
    }
    return token;
  }
  inline Token peek(unsigned int k = 0) { return tokens.peek(k); }

  TokenList tokens;
  ProgramContext *ctx;
  std::vector<ParseError> *errors;
};

}
}

#endif


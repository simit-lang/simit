#ifndef PARSER_NEW_H
#define PARSER_NEW_H

#include <exception>
#include <vector>
#include <utility>

#include "scanner.h"
#include "hir.h"
#include "error.h"

namespace simit { 
namespace internal {

class ParserNew {
public:
  hir::Program::Ptr parse(const TokenStream &, std::vector<ParseError> *);

private:
  class SyntaxError : public std::exception {};
  
  hir::Program::Ptr parseProgram();
  hir::HIRNode::Ptr parseProgramElement();
  hir::ElementTypeDecl::Ptr parseElementTypeDecl();
  std::vector<hir::Field::Ptr> parseFieldDeclList();
  hir::Field::Ptr parseFieldDecl();
  hir::ExternDecl::Ptr parseExternDecl();
  hir::FuncDecl::Ptr parseFunction();
  hir::ProcDecl::Ptr parseProcedure();
  std::vector<hir::Argument::Ptr> parseArguments();
  hir::Argument::Ptr parseArgumentDecl();
  std::vector<hir::IdentDecl::Ptr> parseResults();
  hir::StmtBlock::Ptr parseStmtBlock();
  hir::Stmt::Ptr parseStmt();
  hir::VarDecl::Ptr parseVarDecl();
  hir::ConstDecl::Ptr parseConstDecl();
  hir::IdentDecl::Ptr parseIdentDecl();
  hir::WhileStmt::Ptr parseWhileStmt();
  hir::DoWhileStmt::Ptr parseDoWhileStmt();
  hir::IfStmt::Ptr parseIfStmt();
  hir::Stmt::Ptr parseElseClause();
  hir::ForStmt::Ptr parseForStmt();
  hir::ForDomain::Ptr parseForDomain();
  hir::PrintStmt::Ptr parsePrintStmt();
  hir::ExprStmt::Ptr parseExprOrAssignStmt();
  hir::Expr::Ptr parseExpr();
  hir::Expr::Ptr parseMapExpr();
  hir::Expr::Ptr parseOrExpr();
  hir::Expr::Ptr parseAndExpr();
  hir::Expr::Ptr parseXorExpr();
  hir::Expr::Ptr parseEqExpr();
  hir::Expr::Ptr parseTerm();
  hir::Expr::Ptr parseSolveExpr();
  hir::Expr::Ptr parseAddExpr();
  hir::Expr::Ptr parseMulExpr();
  hir::Expr::Ptr parseNegExpr();
  hir::Expr::Ptr parseExpExpr();
  hir::Expr::Ptr parseTransposeExpr();
  hir::Expr::Ptr parseCallOrReadExpr();
  hir::Expr::Ptr parseFactor();
  std::vector<hir::ReadParam::Ptr> parseReadParams();
  hir::ReadParam::Ptr parseReadParam();
  std::vector<hir::Expr::Ptr> parseCallParams();
  hir::Type::Ptr parseType();
  hir::ElementType::Ptr parseElementType();
  hir::SetType::Ptr parseSetType();
  std::vector<hir::Endpoint::Ptr> parseEndpoints();
  hir::TupleType::Ptr parseTupleType();
  hir::TensorType::Ptr parseTensorType();
  std::vector<hir::IndexSet::Ptr> parseIndexSets();
  hir::IndexSet::Ptr parseIndexSet();
  hir::Expr::Ptr parseTensorLiteral();
  hir::TensorLiteral::Ptr parseSignedNumLiteral();
  hir::DenseTensorLiteral::Ptr parseDenseTensorLiteral();
  hir::DenseTensorLiteral::Ptr parseDenseTensorLiteralInner();
  hir::DenseTensorLiteral::Ptr parseDenseMatrixLiteral();
  hir::DenseTensorLiteral::Ptr parseDenseVectorLiteral();
  hir::DenseIntVectorLiteral::Ptr parseDenseIntVectorLiteral();
  hir::DenseFloatVectorLiteral::Ptr parseDenseFloatVectorLiteral();
  int parseSignedIntLiteral();
  double parseSignedFloatLiteral();
  hir::Test::Ptr parseTest();
  //void parseSystemGenerator();
  //void parseExternAssert();

  inline void reportError(const Token token, const std::string msg) {
    const unsigned lineNum = token.lineNum;
    const unsigned colNum = token.colNum;
    errors->push_back(ParseError(lineNum, colNum, lineNum, colNum, msg));
  }

  inline void skipTo(std::vector<TokenType> types) {
    while (peek().type != TokenType::END) {
      bool inTypes = false;

      for (auto &type : types) {
        if (peek().type == type) {
          inTypes = true;
          break;
        }
      }

      if (!inTypes) {
        tokens.skip();
      } else {
        break;
      }
    }
  }
  inline const Token consume(TokenType type) { 
    const Token token = peek();
    
    //std::cout << token << " " << Token(type,0,0) << std::endl;
    if (!tokens.consume(type)) {
      reportError(token, "unexpected token");
      throw SyntaxError();
    }

    return token;
  }
  inline bool tryconsume(TokenType type) { return tokens.consume(type); }
  inline Token peek(unsigned k = 0) { return tokens.peek(k); }

  TokenStream tokens;
  std::vector<ParseError> *errors;
};

}
}

#endif


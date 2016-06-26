#ifndef PARSER_NEW_H
#define PARSER_NEW_H

#include <exception>
#include <vector>
#include <utility>
#include <string>
#include <iostream>

#include "token.h"
#include "hir.h"
#include "error.h"

namespace simit { 
namespace internal {

class Parser {
public:
  Parser(std::vector<ParseError> *errors) : errors(errors) {}

  hir::Program::Ptr parse(const TokenStream &);

private:
  class SyntaxError : public std::exception {};

private:
  hir::Program::Ptr                      parseProgram();
  hir::HIRNode::Ptr                      parseProgramElement();
  hir::ElementTypeDecl::Ptr              parseElementTypeDecl();
  std::vector<hir::FieldDecl::Ptr>       parseFieldDeclList();
  hir::FieldDecl::Ptr                    parseFieldDecl();
  hir::HIRNode::Ptr                      parseExternFuncOrDecl();
  hir::ExternDecl::Ptr                   parseExternDecl();
  hir::FuncDecl::Ptr                     parseExternFuncDecl();
  hir::FuncDecl::Ptr                     parseFuncDecl();
  hir::FuncDecl::Ptr                     parseProcDecl();
  std::vector<hir::Identifier::Ptr>      parseTypeParams();
  std::vector<hir::Argument::Ptr>        parseArguments();
  hir::Argument::Ptr                     parseArgumentDecl();
  std::vector<hir::IdentDecl::Ptr>       parseResults();
  hir::StmtBlock::Ptr                    parseStmtBlock();
  hir::Stmt::Ptr                         parseStmt();
  hir::VarDecl::Ptr                      parseVarDecl();
  hir::ConstDecl::Ptr                    parseConstDecl();
  hir::IdentDecl::Ptr                    parseIdentDecl();
  hir::IdentDecl::Ptr                    parseTensorDecl();
  hir::WhileStmt::Ptr                    parseWhileStmt();
  hir::DoWhileStmt::Ptr                  parseDoWhileStmt();
  hir::IfStmt::Ptr                       parseIfStmt();
  hir::Stmt::Ptr                         parseElseClause();
  hir::ForStmt::Ptr                      parseForStmt();
  hir::ForDomain::Ptr                    parseForDomain();
  hir::PrintStmt::Ptr                    parsePrintStmt();
  hir::ApplyStmt::Ptr                    parseApplyStmt();
  hir::ExprStmt::Ptr                     parseExprOrAssignStmt();
  hir::Expr::Ptr                         parseExpr();
  hir::MapExpr::Ptr                      parseMapExpr();
  hir::Expr::Ptr                         parseOrExpr();
  hir::Expr::Ptr                         parseAndExpr();
  hir::Expr::Ptr                         parseXorExpr();
  hir::Expr::Ptr                         parseEqExpr();
  hir::Expr::Ptr                         parseTerm();
  hir::Expr::Ptr                         parseAddExpr();
  hir::Expr::Ptr                         parseMulExpr();
  hir::Expr::Ptr                         parseNegExpr();
  hir::Expr::Ptr                         parseExpExpr();
  hir::Expr::Ptr                         parseTransposeExpr();
  hir::Expr::Ptr                         parseCallOrReadExpr();
  hir::Expr::Ptr                         parseFactor();
  hir::Identifier::Ptr                   parseIdent();
  std::vector<hir::ReadParam::Ptr>       parseReadParams();
  hir::ReadParam::Ptr                    parseReadParam();
  std::vector<hir::Expr::Ptr>            parseCallParams();
  hir::Type::Ptr                         parseType();
  hir::ElementType::Ptr                  parseElementType();
  hir::SetType::Ptr                      parseSetType();
  std::vector<hir::Endpoint::Ptr>        parseEndpoints();
  hir::TupleLength::Ptr                  parseTupleLength();
  hir::TupleType::Ptr                    parseTupleType();
  hir::TensorType::Ptr                   parseTensorType();
  hir::NDTensorType::Ptr                 parseVectorBlockType();
  hir::NDTensorType::Ptr                 parseMatrixBlockType();
  hir::NDTensorType::Ptr                 parseTensorBlockType();
  hir::ScalarType::Ptr                   parseTensorComponentType();
  hir::ScalarType::Ptr                   parseScalarType();
  std::vector<hir::IndexSet::Ptr>        parseIndexSets();
  hir::IndexSet::Ptr                     parseIndexSet();
  hir::Expr::Ptr                         parseTensorLiteral();
  hir::DenseTensorLiteral::Ptr           parseDenseTensorLiteral();
  hir::DenseTensorLiteral::Ptr           parseDenseTensorLiteralInner();
  hir::DenseTensorLiteral::Ptr           parseDenseMatrixLiteral();
  hir::DenseTensorLiteral::Ptr           parseDenseVectorLiteral();
  hir::IntVectorLiteral::Ptr             parseDenseIntVectorLiteral();
  hir::FloatVectorLiteral::Ptr           parseDenseFloatVectorLiteral();
  hir::ComplexVectorLiteral::Ptr         parseDenseComplexVectorLiteral();
  int                                    parseSignedIntLiteral();
  double                                 parseSignedFloatLiteral();
  double_complex                         parseComplexLiteral();
  hir::Test::Ptr                         parseTest();

  void reportError(const Token &, std::string);

  Token peek(unsigned k = 0) const { return tokens.peek(k); }
  
  void  skipTo(std::vector<Token::Type>);
  Token consume(Token::Type); 
  bool  tryconsume(Token::Type type) { return tokens.consume(type); }

private:
  TokenStream              tokens;
  std::vector<ParseError> *errors;
};

}
}

#endif

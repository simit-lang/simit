#ifndef PARSER_H
#define PARSER_H

#include <exception>
#include <vector>
#include <utility>
#include <string>
#include <iostream>

#include "error.h"
#include "fir.h"
#include "token.h"
#include "util/scopedmap.h"

namespace simit { 
namespace internal {

class Parser {
public:
  Parser(const std::vector<fir::FuncDecl::Ptr> &, std::vector<ParseError> *);

  fir::Program::Ptr parse(const TokenStream &);

private:
  class SyntaxError : public std::exception {};
  
  enum class IdentType {GENERIC_PARAM, RANGE_GENERIC_PARAM, 
                        TUPLE, FUNCTION, OTHER};

  typedef util::ScopedMap<std::string, IdentType> SymbolTable;

private:
  fir::Program::Ptr                   parseProgram();
  fir::FIRNode::Ptr                   parseProgramElement();
  fir::ElementTypeDecl::Ptr           parseElementTypeDecl();
  std::vector<fir::FieldDecl::Ptr>    parseFieldDeclList();
  fir::FieldDecl::Ptr                 parseFieldDecl();
  fir::FIRNode::Ptr                   parseExternFuncOrDecl();
  fir::ExternDecl::Ptr                parseExternDecl();
  fir::FuncDecl::Ptr                  parseExternFuncDecl();
  fir::FuncDecl::Ptr                  parseFuncDecl();
  std::vector<fir::GenericParam::Ptr> parseGenericParams();
  fir::GenericParam::Ptr              parseGenericParam();
  std::vector<fir::Argument::Ptr>     parseArguments();
  fir::Argument::Ptr                  parseArgumentDecl();
  std::vector<fir::IdentDecl::Ptr>    parseResults();
  fir::StmtBlock::Ptr                 parseStmtBlock();
  fir::Stmt::Ptr                      parseStmt();
  fir::VarDecl::Ptr                   parseVarDecl();
  fir::ConstDecl::Ptr                 parseConstDecl();
  fir::IdentDecl::Ptr                 parseIdentDecl();
  fir::IdentDecl::Ptr                 parseTensorDecl();
  fir::WhileStmt::Ptr                 parseWhileStmt();
  fir::DoWhileStmt::Ptr               parseDoWhileStmt();
  fir::IfStmt::Ptr                    parseIfStmt();
  fir::Stmt::Ptr                      parseElseClause();
  fir::ForStmt::Ptr                   parseForStmt();
  fir::ForDomain::Ptr                 parseForDomain();
  fir::PrintStmt::Ptr                 parsePrintStmt();
  fir::ApplyStmt::Ptr                 parseApplyStmt();
  fir::ExprStmt::Ptr                  parseExprOrAssignStmt();
  fir::Expr::Ptr                      parseExpr();
  fir::MapExpr::Ptr                   parseMapExpr();
  fir::Expr::Ptr                      parseOrExpr();
  fir::Expr::Ptr                      parseAndExpr();
  fir::Expr::Ptr                      parseXorExpr();
  fir::Expr::Ptr                      parseEqExpr();
  fir::Expr::Ptr                      parseTerm();
  fir::Expr::Ptr                      parseAddExpr();
  fir::Expr::Ptr                      parseMulExpr();
  fir::Expr::Ptr                      parseNegExpr();
  fir::Expr::Ptr                      parseExpExpr();
  fir::Expr::Ptr                      parseTransposeExpr();
  fir::Expr::Ptr                      parseTensorReadExpr();
  fir::Expr::Ptr                      parseFieldReadExpr();
  fir::Expr::Ptr                      parseSetReadExpr();
  fir::Expr::Ptr                      parseFactor();
  fir::VarExpr::Ptr                   parseVarExpr();
  fir::RangeConst::Ptr                parseRangeConst();
  fir::CallExpr::Ptr                  parseCallExpr();
  fir::UnnamedTupleReadExpr::Ptr      parseUnnamedTupleReadExpr();
  fir::NamedTupleReadExpr::Ptr        parseNamedTupleReadExpr();
  fir::Identifier::Ptr                parseIdent();
  std::vector<fir::ReadParam::Ptr>    parseReadParams();
  fir::ReadParam::Ptr                 parseReadParam();
  std::vector<fir::Expr::Ptr>         parseExprParams();
  fir::Type::Ptr                      parseType();
  fir::ElementType::Ptr               parseElementType();
  fir::SetType::Ptr                   parseUnstructuredSetType();
  fir::SetType::Ptr                   parseLatticeLinkSetType();
  std::vector<fir::Endpoint::Ptr>     parseEndpoints();
  fir::Endpoint::Ptr                  parseEndpoint();
  fir::TupleElement::Ptr              parseTupleElement();
  fir::TupleType::Ptr                 parseNamedTupleType();
  fir::TupleLength::Ptr               parseTupleLength();
  fir::TupleType::Ptr                 parseUnnamedTupleType();
  fir::TensorType::Ptr                parseTensorType();
  fir::NDTensorType::Ptr              parseVectorBlockType();
  fir::NDTensorType::Ptr              parseMatrixBlockType();
  fir::NDTensorType::Ptr              parseTensorBlockType();
  fir::ScalarType::Ptr                parseTensorComponentType();
  fir::ScalarType::Ptr                parseScalarType();
  std::vector<fir::IndexSet::Ptr>     parseIndexSets();
  fir::IndexSet::Ptr                  parseIndexSet();
  fir::SetIndexSet::Ptr               parseSetIndexSet();
  fir::Expr::Ptr                      parseTensorLiteral();
  fir::DenseTensorLiteral::Ptr        parseDenseTensorLiteral();
  fir::DenseTensorLiteral::Ptr        parseDenseTensorLiteralInner();
  fir::DenseTensorLiteral::Ptr        parseDenseMatrixLiteral();
  fir::DenseTensorLiteral::Ptr        parseDenseVectorLiteral();
  fir::IntVectorLiteral::Ptr          parseDenseIntVectorLiteral();
  fir::FloatVectorLiteral::Ptr        parseDenseFloatVectorLiteral();
  fir::ComplexVectorLiteral::Ptr      parseDenseComplexVectorLiteral();
  int                                 parseSignedIntLiteral();
  double                              parseSignedFloatLiteral();
  double_complex                      parseComplexLiteral();
  fir::Test::Ptr                      parseTest();

  void reportError(const Token &, std::string);

  Token peek(unsigned k = 0) const { return tokens.peek(k); }
  
  void  skipTo(std::vector<Token::Type>);
  Token consume(Token::Type); 
  bool  tryConsume(Token::Type type) { return tokens.consume(type); }

private:
  SymbolTable decls;
  TokenStream tokens;

  const std::vector<fir::FuncDecl::Ptr> &intrinsics;
  std::vector<ParseError>               *errors;
};

}
}

#endif

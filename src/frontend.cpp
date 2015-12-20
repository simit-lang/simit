#include "frontend.h"

#include <sstream>
#include <iostream>

#include "program_context.h"
#include "error.h"
#include "parser/scanner.h"
#include "parser/parser.h"
#include "scanner.h"
#include "parser.h"
#include "hir.h"
#include "hir_visitor.h"
#include "hir_rewriter.h"
#include "func_call_rewriter.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
#if 0
  TokenStream tokens = ScannerNew::lex(programStream);
  hir::Program::Ptr program = ParserNew().parse(tokens, errors);
  std::cout << "after parse" << std::endl;
  std::cout << *program;
  //hir::HIRPrinter visitor(std::cout);
  //visitor.visit(program);
  std::cout << "before rewrite" << std::endl;
  program = hir::FuncCallRewriter(errors).rewrite<hir::Program>(program);
  std::cout << "after rewrite" << std::endl;
  std::cout << *program;
  return 1;
  //return (errors->size() == 0 ? 0 : 1);
  //std::cout << ScannerNew::lex(programStream);
#else
  Scanner scanner(&programStream);
  Parser parser(&scanner, ctx, errors);
  return parser.parse();
#endif
}

int Frontend::parseString(const std::string &programString, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
  std::istringstream programStream(programString);
  return parseStream(programStream, ctx, errors);
}

int Frontend::parseFile(const std::string &filename, ProgramContext *ctx,
                        std::vector<ParseError> *errors) {
  std::ifstream programStream(filename);
  if (!programStream.good()) {
    return 2;
  }
  return parseStream(programStream, ctx, errors);
}

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
#include "ir_emitter.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
#if 1
  // Lexical and syntactic analyses.
  TokenStream tokens = ScannerNew::lex(programStream);
  hir::Program::Ptr program = ParserNew().parse(tokens, errors);

  // Semantic analyses.
  program = hir::FuncCallRewriter(errors).rewrite<hir::Program>(program);
  
  // IR generation.
  hir::IREmitter(ctx).emitIR(program);
  //for (const auto &func : ctx->getFunctions()) {
  //  std::cout << func.second << std::endl;
  //}

  return (errors->size() == 0 ? 0 : 1);
#else
  Scanner scanner(&programStream);
  Parser parser(&scanner, ctx, errors);
  int ret = parser.parse();
  for (const auto &func : ctx->getFunctions()) {
    std::cout << func.second << std::endl;
  }
  return ret;
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

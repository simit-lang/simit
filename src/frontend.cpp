#include "frontend.h"

#include <sstream>
#include <iostream>
#include <algorithm>

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
#include "type_checker.h"
#include "const_fold.h"
#include "tuple_read_rewriter.h"
#include "assign_checker.h"
#include "pad_tensor_blocks.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
#if 1
  // Lexical and syntactic analyses.
  TokenStream tokens = ScannerNew(errors).lex(programStream);
  hir::Program::Ptr program = ParserNew(errors).parse(tokens);

  // Semantic analyses.
  program = hir::FuncCallRewriter(errors).rewrite<hir::Program>(program);
  program = hir::ConstantFolding().rewrite<hir::Program>(program);
  // hir::PadTensorBlocks().pad(program);
  hir::TypeChecker(errors).check(program);
  program = hir::TupleReadRewriter().rewrite<hir::Program>(program);
  hir::AssignChecker(errors).check(program);

  // Only emit IR if no syntactic or semantic error was found.
  if (!errors->empty()) {
    std::stable_sort(errors->begin(), errors->end());
    return 1;
  }
  
  // IR generation.
  hir::IREmitter(ctx).emitIR(program);
  //for (const auto &func : ctx->getFunctions()) {
  //  std::cout << func.second << std::endl;
  //}
  return 0;
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

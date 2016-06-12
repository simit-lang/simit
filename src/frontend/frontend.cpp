#include "frontend.h"

#include <sstream>
#include <iostream>
#include <algorithm>

#include "program_context.h"
#include "error.h"
#include "token.h"
#include "scanner.h"
#include "parser.h"
#include "hir.h"

#include "type_param_rewriter.h"
#include "func_call_rewriter.h"
#include "const_fold.h"
#include "pad_tensor_blocks.h"
#include "const_checker.h"
#include "type_checker.h"
#include "tuple_read_rewriter.h"
#include "ir_emitter.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
  // Lexical and syntactic analyses.
  TokenStream tokens = Scanner(errors).lex(programStream);
  hir::Program::Ptr program = Parser(errors).parse(tokens);

  // Semantic analyses.
  program = hir::TypeParamRewriter().rewrite(program);
//  std::cout << *program << std::endl;
  program = hir::FuncCallRewriter(errors).rewrite(program);
  program = hir::ConstantFolding().rewrite(program);
  // hir::PadTensorBlocks().pad(program);
  hir::ConstChecker(errors).check(program);
  //hir::InferElementSets().check(program);
  //program = hir::SpecializeGenericFunctions().rewrite(program);
  hir::TypeChecker(errors).check(program);
  program = hir::TupleReadRewriter().rewrite(program);

  // Only emit IR if no syntactic or semantic error was found.
  if (!errors->empty()) {
    std::stable_sort(errors->begin(), errors->end());
    return 1;
  }
  
  // IR generation.
  hir::IREmitter(ctx).emitIR(program);
  return 0;
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

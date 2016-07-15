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

#include "const_checker.h"
#include "const_fold.h"
#include "context_sensitive_rewriter.h"
#include "infer_element_sources.h"
#include "ir_emitter.h"
#include "pad_tensor_blocks.h"
#include "specialize_generic_functions.h"
#include "type_checker.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
  // Lexical and syntactic analyses.
  TokenStream tokens = Scanner(errors).lex(programStream);
  hir::Program::Ptr program = Parser(errors).parse(tokens);

  // Semantic analyses.
  program = hir::ContextSensitiveRewriter(errors).rewrite(program);
  program = hir::ConstantFolding().rewrite(program);
  // hir::PadTensorBlocks().pad(program);
  hir::ConstChecker(errors).check(program);
  hir::InferElementSources().infer(program);
  //std::cout << *program << std::endl;
  hir::SpecializeGenericFunctions().specialize(program);
  std::cout << *program << std::endl;
  hir::TypeChecker(errors).check(program);
  //std::cout << *program << std::endl;

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

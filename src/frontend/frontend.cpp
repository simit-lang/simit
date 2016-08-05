#include "frontend.h"

#include <sstream>
#include <iostream>
#include <algorithm>

#include "program_context.h"
#include "error.h"
#include "token.h"
#include "scanner.h"
#include "parser.h"
#include "fir.h"

#include "const_checker.h"
#include "const_fold.h"
#include "infer_element_sources.h"
#include "ir_emitter.h"
#include "fir_intrinsics.h"
#include "clone_generic_functions.h"
#include "type_checker.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<ParseError> *errors) {
  std::vector<fir::FuncDecl::Ptr> intrinsics = fir::createIntrinsics();

  // Lexical and syntactic analyses.
  TokenStream tokens = Scanner(errors).lex(programStream);
  fir::Program::Ptr program = Parser(errors).parse(tokens);

  // Semantic analyses.
  program = fir::ConstantFolding().rewrite(program);
  fir::ConstChecker(errors).check(program);
  fir::InferElementSources().infer(program);
  fir::CloneGenericFunctions(intrinsics).specialize(program);
  fir::TypeChecker(intrinsics, errors).check(program);

  // Only emit IR if no syntactic or semantic error was found.
  if (!errors->empty()) {
    std::stable_sort(errors->begin(), errors->end());
    return 1;
  }
  
  // IR generation.
  fir::IREmitter(ctx).emitIR(program);
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

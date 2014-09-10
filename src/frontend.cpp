#include "frontend.h"

#include <assert.h>
#include <sstream>

#include "program_context.h"
#include "errors.h"
#include "parser/scanner.h"
#include "parser/parser.h"

using namespace simit::internal;

// Frontend
int Frontend::parseStream(std::istream &programStream, ProgramContext *ctx,
                          std::vector<Error> *errors) {
  Scanner scanner(&programStream);
  Parser parser(&scanner, ctx, errors);
  return parser.parse();
}

int Frontend::parseString(const std::string &programString,
                          ProgramContext *ctx, std::vector<Error> *errors) {
  std::istringstream programStream(programString);
  return parseStream(programStream, ctx, errors);
}

int Frontend::parseFile(const std::string &filename, ProgramContext *ctx,
                        std::vector<Error> *errors) {
  std::ifstream programStream(filename);
  if (!programStream.good()) {
    return 2;
  }
  return parseStream(programStream, ctx, errors);
}

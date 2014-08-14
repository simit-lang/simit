#include "frontend.h"

#include <assert.h>
#include <sstream>

#include "symboltable.h"
#include "parser/scanner.h"
#include "parser/parser.h"

using namespace simit::internal;

/* Frontend */
int Frontend::parseStream(std::istream              &programStream,
                          std::vector<Function *>   *functions,
                          std::vector<simit::Error> *errors,
                          std::vector<Test*>        *tests) {
  Scanner scanner(&programStream);
  auto ctx = ParserParams(&symtable, functions, errors, tests);
  Parser parser(&scanner, &ctx);
  return parser.parse();
}

int Frontend::parseString(const std::string         &programString,
                          std::vector<Function*>    *functions,
                          std::vector<simit::Error> *errors,
                          std::vector<Test*>        *tests) {

  std::istringstream programStream(programString);
  return parseStream(programStream, functions, errors, tests);
}

int Frontend::parseFile(const std::string         &filename,
                        std::vector<Function*>    *functions,
                        std::vector<simit::Error> *errors,
                        std::vector<Test*>        *tests) {
  std::ifstream programStream(filename);
  if (!programStream.good()) {
    return 2;
  }
  return parseStream(programStream, functions, errors, tests);
}

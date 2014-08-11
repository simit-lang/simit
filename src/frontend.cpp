#include "frontend.h"

#include <assert.h>

#include "parser/scanner.h"
#include "parser/parser.h"
#include "ir.h"
#include "util.h"

using namespace simit::internal;
using namespace std;

/* SymbolTable */
void SymbolTable::addNode(const std::shared_ptr<IRNode> &irNode) {
  (*this)[irNode->getName()] = irNode;
}

std::shared_ptr<IRNode> &SymbolTable::operator[](const std::string &name) {
  for (auto &scope : scopes) {
    if (scope.find(name) != scope.end()) {
      return scope[name];
    }
  }
  return scopes.front()[name];
}

std::ostream &simit::internal::operator<<(std::ostream &os,
                                          const SymbolTable &table) {
  os << "SymbolTable:\n";
  for (auto scope : table.scopes) {
    for (auto symPair : scope) {
      string symString = (symPair.second == NULL)
                           ? "NULL"
                           : simit::util::toString(symPair.second);
      os << simit::util::indent(symPair.first + ":" + symString + ", ", 1);
    }
    os << "\n";
  }
  return os;
}


/* Frontend */
int Frontend::parseStream(std::istream              &programStream,
                          std::vector<Function *>   *functions,
                          std::vector<simit::Error> *errors,
                          std::vector<Test>         *tests) {
  Scanner scanner(&programStream);
  auto ctx = ParserParams(&symtable, functions, errors, tests);
  Parser parser(&scanner, &ctx);
  return parser.parse();
}

int Frontend::parseString(const std::string         &programString,
                          std::vector<Function*>    *functions,
                          std::vector<simit::Error> *errors,
                          std::vector<Test>         *tests) {

  std::istringstream programStream(programString);
  return parseStream(programStream, functions, errors, tests);
}

int Frontend::parseFile(const std::string         &filename,
                        std::vector<Function*>    *functions,
                        std::vector<simit::Error> *errors,
                        std::vector<Test>         *tests) {
  ifstream programStream(filename);
  if (!programStream.good()) {
    return 2;
  }
  return parseStream(programStream, functions, errors, tests);
}

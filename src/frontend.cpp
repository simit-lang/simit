#include "frontend.h"

#include <assert.h>

#include "ir.h"
#include "logger.h"
#include "tokens.h"
#include "scanner.h"
#include "util.h"

using namespace simit;
using namespace util;
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

std::ostream &simit::operator<<(std::ostream &os, const SymbolTable &table) {
  os << "SymbolTable:\n";
  for (auto scope : table.scopes) {
    for (auto symPair : scope) {
      string symString = (symPair.second == NULL)
                           ? "NULL"
                           : symPair.second->toString();
      os << util::indent(symPair.first + ":" + symString + ", ", 1);
    }
    os << "\n";
  }
  return os;
}


/* Frontend */
int Frontend::parseString(std::string programString,
                          std::list<std::shared_ptr<IRNode>> &programNodes,
                          std::list<simit::Error> &errors,
                          std::list<simit::Test> &tests) {
  log("Parsing program: ");
  logger.indent();
  log(programString);
  logger.dedent();

  struct yy_buffer_state *bufferState;
  bufferState = yy_scan_string(programString.c_str());
  auto ctx = ParseParams(symtable, errors, tests);
  int status = yyparse(&ctx);
  yylex_destroy();

  if (status == 0) {
    log("Parsed correctly");
    programNodes.insert(programNodes.end(),
                        ctx.programNodes.begin(), ctx.programNodes.end());
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }
}

int Frontend::parseFile(std::string filename,
                        std::list<std::shared_ptr<IRNode>> &programNodes,
                        std::list<simit::Error> &errors,
                        std::list<simit::Test> &tests) {
  log("Program: ");
  logger.indent();
  string line;
  ifstream file(filename);
  if (!file.good()) {
    log("Unable to open file");
    return 1;
  }
  while (getline (file, line)) {
    log(line);
  }
  file.close();
  logger.dedent();

  yyin = fopen(filename.c_str(), "r");
  auto ctx = ParseParams(symtable, errors, tests);
  int status = yyparse(&ctx);
  fclose(yyin);
  yylex_destroy();

  if (status == 0) {
    log("Parsed correctly");
    programNodes.insert(programNodes.end(),
                        ctx.programNodes.begin(), ctx.programNodes.end());
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }

}

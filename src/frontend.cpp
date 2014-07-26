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

std::string SymbolTable::toString() const {
  string result = "SymbolTable:\n";
  for (auto scope : scopes) {
    for (auto symPair : scope) {
      string symString = (symPair.second == NULL)
                           ? "NULL"
                           : symPair.second->toString();
      result += util::indent(symPair.first + ":" + symString + ", ", 1);
    }
    result += "\n";
  }
  return result;
}


/* Frontend */
int Frontend::parseString(string programString) {
  log("Parsing program: ");
  logger.indent();
  log(programString);
  logger.dedent();

  struct yy_buffer_state *bufferState;
  bufferState = yy_scan_string(programString.c_str());
  int status = yyparse(symtable, errors, tests);
  yylex_destroy();

  if (status == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }
}

int Frontend::parseFile(string filename) {
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
  int status = yyparse(symtable, errors, tests);
  fclose(yyin);
  yylex_destroy();

  if (status == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }

}

#include "Frontend.h"
#include <assert.h>

#include "IR.h"
#include "Logger.h"
#include "Tokens.h"
#include "Scanner.h"

using namespace simit;
using namespace util;
using namespace std;


/* SymbolTable */
void SymbolTable::addNode(IRNode *irNode) {
  (*this)[irNode->getName()] = irNode;
}

IRNode *&SymbolTable::operator[](const std::string &name) {
  for (auto scope : scopes) {
    if (scope.find(name) != scope.end()) {
      return scope[name];
    }
  }
  cout << "Adding symbol: " << name << endl;
  return scopes.front()[name];
}


/* Frontend */
extern FILE *yyin;
int yyparse(simit::Program *program);
struct yy_buffer_state *yy_scan_string(const char *);
int yylex_destroy();

Frontend::Frontend() {
}

Frontend::~Frontend() {
}

int Frontend::parseString(string programString, Program *program) {
  log("Program: ");
  logger.indent();
  log(programString);
  logger.dedent();

  struct yy_buffer_state *bufferState;
  bufferState = yy_scan_string(programString.c_str());
  int status = yyparse(symbolTable, program);
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

int Frontend::parseFile(string filename, Program *program) {
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
  int status = yyparse(symbolTable, program);
  yylex_destroy();
  //fclose(yyin);

  if (status == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }
}

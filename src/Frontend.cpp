#include "Frontend.h"
#include "IR.h"
#include "Logger.h"

#include <assert.h>

using namespace Simit;
using namespace std;

Frontend::Frontend() {
}

Frontend::~Frontend() {
}

int Frontend::parseString(string program,
                          vector<shared_ptr<IRNode> > &irNodes,
                          string &errors) {
  log("Program: ");
  logger.indent();
  log(program);
  logger.dedent();

  //  scanString(program);
  struct yy_buffer_state *bufferState;
  bufferState = yy_scan_string(program.c_str());

  IRNode *irNode = NULL;
  int status = yyparse(&irNode, &errors);
  yylex_destroy();
  if (status == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return -1;
  }
}

int Frontend::parseFile(std::string filename,
                        std::vector<std::shared_ptr<IRNode> > &irNodes,
                        std::string &errors) {
  log("Program: ");
  logger.indent();
  string line;
  ifstream file (filename);
  if (!file.is_open()) {
    log("Unable to open file");
    return -1;
  }
  while (getline (file, line)) {
    log(line);
  }
  file.close();
  logger.dedent();

  yyin = fopen(filename.c_str(), "r");

  IRNode *irNode = NULL;
  int status = yyparse(&irNode, &errors);
  fclose(yyin);
  if (status == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return -1;
  }
}

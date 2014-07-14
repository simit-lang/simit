#include "Frontend.h"
#include "IR.h"
#include "Logger.h"

#include <assert.h>

using namespace Simit;
using namespace std;

extern FILE *yyin;
int yyparse(Simit::Program *program);
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
  int status = yyparse(program);
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
  int status = yyparse(program);
  fclose(yyin);

  if (status == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }
}

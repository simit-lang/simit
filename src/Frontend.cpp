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

int Frontend::parse(const string &programString,
                    vector<shared_ptr<IRNode> > *irNodes,
                    string *errors) {
  assert(irNodes != NULL);

  log("Program: ");
  logger.indent();
  log(programString);
  logger.dedent();

  yy_scan_string(programString.c_str());

  IRNode *irNode = NULL;
  if (yyparse(&irNode) == 0) {
    log("Parsed correctly");
    return 0;
  }
  else {
    log("Parse error");
    return 1;
  }
}

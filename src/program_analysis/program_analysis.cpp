#include "program_analysis.h"

using namespace simit::ir;
using namespace simit::ir::program_analysis;

Func simit::ir::program_analysis::program_analysis(Func func) {
  func = CSE().rewrite(func);

  return func;
}

#ifndef SIMIT_IR_PROGRAM_ANALYSIS_H
#define SIMIT_IR_PROGRAM_ANALYSIS_H

#include "ir.h"

namespace simit {
namespace ir {
namespace program_analysis {

class CSE {
public:
  simit::ir::Func rewrite(simit::ir::Func func);
};

simit::ir::Func program_analysis(simit::ir::Func func);

}
}
}

#endif

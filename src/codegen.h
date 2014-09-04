#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include <vector>
#include <map>

#include "interfaces.h"

namespace simit {
class Diagnostics;
class Function;

namespace internal {
class Test;
class Function;

/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class CodeGen : simit::util::Uncopyable {
 public:
  CodeGen() {}
  virtual ~CodeGen() {}

  virtual simit::Function *compile(Function *function) = 0;

  int verify(std::map<std::string, Function*> &functions,
             const std::vector<Test*> &tests, Diagnostics *diags);
};

}} // namespace simit::internal
#endif

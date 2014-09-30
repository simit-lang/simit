#ifndef SIMIT_SIR_CODEGEN_H
#define SIMIT_SIR_CODEGEN_H

#include "ir_visitors.h"

#include <memory>
#include <stack>

namespace simit {

namespace internal {
template <typename, typename> class ScopedMap;
}

namespace ir {
class Function;

struct Stmt;
struct Block;

/// Code Generator that lowers tensor ir to set ir.
class SetIRCodeGen : public IRVisitor {
public:
  SetIRCodeGen();
  ~SetIRCodeGen();

  std::unique_ptr<Stmt> codegen(simit::ir::Function *function);

private:
//  internal::ScopedMap<std::string, SetIRNode*> *symtable;
  
  std::stack<Block*> blockStack;

  void handle(ir::Function *f);
  void handle(ir::IndexExpr *t);
};

}}

#endif

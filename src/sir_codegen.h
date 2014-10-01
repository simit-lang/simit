#ifndef SIMIT_SIR_CODEGEN_H
#define SIMIT_SIR_CODEGEN_H

#include "ir_visitors.h"

#include <memory>
#include <stack>
#include <string>

namespace simit {

namespace internal {
template <typename, typename> class ScopedMap;
}

namespace ir {
class Function;
class IRNode;
struct Stmt;
struct Expr;

/// Code Generator that lowers tensor ir to set ir.
class SetIRCodeGen : public IRVisitor {
public:
  SetIRCodeGen();
  ~SetIRCodeGen();

  std::unique_ptr<Stmt> codegen(Function *function);

private:
  internal::ScopedMap<std::string, Expr> *symtable;

  std::stack<Stmt> *scopeStack;

  void handle(Function *f);
  void handle(IndexExpr *t);
};

}}

#endif

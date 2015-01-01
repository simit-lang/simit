#include "ir_codegen.h"

#include <vector>

#include "ir_rewriter.h"
#include "ir_queries.h"
#include "ir_builder.h"
#include "indexvar.h"

using namespace std;

namespace simit {
namespace ir {

Stmt initializeLhsToZero(Stmt stmt) {
  class ReplaceRhsWithZero : public IRRewriter {
    void visit(const AssignStmt *op) {
      stmt = AssignStmt::make(op->var, 0.0);
    }

    void visit(const FieldWrite *op) {
      stmt = FieldWrite::make(op->elementOrSet, op->fieldName, 0.0);
    }

    void visit(const TensorWrite *op) {
      stmt = TensorWrite::make(op->tensor, op->indices, 0.0);
    }
  };

  return ReplaceRhsWithZero().rewrite(stmt);
}

}}

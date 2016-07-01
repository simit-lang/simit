#include "insert_frees.h"

#include <stack>

#include "ir.h"
#include "ir_rewriter.h"
#include "intrinsics.h"
#include "tensor_index.h"
#include "path_expressions.h"

using namespace std;

namespace simit {
namespace ir {

class InsertFrees : public IRRewriter {
public:
  InsertFrees(const Storage& storage) : storage{storage} {}

private:
  const Storage& storage;
  stack<vector<Var>> varsToFreeStack;

  using IRRewriter::visit;

  void visit(const Scope* op) {
    varsToFreeStack.push(vector<Var>());
    stmt = rewrite(op->scopedStmt);
    vector<Var> varsToFree = varsToFreeStack.top();
    for (auto var : varsToFree) {
      Stmt freeVar = CallStmt::make({}, intrinsics::free(), {var});
      stmt = Block::make(stmt, freeVar);
    }
    stmt = Scope::make(stmt);
    varsToFreeStack.pop();
  }

  void visit(const VarDecl* op) {
    // If a indexed tensor does not have a path expression, then its storage is
    // managed on the stack and it must be freed.
    Var var = op->var;
    if (storage.hasStorage(var)) {
      auto tensorStorage = storage.getStorage(var);
      if (tensorStorage.getKind() == TensorStorage::Indexed) {
        auto index = tensorStorage.getTensorIndex();
        if (!index.getPathExpression().defined()) {
          varsToFreeStack.top().push_back(index.getRowptrArray());
          varsToFreeStack.top().push_back(index.getColidxArray());
          varsToFreeStack.top().push_back(var);
        }
      }
    }
    stmt = op;
  }
};

Func insertFrees(Func func) {
  return InsertFrees(func.getStorage()).rewrite(func);
}

}}

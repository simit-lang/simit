#include "ir_queries.h"

#include <vector>
#include <set>

using namespace std;

namespace simit {
namespace ir {

class GetFreeIndexVars : private IRVisitor {
public:
  std::vector<IndexVar> get(Expr expr) {
    expr.accept(this);
    return indexVars;
  }

private:
  set<IndexVar> added;
  vector<IndexVar> indexVars;

  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isFreeVar() && added.find(iv) == added.end()) {
        added.insert(iv);
        indexVars.push_back(iv);
      }
    }
  }
};

std::vector<IndexVar> getFreeVars(Expr expr) {
  return GetFreeIndexVars().get(expr);
}


class GetReductionIndexVars : private IRVisitor {
public:
  std::vector<IndexVar> get(Expr expr) {
    expr.accept(this);
    return indexVars;
  }

private:
  set<IndexVar> added;
  vector<IndexVar> indexVars;

  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isReductionVar() && added.find(iv) == added.end()) {
        added.insert(iv);
        indexVars.push_back(iv);
      }
    }
  }
};

std::vector<IndexVar> getReductionVars(Expr expr) {
  return GetReductionIndexVars().get(expr);
}

// containsReduction
class ContainsReduction : public IRQuery {
  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isReductionVar()) {
        result = true;
        return;
      }
    }
  }
};

bool containsReduction(Expr expr) {
  return ContainsReduction().query(expr);
}

bool containsReduction(Stmt stmt) {
  return ContainsReduction().query(stmt);
}


// isFlattened
class CheckIsFlattened : private IRVisitor {
public:
  bool check(Stmt stmt) {
    isFlattened = true;
    indexExprFound = false;
    stmt.accept(this);
    return isFlattened;
  }

private:
  bool indexExprFound;
  bool isFlattened;

  void visit(const IndexExpr *op) {
    if (!indexExprFound) {
      indexExprFound = true;
    }
    else {
      isFlattened = false;
    }
  }
};

bool isFlattened(Stmt stmt) {
  return CheckIsFlattened().check(stmt);
}

}}

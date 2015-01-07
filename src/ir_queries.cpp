#include "ir_queries.h"

#include <vector>
#include <set>

#include "types.h"

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

// containsFreeVar
class ContainsFree : public IRQuery {
  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isFreeVar()) {
        result = true;
        return;
      }
    }
  }
};

bool containsFreeVar(Expr expr) {
  return ContainsFree().query(expr);
}

bool containsFreeVar(Stmt stmt) {
  return ContainsFree().query(stmt);
}


// containsReductionVar
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

bool containsReductionVar(Expr expr) {
  return ContainsReduction().query(expr);
}

bool containsReductionVar(Stmt stmt) {
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

bool isBlocked(Stmt stmt) {
  class IsBlockedVisitor : private IRVisitor {
  public:
    bool check(Stmt stmt) {
      isBlocked = false;
      stmt.accept(this);
      return isBlocked;
    }
  private:
    bool isBlocked;

    void updateBlocked(Expr expr) {
      Type type = expr.type();
      if (type.isTensor() && !isScalar(type.toTensor()->blockType())) {
        isBlocked = true;
      }
    }

    void visit(const IndexedTensor *op) {
      updateBlocked(op->tensor);
    }
  };

  return IsBlockedVisitor().check(stmt);
}

std::vector<Func> getCallTree(Func func) {
  class GetCallTreeVisitor : public IRVisitor {
  public:
    vector<Func> get(Func func) {
      callTree.clear();
      func.accept(this);
      return callTree;
    }

  private:
    set<Func> visited;
    vector<Func> callTree;

    void visit(const Call *op) {
      op->func.accept(this);
    }

    void visit(const Func *op) {
      if (visited.find(*op) != visited.end()) {
        return;
      }
      visited.insert(*op);
      callTree.push_back(*op);
      IRVisitor::visit(op);
    }
  };

  return GetCallTreeVisitor().get(func);
}

}}

#include "ir_queries.h"

#include <vector>
#include <set>
#include <stack>

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
  
  using IRVisitor::visit;

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
  
  using IRVisitor::visit;
  
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
  using IRQuery::visit;
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

bool containsIndexedTensor(Expr expr) {
  class ContainsIndexedTensor : public IRQuery {
    using IRQuery::visit;
    void visit(const IndexedTensor *op) {
      result = true;
    }
  };
  return ContainsIndexedTensor().query(expr);
}

// containsReductionVar
class ContainsReduction : public IRQuery {
  using IRQuery::visit;
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
  
  using IRVisitor::visit;

  void visit(const IndexExpr *op) {
    if (!indexExprFound) {
      indexExprFound = true;
    }
    else {
      isFlattened = false;
    }
  }
};

size_t countIndexVars(Expr expr) {
  class CountIndexVarsVisitor : public IRVisitor {
  public:
    using IRVisitor::visit;
    set<IndexVar> indexVars;

    size_t count(Expr expr) {
      indexVars.clear();
      expr.accept(this);
      return indexVars.size();
    }

    void visit(const IndexedTensor *op) {
      for (auto &indexVar : op->indexVars) {
        if (indexVars.find(indexVar) == indexVars.end()) {
          indexVars.insert(indexVar);
        }
      }
    }
  };
  return CountIndexVarsVisitor().count(expr);
}

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
    
    using IRVisitor::visit;

    void updateBlocked(Expr expr) {
      Type type = expr.type();
      if (type.isTensor() && !isScalar(type.toTensor()->getBlockType())) {
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
  class ReverseCallGraphBuilder : public IRVisitor {
  public:
    map<Func, set<Func>> get(Func func) {
      reverseCallGraph.clear();
      func.accept(this);
      return reverseCallGraph;
    }

  private:
    std::stack<Func> functionStack;
    map<Func, set<Func>> reverseCallGraph;

    void addReverseEdge(Func caller, Func callee) {
      if (reverseCallGraph.find(caller) == reverseCallGraph.end()) {
        reverseCallGraph[callee] = set<Func>();
      }
      reverseCallGraph[callee].insert(caller);
    }

    void visit(const Func *op) {
      if (op->getKind() != Func::Internal) return;
      functionStack.push(*op);
      op->getBody().accept(this);
      functionStack.pop();
    }

    void visit(const CallStmt *op) {
      iassert(functionStack.size() > 0);
      addReverseEdge(functionStack.top(), op->callee);
      op->callee.accept(this);
    }
  };

  class GetCallTree {
  public:
    map<Func, set<Func>> callGraph;
    set<Func> unmarked;
    set<Func> temporaryMarked;
    vector<Func> L;

    void visit(Func n) {
      tassert(temporaryMarked.find(n) == temporaryMarked.end())
          << "Not a call DAG (mutual recursion?)";
      if (unmarked.find(n) != unmarked.end()) {
        temporaryMarked.insert(n);
        for (auto &m : callGraph[n]) {
          visit(m);
        }
        unmarked.erase(n);
        temporaryMarked.erase(n);
        L.push_back(n);
      }
    }

    vector<Func> get(Func func) {
      class GetFuncs : IRVisitorCallGraph {
      public:
        set<Func> funcs;
        set<Func> get(Func func) {
          funcs.clear();
          func.accept(this);
          return funcs;
        }

        using IRVisitor::visit;
        void visit(const Func *f) {
          funcs.insert(*f);
          IRVisitorCallGraph::visit(f);
        }
      };
      callGraph = ReverseCallGraphBuilder().get(func);
      unmarked = GetFuncs().get(func);
      while (unmarked.size() > 0) {
        Func n = *unmarked.begin();
        visit(n);
      }
      return L;
    }
  };

  return GetCallTree().get(func);
}

}}

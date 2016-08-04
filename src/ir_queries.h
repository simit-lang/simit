#ifndef SIMIT_EXPR_QUERIES_H
#define SIMIT_EXPR_QUERIES_H

#include "ir.h"
#include "ir_rewriter.h"
#include "indexvar.h"

namespace simit {
namespace ir {

std::vector<IndexVar> getFreeVars(Expr expr);
std::vector<IndexVar> getReductionVars(Expr expr);

bool containsFreeVar(Expr expr);
bool containsFreeVar(Stmt stmt);

bool containsReductionVar(Expr expr);
bool containsReductionVar(Stmt stmt);

bool containsIndexedTensor(Expr expr);

size_t countIndexVars(Expr expr);

/// Returns true if the statement has been flattened (only contains one index
/// expression), and false otherwise.
bool isFlattened(Stmt stmt);

/// Returns true if it is an assignment, tensor write or field write, whose
/// rhs is a blocked tensor
bool isBlocked(Stmt stmt);

/// Returns the call tree of `func`. The call tree constains all functions
/// (transitively) called from `func`.
std::vector<Func> getCallTree(Func func);

/// Returns three pieces of a block of code: a node matching the given
/// predicate, the head prior to it, and the tail following it. The predicate
/// must be specific to a type of node.
template <typename T>
inline std::vector<Stmt> splitOnPredicate(
    Stmt stmt, std::function<bool(const T*)> pred) {
  const T* found = nullptr;
  Stmt foundStmt;
  // Find the first op that matches the given predicate. If this operation
  // lives inside any conditionals, keep it wrapped in those. If it lives
  // inside any while loops, throw an error (don't know how to handle these
  // without changing semantics).
  class SearchVisitor : public IRVisitor {
  public:
    SearchVisitor(std::function<bool(const T*)> pred)
        : pred(pred), found(nullptr) {}
    const T* getNode() const {return found;}
    Stmt getStmt() const {return foundStmt;}
  private:
    std::function<bool(const T*)> pred;
    const T* found;
    Stmt foundStmt;

    void visit(const T* op) {
      if (!found && pred(op)) {
        found = op;
        foundStmt = op;
      }
    }

    void visit(const IfThenElse* op) {
      if (foundStmt.defined()) return;

      op->thenBody.accept(this);
      if (foundStmt.defined()) {
        foundStmt = IfThenElse::make(op->condition, foundStmt);
      }
      else if (op->elseBody.defined()) {
        op->elseBody.accept(this);
        if (foundStmt.defined()) {
          foundStmt = IfThenElse::make(Not::make(op->condition), foundStmt);
        }
      }
    }

    void visit(const While* op) { not_supported_yet; }
  };
  SearchVisitor searchVisitor(pred);
  stmt.accept(&searchVisitor);
  found = searchVisitor.getNode();
  foundStmt = searchVisitor.getStmt();

  class SplitRewriter : public IRRewriter {
  private:
    const T* found;
    Stmt head, tail;

    bool visitDone = false;
    bool visitFirst = true;

    // NOTE: splitOnPredicate will fail to compile if T == Block,
    // but this is an unused corner case.
    void visit(const T* op) {
      if (op == found) {
        visitDone = true;
        stmt = Pass::make();
      }
      else {
        IRRewriter::visit(op);
      }
    }

    void visit(const Block* op) {
      Stmt first, rest;
      if (visitFirst) {
        // Visit in first-to-last order
        if (!visitDone) {
          first = rewrite(op->first);
        }
        if (!visitDone) {
          rest = rewrite(op->rest);
        }
      }
      else {
        // Visit in last-to-first order
        if (!visitDone) {
          rest = rewrite(op->rest);
        }
        if (!visitDone) {
          first = rewrite(op->first);
        }
      }
      stmt = Block::make(first, rest);
    }

    void visit(const IfThenElse *op) {
      Stmt thenBody, elseBody;
      if (visitFirst) {
        // Visit in then-to-else order
        if (!visitDone) {
          thenBody = rewrite(op->thenBody);
        }
        if (!visitDone && op->elseBody.defined()) {
          elseBody = rewrite(op->elseBody);
        }
      }
      else {
        // Visit in then-to-else order
        if (!visitDone && op->elseBody.defined()) {
          elseBody = rewrite(op->elseBody);
        }
        if (!visitDone) {
          thenBody = rewrite(op->thenBody);
        }
      }
      if (elseBody.defined()) {
        stmt = IfThenElse::make(op->condition, thenBody, elseBody);
      }
      else if (thenBody.defined()) {
        stmt = IfThenElse::make(op->condition, thenBody);
      }
    }

  public:
    SplitRewriter(const T* found) : found(found) {}

    Stmt getHead(Stmt s) {
      visitDone = false;
      visitFirst = true;
      return rewrite(s);
    }
    Stmt getTail(Stmt s) {
      visitDone = false;
      visitFirst = false;
      return rewrite(s);
    }
  };


  if (found != nullptr) {
    SplitRewriter rewriter(found);
    Stmt head = rewriter.getHead(stmt);
    Stmt tail = rewriter.getTail(stmt);
    return {foundStmt, head, tail};
  }
  else {
    // If not found, return everything in head
    return {Stmt(), stmt, Stmt()};
  }
}

}}

#endif

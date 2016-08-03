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
  match(stmt,
        std::function<void(const T*)>([&found, &pred](const T* op) {
            if (!found && pred(op)) {
              found = op;
            }
          }));

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
    return {Stmt(found), head, tail};
  }
  else {
    // If not found, return everything in head
    return {Stmt(), stmt, Stmt()};
  }
}

}}

#endif

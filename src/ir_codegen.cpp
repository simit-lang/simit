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
    using IRRewriter::visit;
    
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

Func insertVarDecls(Func func) {
  class InsertVarDeclsRewriter : public IRRewriter {
    /// The set of variables that have already been declared. We do not need a
    /// scoped map here because Vars are compared by identity and cannot shadow.
    set<Var> declared;

    void visit(const Func *f) {
      declared.clear();

      for (auto &argument : f->getArguments()) {
        declared.insert(argument);
      }

      for (auto &result : f->getResults()) {
        declared.insert(result);
      }

      for (auto &global : f->getEnvironment().globals) {
        declared.insert(global.first);
      }

      IRRewriter::visit(f);
    }

    void visit(const VarDecl *op) {
      declared.insert(op->var);
      stmt = op;
    }

    void visit(const AssignStmt *op) {
      if (declared.find(op->var) == declared.end()) {
        stmt = Block::make(VarDecl::make(op->var), op);
        declared.insert(op->var);
      }
      else {
        stmt = op;
      }
    }

    void visit(const Map *op) {
      stmt = op;
      for (auto &var : op->vars) {
        if (declared.find(var) == declared.end()) {
          stmt = Block::make(VarDecl::make(var), stmt);
          declared.insert(var);
        }
      }
    }
  };
  func = InsertVarDeclsRewriter().rewrite(func);
  return func;
}

}}

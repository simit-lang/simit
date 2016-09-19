#ifndef SIMIT_INLINE_H
#define SIMIT_INLINE_H

#include "ir.h"
#include "ir_rewriter.h"
#include "storage.h"
#include "tensor_index.h"
#include <vector>
#include <map>

namespace simit {
namespace ir {

class CallRewriter : public IRRewriter {
public:
  CallRewriter(Storage *storage, Environment *env)
      : storage(storage), env(env) {}

protected:
  Storage *storage;
  Environment *env;

  using IRRewriter::visit;

  bool shouldInline(const CallStmt *op);

  void visit(const CallStmt *op);
};

/// Rewrites a mapped function body to compute on sets w.r.t. a loop variable,
/// instead of arguments.
class MapFunctionRewriter : protected IRRewriter {
public:
  virtual ~MapFunctionRewriter() {}

  Stmt inlineMapFunc(const Map *map, Var targetLoopVar,
                     Storage *storage,
                     Var endpoints=Var(),
                     std::map<TensorIndex,Var> locs=std::map<TensorIndex,Var>(),
                     std::map<vector<int>, Expr> clocs={},
                     vector<Var> gridIndexVars={});

protected:
  std::map<Var,Var> resultToMapVar;
  Storage *storage;

  Expr targetLoopVar;
  vector<Var> gridIndexVars;

  // Arguments to map expr
  Expr targetSet;
  std::vector<Expr> neighborSets;
  Expr throughSet;

  // Args to assembly func
  Var target;
  Var neighbors;
  Var throughEdges;
  Var throughPoints;

  ReductionOperator reduction;

  // Compiled endpoints and locs arrays for matrix assembly
  Var endpoints;
  std::map<TensorIndex,Var> locs;

  // Compile-time version of locs, used for generating stencil indices
  std::map<vector<int>, Expr> clocs;

  /// Check if the given variable is a result variable
  bool isResult(Var var);

  /// Translate the given result variable into the map variable
  Var getMapVar(Var resultVar);

  using IRRewriter::visit;

    /// Replace element field reads with set field reads
  void visit(const FieldRead *op);
  
  /// Replace element field writes with set field writes
  void visit(const FieldWrite *op);

  /// Replace unnamed neighbor tuple reads with reads from target endpoints
  void visit(const UnnamedTupleRead *op);

  /// Replace named neighbor tuple reads with reads from target endpoints
  void visit(const NamedTupleRead *op);

  /// Replace relative grid indexing with computed indices
  void visit(const SetRead *op);

  /// Replace function formal results with map actual results
  void visit(const VarExpr *op);
};

Func inlineCalls(Func func);

/// Inlines the map returning a loop, using the given rewriter.
Stmt inlineMap(const Map *map, MapFunctionRewriter &rewriter,
               Storage* storage);

}}

#endif

#ifndef SIMIT_INLINE_H
#define SIMIT_INLINE_H

#include "ir.h"
#include "ir_rewriter.h"
#include "storage.h"
#include <vector>
#include <map>

using namespace std;

namespace simit {
namespace ir {

/// Utilities for working with offset indices
bool isAllZeros(vector<int> offsets);
vector<int> getOffsets(vector<Expr> offsets);

/// Rewrites a mapped function body to compute on sets w.r.t. a loop variable,
/// instead of arguments.
class MapFunctionRewriter : protected IRRewriter {
public:
  virtual ~MapFunctionRewriter() {}

  Stmt inlineMapFunc(const Map *map, Var targetLoopVar,
                     Storage *storage,
                     Var endpoints=Var(), Var locs=Var(),
                     std::map<vector<int>, Expr> clocs={},
                     vector<Var> latticeIndexVars={});

protected:
  std::map<Var,Var> resultToMapVar;
  Storage *storage;

  Expr targetLoopVar;
  vector<Var> latticeIndexVars;

  // Arguments to map expr
  Expr targetSet;
  Expr neighborSet;
  Expr throughSet;

  // Args to assembly func
  Var target;
  Var neighbors;
  Var throughEdges;
  Var throughPoints;

  ReductionOperator reduction;

  // Compiled endpoints and locs arrays for matrix assembly
  Var endpoints;
  Var locs;
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

  /// Replace neighbor tuple reads with reads from target endpoints
  void visit(const TupleRead *op);

  /// Replace relative lattice indexing with computed indices
  void visit(const SetRead *op);

  /// Replace function formal results with map actual results
  void visit(const VarExpr *op);
};

/// Inlines the map returning a loop, using the given rewriter.
Stmt inlineMap(const Map *map, MapFunctionRewriter &rewriter,
               Storage* storage);

}}

#endif

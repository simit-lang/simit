#ifndef SIMIT_INLINE_H
#define SIMIT_INLINE_H

#include "ir.h"
#include "ir_rewriter.h"
#include <vector>

namespace simit {
namespace ir {

/// Rewrites a mapped function body to compute on sets w.r.t. a loop variable,
/// instead of arguments.
class MapFunctionRewriter : protected IRRewriter {
public:
  virtual ~MapFunctionRewriter() {}
  Stmt inlineMapFunc(const Map *map, Var targetLoopVar);

protected:
  std::map<Var,Var> resultToMapVar;

  Expr targetLoopVar;

  Expr targetSet;
  Expr neighborSet;

  Var target;
  Var neighbors;

  /// Check if the given variable is a result variable
  bool isResult(Var var);

  /// Replace element field reads with set field reads
  virtual void visit(const FieldRead *op);
  
  /// Replace element field writes with set field writes
  virtual void visit(const FieldWrite *op);

  /// Replace neighbor tuple reads with reads from target endpoints
  virtual void visit(const TupleRead *op);

  /// Replace function formal results with map actual results
  virtual void visit(const VarExpr *op);
};

/// Inlines the map returning a loop, using the given rewriter.
Stmt inlineMap(const Map *map, MapFunctionRewriter &rewriter);

}}

#endif

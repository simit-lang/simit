#ifndef SIMIT_INLINE_H
#define SIMIT_INLINE_H

#include "ir.h"
#include "ir_rewriter.h"

namespace simit {
namespace ir {

/// Inline mapped function w.r.t. a loop variable over the target set
class InlineMappedFunction : public IRRewriter {
public:
  InlineMappedFunction(const Map *map, Var targetLoopVar);
  virtual ~InlineMappedFunction();

protected:
  std::map<Var,Var> resultToMapVar;

  Expr targetLoopVar;

  Expr targetSet;
  Expr neighborSet;

  Var target;
  Var neighbors;

  /// Replace element field reads with set field reads
  virtual void visit(const FieldRead *op);

  /// Replace neighbor tuple reads with reads from target endpoints
  virtual void visit(const TupleRead *op);

  /// Replace function formal results with map actual results
  virtual void visit(const VarExpr *op);
};

/// Inlines the mapped function with respect to the given loop variable over
/// the target set.
Stmt inlineMappedFunction(const Map *map, Var loopVar);

/// Inlines the map, returning a loop with the same effect.
Stmt inlineMap(const Map *map);

}}

#endif

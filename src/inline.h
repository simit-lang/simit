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
  virtual ~InlineMappedFunction() {}

protected:
  Expr targetLoopVar;

  Expr targetSet;
  Expr neighborSet;

  Var target;
  Var neighbors;

  /// Replace element field reads with set field reads
  virtual void visit(const FieldRead *op);

  /// Replace neighbor tuple read with reads from target endpoints
  virtual void visit(const TupleRead *op);
};

}}

#endif

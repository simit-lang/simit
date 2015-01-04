#include "gpu_ir.h"

#include <set>

#include "ir_rewriter.h"
#include "scopedmap.h"

namespace simit {
namespace internal {

void GPUSharding::addShardDomain(const ir::GPUFor *op) {
  ir::ForDomain domain = op->domain;
  GPUSharding::ShardDimension sharded = op->dimension;
  switch (sharded) {
    case X:
      xDomain = domain.indexSet;
      xSharded = true;
      break;
    case Y:
      yDomain = domain.indexSet;
      ySharded = true;
      break;
    case Z:
      zDomain = domain.indexSet;
      zSharded = true;
      break;
  }
}

GPUSharding::ShardDimension GPUSharding::maybeShardFor(const ir::For *op) {
  ir::ForDomain domain = op->domain;
  if (domain.kind == ir::ForDomain::IndexSet &&
      domain.indexSet.getKind() != ir::IndexSet::Range) {
    if (domain.indexSet == xDomain) {
      return X;
    }
    else if (domain.indexSet == yDomain) {
      return Y;
    }
    else if (domain.indexSet == zDomain) {
      return Z;
    }
    else if (!xSharded) {
      xDomain = domain.indexSet;
      xSharded = true;
      return X;
    }
    else if (!ySharded) {
      yDomain = domain.indexSet;
      ySharded = true;
      return Y;
    }
    else if (!zSharded) {
      zDomain = domain.indexSet;
      zSharded = true;
      return Z;
    }
  }
  return NONE;
}

}  // namespace simit::internal

namespace ir {

class ShardLoops : public IRRewriter {
public:
  Var getSharedVar() {
    return sharedVar;
  }

protected:
  void visit(const Func *f) {
    for (auto &arg : f->getArguments()) {
      symtable.insert(arg.getName(), false);
    }
    for (auto &res : f->getResults()) {
      symtable.insert(res.getName(), false);
    }
    IRRewriter::visit(f);
    // Add shared arg
    std::vector<Var> args = func.getArguments();
    sharedVar = Var("$shared", SetType::make(
        ElementType::make("$shared", sharedFields), {}));
    args.push_back(sharedVar);
    func = Func(func.getName(), args, func.getResults(),
                func.getBody(), func.getKind());
  }

  void visit(const For *op) {
    internal::GPUSharding::ShardDimension sharded = sharding.maybeShardFor(op);
    sharding.scope(sharded);
    symtable.scope();
    symtable.insert(op->var.getName(), false);
    IRRewriter::visit(op);
    symtable.unscope();
    sharding.unscope(sharded);

    // Rewrite the For statement with sharding information
    if (sharded != internal::GPUSharding::NONE) {
      const ir::For* forStmt = ir::to<ir::For>(stmt);
      stmt = GPUFor::make(forStmt->var, forStmt->domain,
                          forStmt->body, sharded);
    }
  }

  void visit(const AssignStmt *op) {
    const std::string& varName = op->var.getName();
    if (!symtable.contains(varName)) {
      firstAssign(op->var);
    }
    Expr value = rewrite(op->value);
    // Rewrite the assign as a field write
    if (symtable.get(varName)) {
      // TODO(gkanwar): This type is incorrect for the set, because it doesn't
      // include future fields.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), varName);
      stmt = Store::make(fieldRead, Literal::make(Int, &zero), value, op->cop);
    }
    else {
      stmt = AssignStmt::make(op->var, value, op->cop);
    }
  }

  void visit(const VarExpr *op) {
    // Rewrite the read as a field read from the shared set
    if (symtable.get(op->var.getName())) {
      // This type is incorrect for the set, because it doesn't
      // include future fields. ReplaceShared will fix this in a second pass.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), op->var.getName());
      expr = Load::make(fieldRead, Literal::make(Int, &zero));
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void firstAssign(Var var) {
    std::string varName = var.getName();
    int depth = sharding.getDepth();
    iassert(depth >= 0 && depth < 2)
    << "Sharding depth must be 0, 1, or 2";
    const TensorType *origType = var.getType().toTensor();
    Type type = TensorType::make(origType->componentType,
                                 origType->dimensions,
                                 origType->isColumnVector);
    for (int dim = 0; dim < depth; ++dim) {
      // Use a dummy dimension of 1
      const_cast<TensorType*>(type.toTensor())
          ->dimensions.emplace_back(IndexSet(1));
    }
    sharedFields.emplace_back(varName, type);
    symtable.insert(varName, true);
  }

  // Map from symbol to whether it is being lifted to the shared set
  internal::ScopedMap<std::string, bool> symtable;
  std::vector<Field> sharedFields;
  internal::GPUSharding sharding;


  Var sharedVar;
};

// Visit all IR nodes which can store Var, and replace if it is
// the shared var
class ReplaceShared : public IRRewriter {
public:
  ReplaceShared(Var sharedVar) : sharedVar(sharedVar) {}

protected:
  void visit(const VarExpr *op) {
    if (op->var.getName() == "$shared") {
      expr = VarExpr::make(sharedVar);
    }
    else {
      expr = op;
    }
  }

  void visit(const Map *op) {
    ierror << "Maps should be eliminated before sharding loops";
  }

  void visit(const AssignStmt *op) {
    if (op->var.getName() == "$shared") {
      Expr value = rewrite(op->value);
      stmt = AssignStmt::make(sharedVar, value, op->cop);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const ForRange *op) {
    if (op->var.getName() == "$shared") {
      Expr start = rewrite(op->start);
      Expr end = rewrite(op->end);
      Stmt body = rewrite(op->body);
      stmt = ForRange::make(sharedVar, start, end, body);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const For *op) {
    if (op->var.getName() == "$shared") {
      Stmt body = rewrite(op->body);
      stmt = For::make(sharedVar, op->domain, body);
    }
    else {
      IRRewriter::visit(op);
    }
  }
  
  Var sharedVar;
};

Func shardLoops(Func func) {
  ShardLoops shardingRewriter;
  Func sharded = shardingRewriter.rewrite(func);
  Var sharedVar = shardingRewriter.getSharedVar();
  Func final = ReplaceShared(sharedVar).rewrite(sharded);
  return final;
}

}}  // namespace simit::ir

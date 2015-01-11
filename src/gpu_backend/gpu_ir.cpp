#include "gpu_ir.h"

#include <set>

#include "ir_rewriter.h"
#include "scopedmap.h"

namespace simit {

static bool isShardable(const ir::For *loop) {
  return (loop->domain.kind == ir::ForDomain::IndexSet &&
          loop->domain.indexSet.getKind() != ir::IndexSet::Range);
}

namespace internal {

void GPUSharding::shardFor(const ir::For *op) {
  iassert(isShardable(op));
  std::cerr << "Shard\n" << *op << "\n";
  std::cerr << "...with domain " << op->domain << " kind " << op->domain.kind << " index set " << op->domain.indexSet.getKind() << "\n";
  // TODO(jrk) these repeats should never happen
  iassert (op->domain.indexSet != xDomain && op->domain.indexSet != yDomain && op->domain.indexSet != zDomain);
  if (!xSharded) {
    xDomain = op->domain.indexSet;
    xVar = op->var;
    xSharded = true;
  }
  else if (!ySharded) {
    yDomain = op->domain.indexSet;
    yVar = op->var;
    ySharded = true;
  }
  else {
    iassert(!zSharded);
    zDomain = op->domain.indexSet;
    zVar = op->var;
    zSharded = true;
  }
}

std::ostream &operator<<(std::ostream &os, const GPUSharding &shard) {
  os << "GPU Sharding [ ";
  if (shard.xSharded) os << shard.xVar << " : " << shard.xDomain;
  if (shard.ySharded) os << ", " << shard.yVar << " : " << shard.yDomain;
  if (shard.zSharded) os << ", " << shard.zVar << " : " << shard.zDomain;
  os << " ]\n";
  return os;
}

bool operator==(const GPUSharding& sharding1, const GPUSharding& sharding2) {
  return ((!sharding1.xSharded && !sharding2.xSharded) ||
          (sharding1.xSharded && sharding2.xSharded &&
          sharding1.xDomain == sharding2.xDomain)) &&
      ((!sharding1.ySharded && !sharding2.ySharded) ||
       (sharding1.ySharded && sharding2.ySharded &&
       sharding1.yDomain == sharding2.yDomain)) &&
      ((!sharding1.zSharded && !sharding2.zSharded) ||
       (sharding1.zSharded && sharding2.zSharded &&
       sharding1.zDomain == sharding2.zDomain));
}


}  // namespace simit::internal

namespace ir {

class ShardLoops : public IRRewriter {
public:
  ShardLoops() : currentKernelSharding(nullptr) {}

private:
  using IRRewriter::visit;

  void visit(const For *loop) {
    // Leave this as a loop if it's not a shardable loop, or if our kernel is
    // already full of dimensions.
    if (!isShardable(loop) ||
        (currentKernelSharding && currentKernelSharding->shardFull())) {
      IRRewriter::visit(loop);
      return;
    }
    
    bool ownsKernel = false;
    internal::GPUSharding _sharding;
    
    // TODO(jrk) we may never have multiple nested kernel loops, at all, in our input - simplify away entirely?
    // for now:
    iassert(currentKernelSharding);
    
    // if we're the first loop to en
    if (!currentKernelSharding) {
      ownsKernel = true;
      currentKernelSharding = &_sharding;
    }

    // TODO(jrk) enforce that no logic comes between kernel loops?
    currentKernelSharding->shardFor(loop);
    stmt = rewrite(loop->body);

    if (ownsKernel) {
      stmt = GPUKernel::make(stmt, *currentKernelSharding);
      currentKernelSharding = nullptr;
    }
  }

  internal::GPUSharding *currentKernelSharding;
};

class LiftGPUVars : public IRRewriter {
public:
  Var getSharedVar() {
    return sharedVar;
  }

protected:
  enum VarAction { NONE, LIFT };

  void visit(const Func *f) {
    for (auto &arg : f->getArguments()) {
      symtable.insert(arg, NONE);
    }
    for (auto &res : f->getResults()) {
      symtable.insert(res, NONE);
    }

    IRRewriter::visit(f);
    Stmt body = func.getBody();

    // Add shared arg
    std::vector<Var> args = func.getArguments();
    sharedVar = Var("$shared", SetType::make(
        ElementType::make("$shared", sharedFields), {}));
    args.push_back(sharedVar);
    func = Func(func.getName(), args, func.getResults(),
                body, func.getKind());
  }

  void visit(const AssignStmt *op) {
    const std::string& varName = op->var.getName();
    if (!symtable.contains(op->var)) {
      firstAssign(op->var);
    }
    Expr value = rewrite(op->value);
    // Rewrite the assign as a field write
    if (symtable.get(op->var) == LIFT) {
      std::cerr << "Lowering assign to $shared " << op->var << "\n";
      // TODO(gkanwar): This type is incorrect for the set, because it doesn't
      // include future fields.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), varName);
      stmt = Store::make(fieldRead, Literal::make(Int, &zero), value, op->cop);
      std::cerr << "   " << stmt << "\n";
    }
    else {
      stmt = AssignStmt::make(op->var, value, op->cop);
    }
  }

  void visit(const VarExpr *op) {
    // Rewrite the read as a field read from the shared set
    if (symtable.contains(op->var) && symtable.get(op->var) == LIFT) {
      std::cerr << "Lowering ref to $shared " << op->var << "\n";
      // This type is incorrect for the set, because it doesn't
      // include future fields. ReplaceShared will fix this in a second pass.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), op->var.getName());
      expr = Load::make(fieldRead, Literal::make(Int, &zero));
      std::cerr << "   " << expr << "\n";
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const VarDecl *op) {
    firstAssign(op->var, NONE);
    IRRewriter::visit(op);
  }

  void firstAssign(Var var, VarAction action=LIFT) {
    if (action == LIFT) {
      std::string varName = var.getName();
      // int depth = sharding.getDepth();
      // iassert(depth >= 0 && depth < 2)
      // << "Sharding depth must be 0, 1, or 2";
      // XXX: Just for now
      // iassert(depth == 0);
      const TensorType *origType = var.getType().toTensor();
      Type type = TensorType::make(origType->componentType,
                                   origType->dimensions,
                                   origType->isColumnVector);
      // for (int dim = 0; dim < depth; ++dim) {
      //   // Use a dummy dimension of 1
      //   const_cast<TensorType*>(type.toTensor())
      //       ->dimensions.emplace_back(IndexSet(1));
      // }
      sharedFields.emplace_back(varName, type);
    }
    symtable.insert(var, action);
  }

  // Map from symbol to transformative action
  internal::ScopedMap<ir::Var, VarAction> symtable;
  std::vector<Field> sharedFields;
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
      std::cerr << "Replacing reference to $shared\n";
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
      std::cerr << "Replacing assignment to $shared\n";
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
  LiftGPUVars liftingRewriter;
  std::cerr << func << "\n";

  std::cerr << "Sharding loops...\n";
  func = ShardLoops().rewrite(func);
  std::cerr << func << "\n";

  std::cerr << "Lifting shared variables...\n";
  func = liftingRewriter.rewrite(func);
  std::cerr << func << "\n";

  Var sharedVar = liftingRewriter.getSharedVar();
  std::cerr << "Replacing shared variables...\n";
  func = ReplaceShared(sharedVar).rewrite(func);
  std::cerr << func << "\n";

  return func;
}

}}  // namespace simit::ir

#include "gpu_ir.h"

#include <set>

#include "ir_rewriter.h"
#include "scopedmap.h"

namespace simit {
namespace internal {

GPUSharding::ShardDimension GPUSharding::maybeShardFor(const ir::For *op) {
  ir::ForDomain domain = op->domain;
  ir::Var var = op->var;
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
      xVar = var;
      xSharded = true;
      return X;
    }
    else if (!ySharded) {
      yDomain = domain.indexSet;
      yVar = var;
      ySharded = true;
      return Y;
    }
    else if (!zSharded) {
      zDomain = domain.indexSet;
      zVar = var;
      zSharded = true;
      return Z;
    }
  }
  return NONE;
}

bool operator==(const GPUSharding& sharding1, const GPUSharding& sharding2) {
  return (!sharding1.xSharded && !sharding2.xSharded ||
          sharding1.xSharded && sharding2.xSharded &&
          sharding1.xDomain == sharding2.xDomain) &&
      (!sharding1.ySharded && !sharding2.ySharded ||
       sharding1.ySharded && sharding2.ySharded &&
       sharding1.yDomain == sharding2.yDomain) &&
      (!sharding1.zSharded && !sharding2.zSharded ||
       sharding1.zSharded && sharding2.zSharded &&
       sharding1.zDomain == sharding2.zDomain);
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
      symtable.insert(arg, NONE);
    }
    for (auto &res : f->getResults()) {
      symtable.insert(res, NONE);
    }

    // Create an initial kernel
    kernels.emplace_back();
    // Rewrite the func, building list of kernels
    IRRewriter::visit(f);
    kernels.back().sharding = sharding;
    // Recreate the body from proto kernels
    Stmt body;
    if (isa<Block>(f->getBody())) {
      std::vector<Stmt> kernelStmts;
      for (const internal::GPUProtoKernel& kernel : kernels) {
        if (!kernel.stmts.empty()) {
          Stmt gpuKernel = GPUKernel::make(Block::make(kernel.stmts), kernel.sharding);
          kernelStmts.push_back(gpuKernel);
        }
      }
      body = kernelStmts.empty() ? Stmt() : Block::make(kernelStmts);
    }
    // Handle the case of a single Stmt
    else {
      body = GPUKernel::make(func.getBody(), sharding);
    }

    // Add shared arg
    std::vector<Var> args = func.getArguments();
    sharedVar = Var("$shared", SetType::make(
        ElementType::make("$shared", sharedFields), {}));
    args.push_back(sharedVar);
    func = Func(func.getName(), args, func.getResults(),
                body, func.getKind());
  }

  void visit(const Block *op) {
    internal::GPUSharding before = sharding;
    // Only preserve sharding if inside a sharded kernel
    sharding = before.inShard() ? before : internal::GPUSharding();
    Stmt first = rewrite(op->first);
    internal::GPUSharding after = sharding;
    if (first.defined() && !isa<Block>(first)) {
      if (before == after &&
          (!sharding.isSharded() || sharding.inShard())) {
        kernels.back().stmts.push_back(first);
      }
      else {
        // Save this kernel sharding
        kernels.back().sharding = before;
        // Make a new kernel
        internal::GPUProtoKernel kernel;
        kernel.stmts.push_back(first);
        kernels.push_back(kernel);
      }
    }

    before = sharding;
    // Only preserve sharding if inside a sharded kernel
    sharding = before.inShard() ? before : internal::GPUSharding();
    Stmt rest = rewrite(op->rest);
    after = sharding;
    if (rest.defined() && !isa<Block>(rest)) {
      if (before == after &&
          (!sharding.isSharded() || sharding.inShard())) {
        kernels.back().stmts.push_back(rest);
      }
      else {
        // Save this kernel sharding
        kernels.back().sharding = before;
        // Make a new kernel
        internal::GPUProtoKernel kernel;
        kernel.stmts.push_back(rest);
        kernels.push_back(kernel);
      }
    }

    if (first == op->first && rest == op->rest) {
      stmt = op;
    }
    else {
      if (first.defined() && rest.defined()) {
        stmt = Block::make(first, rest);
      }
      else if (first.defined() && !rest.defined()) {
        stmt = first;
      }
      else if (!first.defined() && rest.defined()) {
        stmt = rest;
      }
      else {
        stmt = Stmt();
      }
    }
  }

  void visit(const For *op) {
    internal::GPUSharding::ShardDimension sharded = sharding.maybeShardFor(op);
    symtable.scope();
    switch (sharded) {
      case internal::GPUSharding::X: {
        symtable.insert(op->var, XSHARD);
        break;
      }
      case internal::GPUSharding::Y: {
        symtable.insert(op->var, YSHARD);
        break;
      }
      case internal::GPUSharding::Z: {
        symtable.insert(op->var, ZSHARD);
        break;
      }
      default: {
        symtable.insert(op->var, NONE);
        break;
      }
    }
    sharding.scope(sharded);
    IRRewriter::visit(op);
    sharding.unscope(sharded);
    symtable.unscope();

    // Rewrite the For statement with sharding information
    if (sharded != internal::GPUSharding::NONE) {
      const ir::For* forStmt = ir::to<ir::For>(stmt);
      // Cannot assign directly from pointer held by stmt
      Stmt body = forStmt->body;
      stmt = body;
    }
  }

  void visit(const AssignStmt *op) {
    const std::string& varName = op->var.getName();
    if (!symtable.contains(op->var)) {
      firstAssign(op->var);
    }
    Expr value = rewrite(op->value);
    // Rewrite the assign as a field write
    if (symtable.get(op->var) == LIFT) {
      // TODO(gkanwar): This type is incorrect for the set, because it doesn't
      // include future fields.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), varName);
      stmt = Store::make(fieldRead, Literal::make(Int, &zero), value, op->cop);
    }
    else if (symtable.get(op->var) == NONE) {
      stmt = AssignStmt::make(op->var, value, op->cop);
    }
    else {
      ierror << "Invalid assign to sharded var: " << op->var;
    }
  }

  void visit(const VarExpr *op) {
    // Rewrite the read as a field read from the shared set
    VarAction action = symtable.get(op->var);
    if (action == LIFT) {
      // This type is incorrect for the set, because it doesn't
      // include future fields. ReplaceShared will fix this in a second pass.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), op->var.getName());
      expr = Load::make(fieldRead, Literal::make(Int, &zero));
    }
    else if (action == XSHARD) {
      expr = VarExpr::make(sharding.xVar);
    }
    else if (action == YSHARD) {
      expr = VarExpr::make(sharding.yVar);
    }
    else if (action == ZSHARD) {
      expr = VarExpr::make(sharding.zVar);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void firstAssign(Var var) {
    std::string varName = var.getName();
    int depth = sharding.getDepth();
    // iassert(depth >= 0 && depth < 2)
    // << "Sharding depth must be 0, 1, or 2";
    // XXX: Just for now
    iassert(depth == 0);
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
    symtable.insert(var, LIFT);
  }

  // Map from symbol to transformative action
  enum VarAction { NONE, LIFT, XSHARD, YSHARD, ZSHARD };
  internal::ScopedMap<ir::Var, VarAction> symtable;
  std::vector<Field> sharedFields;

  // Current sharding info
  internal::GPUSharding sharding;
  // List of kernels
  std::vector<internal::GPUProtoKernel> kernels;

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

#include "gpu_ir.h"

#include <set>

#include "ir_rewriter.h"
#include "scopedmap.h"

namespace simit {
namespace ir {

class ShardLoops : public IRRewriter {
  void visit(const Func *f) {
    std::cout << "visit Func" << std::endl;
    for (auto &arg : f->getArguments()) {
      symtable.insert(arg.getName(), false);
    }
    for (auto &res : f->getResults()) {
      symtable.insert(res.getName(), false);
    }
    IRRewriter::visit(f);
    // Add shared arg
    std::vector<Var> args = func.getArguments();
    args.emplace_back("$shared", SetType::make(
        ElementType::make("$shared", sharedFields), {}));
    func = Func(func.getName(), args, func.getResults(),
                func.getBody(), func.getKind());
    std::cout << "end visit Func" << std::endl;
  }

  void visit(const For *op) {
    std::cout << "visit For" << std::endl;
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
    std::cout << "end visit For" << std::endl;
  }

  void visit(const AssignStmt *op) {
    std::cout << "visit Assign" << std::endl;
    const std::string& varName = op->var.getName();
    if (!symtable.contains(varName)) {
      int depth = sharding.getDepth();
      iassert(depth >= 0 && depth < 2)
          << "Sharding depth must be 0, 1, or 2";
      const TensorType *origType = op->var.getType().toTensor();
      Type type = TensorType::make(origType->componentType,
                                   origType->dimensions,
                                   origType->isColumnVector);
      std::cout << "Adding symbol: " << varName
                << ", type: " << type << std::endl;
      for (int dim = 0; dim < depth; ++dim) {
        // Use a dummy dimension of 1
        const_cast<TensorType*>(type.toTensor())
            ->dimensions.emplace_back(IndexSet(1));
      }
      sharedFields.emplace_back(varName, type);
      symtable.insert(varName, true);
      std::cout << "Symbol added." << std::endl;
    }
    Expr value = rewrite(op->value);
    // Rewrite the assign as a field write
    if (symtable.get(varName)) {
      // TODO(gkanwar): This type is incorrect for the set, because it doesn't
      // include future fields.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      std::cout << "Make tensor write" << std::endl;
      auto fieldRead = FieldRead::make(Var("$shared", type), varName);
      std::cout << "sharedFields" << std::endl;
      for (auto &field : sharedFields) {
        std::cout << field.name << "," << field.type << std::endl;
      }
      std::cout << "Field read type: " << fieldRead.type() << std::endl;
      stmt = TensorWrite::make(fieldRead,
                               {Literal::make(Int, &zero)}, value);
      std::cout << "Done make tensor write" << std::endl;
    }
    else {
      stmt = AssignStmt::make(op->var, value);
    }
    std::cout << "end visit Assig" << std::endl;
  }

  void visit(const VarExpr *op) {
    std::cout << "visit Var: " << op->var.getName() << std::endl;
    // Rewrite the read as a field read from the shared set
    if (symtable.get(op->var.getName())) {
      // TODO(gkanwar): This type is incorrect for the set, because it doesn't
      // include future fields.
      Type type(SetType::make(ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      int zero = 0;
      auto fieldRead = FieldRead::make(Var("$shared", type), op->var.getName());
      std::cout << "sharedFields" << std::endl;
      for (auto &field : sharedFields) {
        std::cout << field.name << "," << field.type << std::endl;
      }
      std::cout << "Field read type: " << fieldRead.type() << std::endl;
      expr = TensorRead::make(
          fieldRead,
          {Literal::make(Int, &zero)});
    }
    else {
      IRRewriter::visit(op);
    }
    std::cout << "end visit Var: " << expr.type() << std::endl;
  }

protected:
  // Map from symbol to whether it is being lifted to the shared set
  internal::ScopedMap<std::string, bool> symtable;
  std::vector<Field> sharedFields;
  internal::GPUSharding sharding;
};

Func shardLoops(Func func) {
  return ShardLoops().rewrite(func);
}

}}  // namespace simit::ir

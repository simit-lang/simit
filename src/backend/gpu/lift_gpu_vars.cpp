#include "lift_gpu_vars.h"
#include "gpu_ir.h"

#include "ir_rewriter.h"
#include "util/scopedmap.h"

namespace simit {

namespace ir {

class LiftGPUVars : public IRRewriter {
public:
  LiftGPUVars() : currentKernelSharding(nullptr) {}

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
    sharedVar = Var("$shared", UnstructuredSetType::make(
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
      Type type(UnstructuredSetType::make(
          ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      auto fieldRead = FieldRead::make(Var("$shared", type), varName);
      stmt = Store::make(fieldRead, Literal::make(0), value, op->cop);
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
      Type type(UnstructuredSetType::make(
          ElementType::make("$shared", sharedFields), {}));
      // Build a 0 index into the $shared set
      auto fieldRead = FieldRead::make(Var("$shared", type), op->var.getName());
      expr = Load::make(fieldRead, Literal::make(0));
      std::cerr << "   " << expr << "\n";
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const VarDecl *op) {
    firstAssign(op->var);
    IRRewriter::visit(op);
  }

  void visit(const GPUKernel *op) {
    currentKernelSharding = &op->sharding;
    IRRewriter::visit(op);
    currentKernelSharding = nullptr;
  }

  void firstAssign(Var var) {
    if (!currentKernelSharding) {
      // lift
      std::string varName = var.getName();

      const TensorType *origType = var.getType().toTensor();
      Type type = TensorType::make(origType->getComponentType(),
                                   origType->getDimensions(),
                                   origType->isColumnVector);
      // for (int dim = 0; dim < depth; ++dim) {
      //   // Use a dummy dimension of 1
      //   const_cast<TensorType*>(type.toTensor())
      //       ->dimensions.emplace_back(IndexSet(1));
      // }
      sharedFields.emplace_back(varName, type);
      symtable.insert(var, LIFT);
    }
    else {
      symtable.insert(var, NONE);
    }
  }

  // Map from symbol to transformative action
  util::ScopedMap<ir::Var, VarAction> symtable;
  std::vector<Field> sharedFields;
  const backend::GPUSharding *currentKernelSharding;
  Var sharedVar;
};

// Visit all IR nodes which can store Var, and replace if it is
// the shared var.
// This is only necessary to be sure the type of the sharedVar
// actually matches, since it is expanded and rewritten for every
// new field appended in the LiftGPUVars pass.
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

#if 0 // this should never happen
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
#endif
  
  Var sharedVar;
};

Func liftGPUVars(Func func) {
  LiftGPUVars liftingRewriter;
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

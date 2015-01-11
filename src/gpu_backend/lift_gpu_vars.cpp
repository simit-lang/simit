#include "lift_gpu_vars.h"
#include "gpu_ir.h"

#include "ir_rewriter.h"
#include "scopedmap.h"

namespace simit {

namespace ir {

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

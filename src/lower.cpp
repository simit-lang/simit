#include "lower.h"

#include <set>

#include "ir.h"
#include "domain.h"
#include "ir_mutator.h"
#include "usedef.h"
#include "sig.h"
#include "indexvar.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {


Func lower(Func func) {
  func = insertTemporaries(func);
  func = lowerIndexExpressions(func);
  func = lowerMaps(func);
  func = lowerTensorAccesses(func);
  return func;
}


class GetFieldRead : public IRVisitor {
public:
  GetFieldRead(Expr elementOrSet, std::string fieldName)
      : elementOrSet(elementOrSet), fieldName(fieldName) {}

  Expr check(Expr expr) {
    expr.accept(this);
    return fieldRead;
  }

private:
  Expr elementOrSet;
  std::string fieldName;

  Expr fieldRead;

  void visit(const FieldRead *op) {
    if (op->elementOrSet == elementOrSet && op->fieldName == fieldName) {
      fieldRead = op;
    }
    else {
      IRVisitor::visit(op);
    }
  }
};

class IsFieldReduced : public IRVisitor {
public:
  IsFieldReduced(Expr fieldRead) : fieldRead(fieldRead) {}

  bool check (Expr expr) {
    expr.accept(this);
    return fieldReductionFound;
  }

private:
  Expr fieldRead;
  bool fieldReductionFound = false;

  void visit(const IndexedTensor *op) {
    if (op->tensor == fieldRead) {
      for (auto &iv : op->indexVars) {
        if (iv.isReductionVar()) {
          fieldReductionFound = true;
          return;
        }
      }
    }
  }
};

class InsertTemporaries : public IRMutator {
private:
  int id=0;

  void visit(const FieldWrite *op) {
    Expr elemOrSet = op->elementOrSet;
    std::string fieldName = op->fieldName;

    // If the same field is read and written in the same statement and the
    // values are combined/reduced (e.g. multiplied) then we must introduce a
    // temporary to avoid read/write interference.
    Expr fieldRead = GetFieldRead(elemOrSet, fieldName).check(op->value);
    if (!fieldRead.defined()) {
      stmt = op;
      return;
    }

    bool valsCombined = IsFieldReduced(fieldRead).check(op->value);
    if (!valsCombined) {
      stmt = op;
      return;
    }

    Type fieldType = getFieldType(elemOrSet, fieldName);

    Var tmp("tmp" + to_string(id++), fieldType);

    Stmt tmpAssignment = AssignStmt::make(tmp, op->value);
    Stmt writeTmpToField = FieldWrite::make(elemOrSet, fieldName, tmp);
    stmt = Block::make(tmpAssignment, writeTmpToField);
  }
};

Func insertTemporaries(Func func) {
  return InsertTemporaries().mutate(func);
}


class LoopVars : public SIGVisitor {
public:
  LoopVars() : ud(nullptr) {}
  LoopVars(const SIG &sig, const UseDef *ud) : ud(ud) {apply(sig);}

  const pair<Var,ForDomain> &getVar(const IndexVar &var) const {
    return vertexLoopVars.at(var);
  }

  const pair<Var,ForDomain> &getVar(const Var &tensor) const {
    return edgeLoopVars.at(tensor);
  }

  bool hasVar(const Var &tensor) const {
    return edgeLoopVars.find(tensor) != edgeLoopVars.end();
  }

private:
  const UseDef *ud;

  map<IndexVar, pair<Var,ForDomain>> vertexLoopVars;
  map<Var, pair<Var,ForDomain>> edgeLoopVars;

  void visit(const SIGVertex *v) {
    Var lvar(v->iv.getName(), Int());

    // The vertex is unconstrained and loops over it's whole domain.
    ForDomain ldom = v->iv.getDomain().getIndexSets()[0];

    vertexLoopVars[v->iv] = pair<Var,ForDomain>(lvar,ldom);
  }

  void visit(const SIGEdge *e) {
    std::string varName = "e";
    for (SIGVertex *nbr : e->endpoints) {
      varName += nbr->iv.getName();
    }
    Var lvar(varName, Int());

    VarDef varDef = ud->getDef(e->tensor);
    assert(varDef.getKind() == VarDef::Map);

    const Map *mapStmt = to<Map>(varDef.getStmt());
    ForDomain ldom = ForDomain(mapStmt->target);

    edgeLoopVars[e->tensor] = pair<Var,ForDomain>(lvar,ldom);
  }
};


/// Specializes index expressions to compute one value at the location specified
/// by the given loop variables
class SpecializeIndexExprs : public IRMutator {
public:
  SpecializeIndexExprs(const LoopVars *lvs) : lvs(lvs) {}

private:
  const LoopVars *lvs;
  map<Var,Expr> varExprs;

  Expr getVarExpr(const Var &var) {
    if (varExprs.find(var) == varExprs.end()) {
      varExprs[var] = VarExpr::make(var);
    }
    return varExprs.at(var);
  }

  void visit(const AssignStmt *op) {
    assert(isa<IndexExpr>(op->value) && "Can only specialize IndexExpr stmts");
    const IndexExpr *indexExpr = to<IndexExpr>(op->value);

    Var var = op->var;
    Expr value = mutate(indexExpr);

    if (indexExpr->resultVars.size() == 0) {
      stmt = AssignStmt::make(var, value);
    }
    else {
      Expr varExpr = getVarExpr(var);
      std::vector<Expr> indices;
      for (IndexVar const& iv : indexExpr->resultVars) {
        indices.push_back(lvs->getVar(iv).first);
      }
      stmt = TensorWrite::make(varExpr, indices, value);
    }
  }

  void visit(const FieldWrite *op) {
    assert(isa<IndexExpr>(op->value) && "Can only specialize IndexExpr stmts");
    const IndexExpr *indexExpr = to<IndexExpr>(op->value);

    Expr elementOrSet = mutate(op->elementOrSet);
    std::string fieldName = op->fieldName;
    Expr value = mutate(indexExpr);

    if (indexExpr->resultVars.size() == 0) {
      stmt = FieldWrite::make(elementOrSet, fieldName, value);
    }
    else {
      std::vector<Expr> indices;
      for (IndexVar const& iv : indexExpr->resultVars) {
        Expr varExpr = getVarExpr(lvs->getVar(iv).first);
        indices.push_back(varExpr);
      }
      Expr field = FieldRead::make(elementOrSet, fieldName);
      stmt = TensorWrite::make(field, indices, value);
    }
  }

  void visit(const IndexedTensor *op) {
    // TODO: Flatten IndexExpr. E.g. ((i) A(i,j) *  ((m) c(m)+b(m))(j) )
    if (isa<IndexExpr>(op->tensor)) {
      NOT_SUPPORTED_YET;
    }

    if (op->indexVars.size() == 0) {
      expr = op->tensor;
    }
    else {
      std::vector<Expr> indices;
      for (IndexVar const& iv : op->indexVars) {
        Expr varExpr = getVarExpr(lvs->getVar(iv).first);
        indices.push_back(varExpr);
      }
      expr = TensorRead::make(op->tensor, indices);
    }
  }

  void visit(const IndexExpr *op) {
    expr = mutate(op->value);
  }
};


class GetReductionTmpName : public IRVisitor {
public:
  string get(const TensorWrite *op) {
    op->tensor.accept(this);
    for (auto &index : op->indices) {
      index.accept(this);
    }
    return name;
  }

private:
  std::string name;

  void visit(const VarExpr *op) {
    IRVisitor::visit(op);
    name += op->var.name;
  }
};


/// Rewrites rstmt to reduce it's computed value into a temporary reduction
/// variable using the rop ReductionOperation.
class ReduceOverVar : public IRMutator {
public:
  ReduceOverVar(Stmt rstmt, ReductionOperator rop) : rstmt(rstmt), rop(rop) {}

  Var getTmpVar() {return tmpVar;}

  /// Retrieve a statement that writes the tmp variable to the original
  /// location of the rewritten statement.  If result is !defined then the
  /// reduction variable does not ned to be written back.
  Stmt getTmpWriteStmt() {return tmpWriteStmt;}

private:
  Stmt rstmt;
  ReductionOperator rop;

  Var tmpVar;
  Stmt tmpWriteStmt;

  void visit(const AssignStmt *op) {
    if (op == rstmt) {
      assert(isScalarTensor(op->value.type()) &&
             "assignment non-scalars should have been lowered by now");
      switch (rop.getKind()) {
        case ReductionOperator::Sum: {
          Expr varExpr = VarExpr::make(op->var);
          tmpVar = op->var;
          stmt = AssignStmt::make(op->var, Add::make(varExpr, op->value));
          break;
        }
        case ReductionOperator::Undefined:
          assert(false);
          break;
      }
    }
    else {
      stmt = op;
    }
  }

  void visit(const TensorWrite *op) {
    if (op == rstmt) {
      Expr tensor = op->tensor;
      std::vector<Expr> indices = op->indices;

      assert(tensor.type().isTensor());
      switch (rop.getKind()) {
        case ReductionOperator::Sum: {
          ScalarType componentType = tensor.type().toTensor()->componentType;
          string tmpVarName = GetReductionTmpName().get(op);
          tmpVar = Var(tmpVarName, TensorType::make(componentType));
          stmt = AssignStmt::make(tmpVar, Add::make(tmpVar, op->value));
          break;
        }
        case ReductionOperator::Undefined:
          assert(false);
          break;
      }
      tmpWriteStmt = TensorWrite::make(tensor, indices, VarExpr::make(tmpVar));
    }
  }
};



/// Get the variables that are used to index a tensor in an Expr as well as
/// the expression that indexes the tensors
class GetTensorRead : public IRVisitor {
public:
   const TensorRead *get(Var tensorVar, Stmt stmt) {
    this->tensorVar = tensorVar;
    stmt.accept(this);
    return tensorRead;
  }

private:
  Var tensorVar;
  const TensorRead *tensorRead;

  void visit(const TensorRead *op) {
    if (isa<VarExpr>(op->tensor)) {
      if (to<VarExpr>(op->tensor)->var == this->tensorVar) {
        tensorRead = op;
      }
    }
  }
};


class Substitute : public IRMutator {
public:
  Substitute(Expr oldExpr, Expr newExpr) {
    substitutions.insert(pair<Expr,Expr>(oldExpr, newExpr));
  }

  Substitute(map<Expr,Expr> substitutions) : substitutions(substitutions) {}

  Stmt mutate(Stmt stmt) {
    return IRMutator::mutate(stmt);
  }

  Expr mutate(Expr expr) {
    if (substitutions.find(expr) != substitutions.end()) {
      return substitutions.at(expr);
    }
    else {
      return IRMutator::mutate(expr);
    }
  }

private:
  map<Expr,Expr> substitutions;
};


/// Turns tensor writes into compound assignments (e.g. +=, *=)
/// \todo Generalize to include Assignments, FieldWrite, TupleWrite
class MakeTensorWriteCompound : public IRMutator {
public:
  enum CompoundOperator { Add };

  MakeTensorWriteCompound(CompoundOperator compoundOperator)
      : compoundOperator(compoundOperator) {}

private:
  CompoundOperator compoundOperator;

  void visit(const TensorWrite *op) {
    Expr tensorRead = TensorRead::make(op->tensor, op->indices);

    Expr value;
    switch (compoundOperator) {
      case Add:
        value = Add::make(tensorRead, op->value);
        break;
    }
    assert(value.defined());

    stmt = TensorWrite::make(op->tensor, op->indices, value);
  }
};


class InlineMappedFunctionInLoop : public IRMutator {
public:
  InlineMappedFunctionInLoop(Var lvar, Func func, Expr targets, Expr neighbors,
                             Var resultActual, Stmt computeStmt) {
    assert(func.getArguments().size() == 2 &&
           "mapped functions must have exactly two arguments");

    this->loopVar = lvar;

    this->targetSet = targets;
    this->neighborSet = neighbors;
    this->resultActual = resultActual;

    this->targetElement = func.getArguments()[0];
    this->neighborElements = func.getArguments()[1];

    this->results = set<Var>(func.getResults().begin(),func.getResults().end());

    this->computeStmt = computeStmt;
  }

private:
  set<Var> results;

  Var targetElement;
  Var neighborElements;
  Var resultActual;

  Expr loopVar;
  Expr targetSet;
  Expr neighborSet;

  Stmt computeStmt;

  // Turn argument field reads into loads from the buffer corresponding to that
  // field
  void visit(const FieldRead *op) {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    if (isa<VarExpr>(op->elementOrSet)) {
      if (to<VarExpr>(op->elementOrSet)->var == targetElement) {
        Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
        expr = Load::make(setFieldRead, loopVar);
        return;
      }
    }
    IRMutator::visit(op);
  }

  void visit(const TupleRead *op) {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = p(0); ... = tmp.x;
    if (isa<VarExpr>(op->tuple)) {
      if (to<VarExpr>(op->tuple)->var == neighborElements) {
        const TupleType *tupleType = op->tuple.type().toTuple();
        int cardinality = tupleType->size;

        Expr endpoints = IndexRead::make(targetSet, "endpoints");
        Expr indexExpr = Add::make(Mul::make(loopVar, cardinality), op->index);
        expr = Load::make(endpoints, indexExpr);
        return;
      }
    }
    IRMutator::visit(op);
  }

  void visit(const TensorWrite *op) {
    if (isa<VarExpr>(op->tensor)) {
      if (results.find(to<VarExpr>(op->tensor)->var) != results.end()) {
        Expr tensor = op->tensor;
        std::vector<Expr> indices;
        for (auto &index : op->indices) {
          indices.push_back(mutate(index));
        }
        Expr value = mutate(op->value);
        const TensorRead *tensorRead = GetTensorRead().get(resultActual,
                                                           computeStmt);
        assert(tensorRead->indices.size() == indices.size());

        map<Expr,Expr> substitutions;
        substitutions[tensorRead] = value;
        for (size_t i=0; i<indices.size(); ++i) {
          substitutions[tensorRead->indices[i]] = indices[i];
        }

        stmt = Substitute(substitutions).mutate(computeStmt);

        MakeTensorWriteCompound mac(MakeTensorWriteCompound::Add);
        stmt = mac.mutate(stmt);
        return;
      }
    }
    IRMutator::visit(op);
  }
};


class ReplaceRhsWithZero : public IRMutator {
  void visit(const AssignStmt *op) {
    stmt = AssignStmt::make(op->var, 0);
  }

  void visit(const FieldWrite *op) {
    stmt = FieldWrite::make(op->elementOrSet, op->fieldName, 0);
  }

  void visit(const TensorWrite *op) {
    stmt = TensorWrite::make(op->tensor, op->indices, 0);
  }
};

class LoopBuilder : public SIGVisitor {
public:
  LoopBuilder(const UseDef *ud) : ud(ud) {}

  Stmt create(const IndexExpr *indexExpr, Stmt indexStmt) {
    SIG sig = SIGBuilder(ud).create(indexExpr);
    lvs = LoopVars(sig, ud);

    componentType = indexExpr->type.toTensor()->componentType;

    initStmt = ReplaceRhsWithZero().mutate(indexStmt);
    computeStmt = SpecializeIndexExprs(&lvs).mutate(indexStmt);

    stmt = computeStmt;
    apply(sig);

    Stmt result = stmt;
    stmt = Stmt();
    return result;
  }

private:
  const UseDef *ud;
  LoopVars lvs;
  ScalarType componentType;
  Stmt initStmt;
  Stmt computeStmt;

  Stmt stmt;

  void visit(const SIGVertex *v) {
    pair<Var,ForDomain> loopVar = lvs.getVar(v->iv);
    Var lvar = loopVar.first;
    ForDomain ldom = loopVar.second;

    if (v->iv.isFreeVar()) {
      stmt = For::make(lvar, ldom, stmt);
    }
    else {
      ReduceOverVar rov(computeStmt, v->iv.getOperator());
      Stmt loopBody = rov.mutate(stmt);
      Var tmpVar = rov.getTmpVar();
      assert(tmpVar.defined());

      Stmt alloc = AssignStmt::make(tmpVar, Literal::make(tmpVar.type, {0}));
      Stmt loop = For::make(lvar, ldom, loopBody);

      Stmt tmpWriteStmt = rov.getTmpWriteStmt();
      if (tmpWriteStmt.defined()) {
        stmt = Block::make(alloc, Block::make(loop, tmpWriteStmt));
      }
      else {
        stmt = Block::make(alloc, loop);
      }
    }

    SIGVisitor::visit(v);
  }

  void visit(const SIGEdge *e) {
    pair<Var,ForDomain> loopVar = lvs.getVar(e->tensor);
    Var lvar = loopVar.first;
    ForDomain ldom = loopVar.second;

    VarDef varDef = ud->getDef(e->tensor);
    assert(varDef.getKind() == VarDef::Map);
    const Map *map = to<Map>(varDef.getStmt());

    // Inline the function into the loop
    Func func = map->function;
    Expr target = map->target;
    Expr nbrs = map->neighbors;

    InlineMappedFunctionInLoop rewriter(lvar, func, target, nbrs, e->tensor,
                                        computeStmt);
    Stmt loopBody = rewriter.mutate(func.getBody());
    Stmt loop = For::make(lvar, ldom, loopBody);
    stmt = Block::make(initStmt, loop);

    for (auto &v : e->endpoints) {
      visitedVertices.insert(v);
    }
  }
};

class LowerIndexExpressions : public IRMutator {
public:
  LowerIndexExpressions(const UseDef *ud) : ud(ud) {}

private:
  const UseDef *ud;

  Stmt lower(const IndexExpr *indexExpr, Stmt stmt) {
    return LoopBuilder(ud).create(indexExpr, stmt);
  }

  void visit(const IndexExpr *op) {
    assert(false &&
           "IndexExprs must be assigned to a var/field/tensor before lowering");
  }

  void visit(const AssignStmt *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(to<IndexExpr>(op->value), op);
    }
    else {
      IRMutator::visit(op);
    }
  }

  void visit(const FieldWrite *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(to<IndexExpr>(op->value), op);
    }
    else {
      IRMutator::visit(op);
    }
  }

  void visit(const TensorWrite *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(to<IndexExpr>(op->value), op);
    }
    else {
      IRMutator::visit(op);
    }
  }
};

Func lowerIndexExpressions(Func func) {
  UseDef ud(func);
  return LowerIndexExpressions(&ud).mutate(func);
}


class LowerMaps : public IRMutator {
private:
  void visit(const Map *op) {
    // \todo We should only drop the map statements if it's bound Vars have
    // no uses (extend/invert UseDef to get DefUse info).
  }
};

Func lowerMaps(Func func) {
  return LowerMaps().mutate(func);
}


class LowerTensorAccesses : public IRMutator {
  void visit(const TensorRead *op) {
    assert(op->type.isTensor() && op->tensor.type().toTensor());

    const TensorType *type = op->tensor.type().toTensor();
    assert(type->order() == op->indices.size());

    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    assert(type->order() <= 2);

    if (type->order() == 1) {
      Expr tensor = mutate(op->tensor);
      Expr index = mutate(op->indices[0]);
      expr = Load::make(tensor, index);
    }
    else if (type->order() == 2) {
      // TODO: Clearly we need something more sophisticated here (for sparse
      // tensors or nested dense tensors).  For example, a tensor type could
      // carry a 'TensorStorage' object and we could ask this TensorStorage to
      // give us an Expr that computes an i,j location, or an Expr that gives us
      // a row/column.
      Expr tensor = mutate(op->tensor);

      Expr i = mutate(op->indices[0]);
      Expr j = mutate(op->indices[1]);

      IndexDomain dim1 = type->dimensions[1];
      Expr d1;
      if (dim1.getIndexSets().size() == 1 &&
          dim1.getIndexSets()[0].getKind() == IndexSet::Range) {
        // TODO: Add support for unsigned ScalarTypes
        assert(dim1.getSize() < (size_t)(-1));
        int dimSize = static_cast<int>(dim1.getSize());
        d1 = Literal::make(i.type(), &dimSize);
      }
      else {
        NOT_SUPPORTED_YET;
      }
      assert(d1.defined());

      Expr index = Add::make(Mul::make(i, d1), j);
      expr = Load::make(tensor, index);
    }
    else {
      NOT_SUPPORTED_YET;
    }
  }

  void visit(const TensorWrite *op) {
    assert(op->tensor.type().isTensor());

    const TensorType *type = op->tensor.type().toTensor();
    assert(type->order() == op->indices.size());

    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    assert(type->order() <= 2);

    if (type->order() == 1) {
      Expr tensor = mutate(op->tensor);
      Expr index = mutate(op->indices[0]);
      Expr value = mutate(op->value);
      stmt = Store::make(tensor, index, value);
    }
    else if (type->order() == 2) {
      // TODO: Clearly we need something more sophisticated here (for sparse
      // tensors or nested dense tensors).  For example, a tensor type could
      // carry a 'TensorStorage' object and we could ask this TensorStorage to
      // give us an Expr that computes an i,j location, or an Expr that gives us
      // a row/column.
      Expr tensor = mutate(op->tensor);

      Expr i = mutate(op->indices[0]);
      Expr j = mutate(op->indices[1]);

      IndexDomain dim1 = type->dimensions[1];
      Expr d1;
      if (dim1.getIndexSets().size() == 1 &&
          dim1.getIndexSets()[0].getKind() == IndexSet::Range) {
        // TODO: Add support for unsigned ScalarTypes
        assert(dim1.getSize() < (size_t)(-1));
        int dimSize = static_cast<int>(dim1.getSize());
        d1 = Literal::make(i.type(), &dimSize);
      }
      else {
        NOT_SUPPORTED_YET;
      }
      assert(d1.defined());

      Expr index = Add::make(Mul::make(i, d1), j);
      Expr value = mutate(op->value);
      stmt = Store::make(tensor, index, value);
    }
    else {
      NOT_SUPPORTED_YET;
    }
  }
};

Func lowerTensorAccesses(Func func) {
  return LowerTensorAccesses().mutate(func);
}

}}

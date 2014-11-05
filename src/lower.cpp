#include "lower.h"

#include <set>

#include "ir.h"
#include "domain.h"
#include "ir_rewriter.h"
#include "usedef.h"
#include "sig.h"
#include "indexvar.h"
#include "util.h"
#include "ir_builder.h"

using namespace std;

namespace simit {
namespace ir {

/// Static namegen (hacky: fix later)
string tmpNameGen() {
  static int i = 0;
  return "tmp" + to_string(i++);
}


Func lower(Func func) {
  func = insertTemporaries(func);
  func = flattenIndexExpressions(func);
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


// TODO: Change to
class InsertTemporaries : public IRRewriter {
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


static Func replaceBody(Func func, Stmt body) {
  return Func(func.getName(), func.getArguments(), func.getResults(), body,
              func.getKind());
}

Func flattenIndexExpressions(Func func) {
  Stmt body = flattenIndexExpressions(func.getBody());
  return replaceBody(func, body);
}

Func lowerIndexExpressions(Func func) {
  UseDef ud(func);
  Stmt body = lowerIndexExpressions(func.getBody(), ud);
  return replaceBody(func, body);
}

Func lowerMaps(Func func) {
  Stmt body = lowerMaps(func.getBody());
  return replaceBody(func, body);
}

Func lowerTensorAccesses(Func func) {
  Stmt body = lowerTensorAccesses(func.getBody());
  return replaceBody(func, body);
}


class SubstituteIndexVars : public IRRewriter {
public:
  SubstituteIndexVars(map<IndexVar,IndexVar> subs) : subs(subs) {}

private:
  map<IndexVar,IndexVar> subs;

  void visit(const IndexedTensor *op) {
    vector<IndexVar> indexVars;
    for (auto &iv : op->indexVars) {
      indexVars.push_back((subs.find(iv)!=subs.end()) ? subs.at(iv) : iv);
    }
    expr = IndexedTensor::make(op->tensor, indexVars);
  }
};

class GetFreeIndexVars : private IRVisitor {
public:
  std::vector<IndexVar> get(Expr expr) {
    expr.accept(this);
    return indexVars;
  }

private:
  set<IndexVar> added;
  vector<IndexVar> indexVars;

  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isFreeVar() && added.find(iv) == added.end()) {
        added.insert(iv);
        indexVars.push_back(iv);
      }
    }
  }
};

class GetReductionIndexVars : private IRVisitor {
public:
  std::vector<IndexVar> get(Expr expr) {
    expr.accept(this);
    return indexVars;
  }

private:
  set<IndexVar> added;
  vector<IndexVar> indexVars;

  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isReductionVar() && added.find(iv) == added.end()) {
        added.insert(iv);
        indexVars.push_back(iv);
      }
    }
  }
};

/// Retrieves the IndexVars that are used in an expression.
class HasReduction : private IRVisitor {
public:
  bool check(Stmt stmt) {
    stmt.accept(this);
    return hasReduction;
  }

  bool check(Expr expr) {
    expr.accept(this);
    return hasReduction;
  }

private:
  bool hasReduction = false;

  void visit(const IndexedTensor *op) {
    for (auto &iv : op->indexVars) {
      if (iv.isReductionVar()) {
        hasReduction = true;
        return;
      }
    }
  }
};

std::vector<IndexVar> getFreeVars(Expr expr) {
  return GetFreeIndexVars().get(expr);
}

std::vector<IndexVar> getReductionVars(Expr expr) {
  return GetReductionIndexVars().get(expr);
}

bool overlaps(const std::vector<IndexVar> &as, const std::vector<IndexVar> &bs){
  set<IndexVar> aset(as.begin(), as.end());
  for (auto &b : bs) {
    if (aset.find(b) != aset.end()) {
      return true;
    }
  }
  return false;
}

/// Flattens nested IndexExprs.
/// E.g. ({i,j} (({i} a{i}){i} * a{j})) -> ({i,j} (a{i} * a{j}))
class FlattenIndexExpressions : private IRRewriter {
public:
  Stmt flatten(Stmt stmt) {
    return mutate(stmt);
  }

private:
  std::vector<Stmt> stmts;

  Expr mutate(Expr e) {
    if (e.defined()) {
      e.accept(this);
      e = expr;
    }
    else {
      e = Expr();
    }
    expr = Expr();
    stmt = Stmt();
    return e;
  }

  Stmt mutate(Stmt s) {
    if (s.defined()) {
      s.accept(this);
      stmts.push_back(stmt);
      s = (stmts.size() > 0) ? Block::make(stmts) : stmt;
      stmts.clear();
    }
    else {
      s = Stmt();
    }
    expr = Expr();
    stmt = Stmt();
    return s;
  }

  pair<Expr,Expr> splitInterferringExprs(Expr a, Expr b) {
    std::vector<IndexVar> arvars = getReductionVars(a);
    std::vector<IndexVar> brvars = getReductionVars(b);
    if (arvars.size() > 0 && !overlaps(arvars, brvars)) {
      vector<IndexVar> afvars = getFreeVars(a);
      vector<IndexDomain> adims;
      for (auto &afvar : afvars) {
        adims.push_back(afvar.getDomain());
      }
      Type atype = TensorType::make(a.type().toTensor()->componentType, adims);
      Var atmp(tmpNameGen(), atype);
      Expr aiexpr = IndexExpr::make(afvars, a);
      Stmt astmt = AssignStmt::make(atmp, aiexpr);
      stmts.push_back(astmt);

      vector<IndexVar> bfvars = getFreeVars(b);
      vector<IndexDomain> bdims;
      for (auto &bfvar : bfvars) {
        bdims.push_back(bfvar.getDomain());
      }
      Type btype = TensorType::make(a.type().toTensor()->componentType, bdims);
      Var btmp(tmpNameGen(), btype);
      Expr biexpr = IndexExpr::make(bfvars, b);
      Stmt bstmt = AssignStmt::make(btmp, biexpr);
      stmts.push_back(bstmt);

      a = IndexedTensor::make(VarExpr::make(atmp), afvars);
      b = IndexedTensor::make(VarExpr::make(btmp), bfvars);
    }
    return pair<Expr,Expr>(a,b);
  }

  void visit(const Sub *op) {
    assert(isScalar(op->a.type()) || isa<IndexedTensor>(op->a));
    assert(isScalar(op->b.type()) || isa<IndexedTensor>(op->b));
    Expr a = mutate(op->a);
    Expr b = mutate(op->b);

    pair<Expr,Expr> ab = splitInterferringExprs(a, b);
    expr = Sub::make(ab.first, ab.second);
  }

  // TODO: Add .* amd ./ too
  void visit(const Add *op) {
    assert(isScalar(op->a.type()) || isa<IndexedTensor>(op->a));
    assert(isScalar(op->b.type()) || isa<IndexedTensor>(op->b));

    Expr a = mutate(op->a);
    Expr b = mutate(op->b);

    pair<Expr,Expr> ab = splitInterferringExprs(a, b);
    expr = Add::make(ab.first, ab.second);
  }


  void visit(const IndexedTensor *op) {
    // IndexExprs that are nested inside another IndexExpr must necessarily
    // produce a tensor and therefore be indexed through an IndexedTensor expr.
    if (isa<IndexExpr>(op->tensor)) {
      Expr tensor = mutate(op->tensor);
      const IndexExpr *indexExpr = to<IndexExpr>(tensor);
      assert(indexExpr->resultVars.size() == op->indexVars.size());

      map<IndexVar,IndexVar> substitutions;
      for (size_t i=0; i < indexExpr->resultVars.size(); ++i) {
        pair<IndexVar,IndexVar> sub(indexExpr->resultVars[i], op->indexVars[i]);
        substitutions.insert(sub);
      }

      expr = SubstituteIndexVars(substitutions).mutate(indexExpr->value);
    }
    else {
      IRRewriter::visit(op);
    }
  }
};

Stmt flattenIndexExpressions(Stmt stmt) {
  return FlattenIndexExpressions().flatten(stmt);
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

  friend std::ostream &operator<<(std::ostream &os, const LoopVars &lvs);

private:
  const UseDef *ud;

  map<IndexVar, pair<Var,ForDomain>> vertexLoopVars;
  map<Var, pair<Var,ForDomain>> edgeLoopVars;

  void visit(const SIGVertex *v) {
    Var lvar(v->iv.getName(), Int);

    // The vertex is unconstrained and loops over it's whole domain.
    ForDomain ldom = v->iv.getDomain().getIndexSets()[0];

    vertexLoopVars[v->iv] = pair<Var,ForDomain>(lvar,ldom);
  }

  void visit(const SIGEdge *e) {
    std::string varName = "e";
    for (SIGVertex *nbr : e->endpoints) {
      varName += nbr->iv.getName();
    }
    Var lvar(varName, Int);

    VarDef varDef = ud->getDef(e->tensor);
    assert(varDef.getKind() == VarDef::Map);

    const Map *mapStmt = to<Map>(varDef.getStmt());
    ForDomain ldom = ForDomain(mapStmt->target);

    edgeLoopVars[e->tensor] = pair<Var,ForDomain>(lvar,ldom);
  }
};

inline std::ostream &operator<<(std::ostream &os, const LoopVars &lvs) {
  for (auto &vlv : lvs.vertexLoopVars) {
    os << vlv.second.first << ",";
  }
  for (auto &elv : lvs.edgeLoopVars) {
    os << elv.second.first << ",";
  }
  return os;
}


/// Specializes index expressions to compute one value/block at the location
/// specified by the given loop variables
class RemoveIndexExprs : public IRRewriter {
public:
  RemoveIndexExprs(const LoopVars *lvs) : lvs(lvs) {}

private:
  const LoopVars *lvs;
  map<Var,Expr> varExprs;

  map<string,IndexVar> indexVars;

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

    std::vector<IndexVar> indexVars = GetFreeIndexVars().get(value);
    // TODO: Simplify by emitting empty IndexedTensors for scalar
    //       expressions (in visit(IndexedTensor)). Then we can remove the test
    //       and always turn value into an IndexExpr
    if (indexVars.size() > 0) {
      value = IndexExpr::make(indexVars, value);
    }

    if (indexExpr->resultVars.size() == 0) {
      stmt = AssignStmt::make(var, value);
    }
    else {
      Expr varExpr = getVarExpr(var);

      std::vector<Expr> indices;
      for (IndexVar const& iv : indexExpr->resultVars) {
        Expr varExpr = getVarExpr(lvs->getVar(iv).first);
        indices.push_back(varExpr);
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

    std::vector<IndexVar> indexVars = GetFreeIndexVars().get(value);
    // TODO: Simplify by emitting empty IndexedTensors for scalar
    //       expressions (in visit(IndexedTensor)). Then we can remove the test
    //       and always turn value into an IndexExpr
    if (indexVars.size() > 0) {
      value = IndexExpr::make(indexVars, value);
    }

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

  void visit(const TensorWrite *op) {
    assert(isa<IndexExpr>(op->value) && "Can only specialize IndexExpr stmts");
    const IndexExpr *indexExpr = to<IndexExpr>(op->value);

    Expr value = mutate(op->value);

    if (isScalar(op->value.type())) {
      stmt = TensorWrite::make(op->tensor, op->indices, value);
    }
    else {
      std::vector<Expr> indices;
      for (IndexVar const& iv : indexExpr->resultVars) {
        Expr varExpr = getVarExpr(lvs->getVar(iv).first);
        indices.push_back(varExpr);
      }
      Expr tensor = TensorRead::make(op->tensor, op->indices);
      stmt = TensorWrite::make(tensor, indices, value);
    }
  }

  void visit(const IndexedTensor *op) {
    assert(!isa<IndexExpr>(op->tensor) &&
           "index expressions should have been lowered by now");

    if (op->indexVars.size() == 0) {
      expr = op->tensor;
    }
    else {
      std::vector<Expr> indices;
      for (const IndexVar &iv : op->indexVars) {
        Expr varExpr = getVarExpr(lvs->getVar(iv).first);
        indices.push_back(varExpr);
      }
      expr = TensorRead::make(op->tensor, indices);

      if (expr.type().toTensor()->order() > 0) {
        vector<IndexVar> ivs;
        for (auto &iv : op->indexVars) {
          if (indexVars.find(iv.getName()) == indexVars.end()) {
            vector<IndexSet> indexSets(iv.getDomain().getIndexSets().begin()+1,
                                       iv.getDomain().getIndexSets().end());
            IndexVar blockiv = IndexVar(iv.getName(), IndexDomain(indexSets),
                                        iv.getOperator());
            indexVars.insert(pair<string,IndexVar>(blockiv.getName(), blockiv));
          }
          ivs.push_back(indexVars.at(iv.getName()));
        }

        expr = IndexedTensor::make(expr, ivs);
      }
    }
  }

  void visit(const IndexExpr *op) {
    indexVars.clear();
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
    name += op->var.getName();
  }
};

/// Turns tensor writes into compound assignments (e.g. +=, *=)
/// \todo Generalize to include Assignments, FieldWrite, TupleWrite
class MakeCompound : public IRRewriter {
public:
  enum CompoundOperator { Add };

  MakeCompound(CompoundOperator compoundOperator)
      : compoundOperator(compoundOperator) {}

  Stmt mutate(Stmt stmt) {
    return IRRewriter::mutate(stmt);
  }

private:
  CompoundOperator compoundOperator;
  Expr lhsExpr;

  Expr mutate(Expr e) {
    assert(lhsExpr.defined());

    if (e.defined()) {
      if (!isScalar(e.type())) {
        e = IRRewriter::mutate(e);
      }
      else {
        switch (compoundOperator) {
          case Add:
            e = Add::make(lhsExpr, e);
            break;
        }
      }
    }
    else {
      e = Expr();
    }
    expr = Expr();
    stmt = Stmt();
    return e;
  }

  void visit(const TensorWrite *op) {
    lhsExpr = TensorRead::make(op->tensor, op->indices);

    vector<IndexVar> indexVars = GetFreeIndexVars().get(op->value);
    if (indexVars.size()) {
      lhsExpr = IndexedTensor::make(lhsExpr, indexVars);
    }

    Expr value = mutate(op->value);
    stmt = TensorWrite::make(op->tensor, op->indices, value);
  }
};

/// Rewrites rstmt to reduce it's computed value into a temporary reduction
/// variable using the rop ReductionOperation.
class ReduceOverVar : public IRRewriter {
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
      assert(isScalar(op->value.type()) &&
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
      if (isa<TensorRead>(tensor)) {
        tmpWriteStmt = MakeCompound(MakeCompound::Add).mutate(tmpWriteStmt);
      }
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


class Substitute : public IRRewriter {
public:
  Substitute(Expr oldExpr, Expr newExpr) {
    substitutions.insert(pair<Expr,Expr>(oldExpr, newExpr));
  }

  Substitute(map<Expr,Expr> substitutions) : substitutions(substitutions) {}

  Stmt mutate(Stmt stmt) {
    return IRRewriter::mutate(stmt);
  }

  Expr mutate(Expr expr) {
    if (substitutions.find(expr) != substitutions.end()) {
      return substitutions.at(expr);
    }
    else {
      return IRRewriter::mutate(expr);
    }
  }

private:
  map<Expr,Expr> substitutions;

  void visit(const VarExpr *op) {
    expr = op;
  }
};

// TODO: The if-based pattern matching in the visit rules is a total hack and
//       has to be rewritten.
class InlineMappedFunction : private IRRewriter {
public:
  InlineMappedFunction(Var lvar, Var resultActual, const Map *map,
                       Stmt computeStmt) {
    Func mapFunc = map->function;
    assert(mapFunc.getArguments().size() == 2 &&
           "mapped functions must have exactly two arguments");

    this->loopVar = lvar;

    this->targetSet = map->target;
    this->neighborSet = map->neighbors;

    this->resultActual = resultActual;

    this->targetElement = mapFunc.getArguments()[0];
    this->neighborElements = mapFunc.getArguments()[1];

    this->results = set<Var>(mapFunc.getResults().begin(),
                             mapFunc.getResults().end());

    this->computeStmt = computeStmt;
  }

  Stmt rewrite(Stmt s) {
    s = mutate(s);
    return s;
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
  map<Expr,Expr> substitutions;

  /// Turn argument field reads into loads from the buffer corresponding to that
  /// field
  void visit(const FieldRead *op) {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    if (isa<VarExpr>(op->elementOrSet) &&
        to<VarExpr>(op->elementOrSet)->var == targetElement) {
      Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
      expr = TensorRead::make(setFieldRead, {loopVar});
    }
    else if(isa<TupleRead>(op->elementOrSet) &&
            isa<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple) &&
            to<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple)->var ==neighborElements) {
      Expr setFieldRead = FieldRead::make(neighborSet, op->fieldName);
      expr = setFieldRead;

      Expr index = mutate(op->elementOrSet);
      expr = TensorRead::make(setFieldRead, {index});
    }
    else {
      NOT_SUPPORTED_YET;
    }
  }

  void visit(const TupleRead *op) {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = p(0); ... = tmp.x;
    if (isa<VarExpr>(op->tuple) &&
        to<VarExpr>(op->tuple)->var == neighborElements) {
      const TupleType *tupleType = op->tuple.type().toTuple();
      int cardinality = tupleType->size;

      Expr endpoints = IndexRead::make(targetSet, "endpoints");
      Expr indexExpr = Add::make(Mul::make(loopVar, cardinality), op->index);
      expr = Load::make(endpoints, indexExpr);
    }
    else {
      IRRewriter::visit(op);
    }
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

        stmt = flattenIndexExpressions(stmt);
        bool hasReduction = HasReduction().check(stmt);
        if (!hasReduction) {
          stmt = MakeCompound(MakeCompound::Add).mutate(stmt);
        }
        return;
      }
    }
    IRRewriter::visit(op);
  }
};


class ReplaceRhsWithZero : public IRRewriter {
  void visit(const AssignStmt *op) {
    stmt = AssignStmt::make(op->var, 0.0);
  }

  void visit(const FieldWrite *op) {
    // TODO: Value can't be a float if the field is an integer field
    stmt = FieldWrite::make(op->elementOrSet, op->fieldName, 0.0);
  }

  void visit(const TensorWrite *op) {
    stmt = TensorWrite::make(op->tensor, op->indices, 0.0);
  }
};

/// Retrieves the index expression computed in a given Stmt.  If no index
/// expressions are computed in the Stmt an undefined Expr is returned.
class GetIndexExpr : private IRVisitor {
public:
  const IndexExpr *get(Stmt stmt) {
    stmt.accept(this);
    return indexExpr;
  }

private:
  const IndexExpr *indexExpr;

  void visit(const IndexExpr *op) {
    indexExpr = op;
  }
};

class LowerIndexExpressions : public IRRewriter {
public:
  LowerIndexExpressions(const UseDef *ud) : ud(ud) {}

private:
  const UseDef *ud;

  /// Lower the index statement.  Defined after the LoopBuilder due to a
  /// circular dependency.
  Stmt lower(Stmt stmt);

  void visit(const AssignStmt *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(op);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const FieldWrite *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(op);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const TensorWrite *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(op);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const IndexExpr *op) {
    assert(false &&
           "IndexExprs must be assigned to a var/field/tensor before lowering");
  }
};

class LoopBuilder : public SIGVisitor {
public:
  LoopBuilder(const UseDef *ud) : ud(ud) {}

  Stmt create(Stmt computeStmt) {
    const IndexExpr *indexExpr = GetIndexExpr().get(computeStmt);

    SIG sig = SIGBuilder(ud).create(indexExpr);
    loopVars = LoopVars(sig, ud);

    initToZeroStmt = ReplaceRhsWithZero().mutate(computeStmt);


    // Create the loop body from the IndexExpr computeStmt
    loopBody = RemoveIndexExprs(&loopVars).mutate(computeStmt);
    std::vector<const SIGEdge *> edges = sig.getEdges();


    if (edges.size() > 1) {
      NOT_SUPPORTED_YET;
    }

    std::map<Var,const Map*> vars2maps;
    for (auto &e : edges) {
      VarDef varDef = ud->getDef(e->tensor);
      assert(varDef.getKind() == VarDef::Map);

      Var lvar = loopVars.getVar(e->tensor).first;
      const Map *map = to<Map>(varDef.getStmt());

      vars2maps[e->tensor] = map;

      Func func = map->function;

      // Inline the mapped function in the IndexExpr loop nests
      Stmt funcBody = func.getBody();

      loopBody = InlineMappedFunction(lvar, e->tensor, map,
                                      loopBody).rewrite(funcBody);

      // TODO: We should knock out redundant subexpressions in the loopBody
      //       before lowering the index expressions there
      loopBody = flattenIndexExpressions(loopBody);

      UseDef fud(func);
      loopBody = LowerIndexExpressions(&fud).mutate(loopBody);
    }

    stmt = loopBody;
    apply(sig);
    Stmt result = stmt;
    stmt = Stmt();

    return result;
  }

private:
  const UseDef *ud;
  LoopVars loopVars;
  Stmt initToZeroStmt;
  Stmt loopBody;

  Stmt stmt;

  std::vector<const SIGEdge *> edges;

  void visit(const SIGVertex *v) {
    pair<Var,ForDomain> loopVar = loopVars.getVar(v->iv);
    Var lvar = loopVar.first;
    ForDomain ldom = loopVar.second;

    if (v->iv.isFreeVar()) {
      stmt = For::make(lvar, ldom, stmt);
    }
    else {
      ReduceOverVar rov(loopBody, v->iv.getOperator());
      Stmt loopBody = rov.mutate(stmt);

      Var tmpVar = rov.getTmpVar();
      assert(tmpVar.defined());

      Stmt alloc = AssignStmt::make(tmpVar,Literal::make(tmpVar.getType(),{0}));
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
    edges.push_back(e);

    pair<Var,ForDomain> loopVar = loopVars.getVar(e->tensor);

    Var lvar = loopVar.first;
    ForDomain ldom = loopVar.second;

    VarDef varDef = ud->getDef(e->tensor);
    assert(varDef.getKind() == VarDef::Map);

    Stmt loop = For::make(lvar, ldom, loopBody);
    stmt = Block::make(initToZeroStmt, loop);

    for (auto &v : e->endpoints) {
      visitedVertices.insert(v);
    }
  }
};

Stmt LowerIndexExpressions::lower(Stmt stmt) {
  return LoopBuilder(ud).create(stmt);
}

Stmt lowerIndexExpressions(Stmt stmt, const UseDef &ud) {
  return LowerIndexExpressions(&ud).mutate(stmt);
}


class LowerMaps : public IRRewriter {
private:
  void visit(const Map *op) {
    // \todo We should only drop the map statements if it's bound Vars have
    // no uses (extend/invert UseDef to get DefUse info).
  }
};

Stmt lowerMaps(Stmt stmt) {
  return LowerMaps().mutate(stmt);
}


Expr createLengthComputation(const IndexSet &indexSet) {
  return Length::make(indexSet);
}

Expr createLengthComputation(const IndexDomain &dimensions) {
  assert(dimensions.getIndexSets().size() > 0);
  const vector<IndexSet> &indexSets = dimensions.getIndexSets();
  Expr len = createLengthComputation(indexSets[0]);
  for (size_t i=1; i < indexSets.size(); ++i) {
    len = Mul::make(len, createLengthComputation(indexSets[i]));
  }
  return len;
}

Expr createLengthComputation(const vector<IndexDomain> &dimensions) {
  assert(dimensions.size() > 0);
  Expr len = createLengthComputation(dimensions[0]);
  for (size_t i=1; i < dimensions.size(); ++i) {
    len = Mul::make(len, createLengthComputation(dimensions[i]));
  }
  return len;
}

Expr createLoadExpr(Expr tensor, Expr index) {
  // If the tensor is a load then we hada  nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    assert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->blockType();
    Expr len  = createLengthComputation(blockType.toTensor()->dimensions);

    index = Add::make(Mul::make(load->index, len), index);
    return Load::make(load->buffer, index);
  }
  else {
    return Load::make(tensor, index);
  }
}

Stmt createStoreStmt(Expr tensor, Expr index, Expr value) {
  // If the tensor is a load then we hada  nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    assert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->blockType();
    Expr len  = createLengthComputation(blockType.toTensor()->dimensions);

    index = Add::make(Mul::make(load->index, len), index);
    return Store::make(load->buffer, index, value);
  }
  else {
    return Store::make(tensor, index, value);
  }
}

class LowerTensorAccesses : public IRRewriter {
  void visit(const TensorRead *op) {
    assert(op->type.isTensor() && op->tensor.type().toTensor());

    const TensorType *type = op->tensor.type().toTensor();

    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    assert(op->indices.size() <= 2);

    Expr tensor = mutate(op->tensor);
    Expr index;
    if (op->indices.size() == 1) {
      index = mutate(op->indices[0]);
    }
    else if (op->indices.size() == 2) {
      // TODO: Clearly we need something more sophisticated here (for sparse
      // tensors or nested dense tensors).  For example, a tensor type could
      // carry a 'TensorStorage' object and we could ask this TensorStorage to
      // give us an Expr that computes an i,j location, or an Expr that gives us
      // a row/column.
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

      index = Add::make(Mul::make(i, d1), j);
    }
    else {
      NOT_SUPPORTED_YET;
    }
    assert(index.defined());

    expr = createLoadExpr(tensor, index);
  }

  void visit(const TensorWrite *op) {
    assert(op->tensor.type().isTensor());

    const TensorType *type = op->tensor.type().toTensor();

    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    assert(op->indices.size() <= 2);

    Expr tensor = mutate(op->tensor);
    Expr value = mutate(op->value);

    Expr index;
    if (op->indices.size() == 1) {
      index = mutate(op->indices[0]);
    }
    else if (op->indices.size() == 2) {
      // TODO: Clearly we need something more sophisticated here (for sparse
      // tensors or nested dense tensors).  For example, a tensor type could
      // carry a 'TensorStorage' object and we could ask this TensorStorage to
      // give us an Expr that computes an i,j location, or an Expr that gives us
      // a row/column.
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

      index = Add::make(Mul::make(i, d1), j);
    }
    else {
      NOT_SUPPORTED_YET;
    }
    assert(index.defined());

    stmt = createStoreStmt(tensor, index, value);
  }
};

Stmt lowerTensorAccesses(Stmt stmt) {
  return LowerTensorAccesses().mutate(stmt);
}

}}

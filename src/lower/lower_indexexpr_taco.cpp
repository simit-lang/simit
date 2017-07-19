#include "lower_indexexpr_taco.h"

#include "index_expressions/lower_index_expressions.h"
#include "intrinsics.h"
#include "ir_queries.h"
#include "ir_transforms.h"
#include "tensor_index.h"
#include "util/name_generator.h"

#include "taco/tensor.h"
#include "taco/ir/ir_visitor.h"
#include "taco/lower/lower.h"

using namespace std;
using namespace simit;
using namespace simit::ir;

namespace {

struct TacoException : public exception {};

template <typename T>
struct Hash {
  size_t operator()(const T& a) const {
    return (size_t) a.ptr;
  }
};

template <typename T, typename Func1, typename Func2, typename Callback>
static void flattenIndices(const vector<T>& inputs, const Func1& getSizeFunc,
    const Func2& getComponentsFunc, const Callback& callback) {
  typedef typename remove_reference<decltype(getComponentsFunc(declval<T>()))>::
      type::value_type ComponentType;

  size_t maxNest = 0;
  for (const auto& input : inputs) {
    maxNest = max(maxNest, getSizeFunc(input));
  }

  vector<vector<ComponentType>> componentss;
  vector<size_t> componentSizes;
  for (const auto& input : inputs) {
    componentss.push_back(vector<ComponentType>(maxNest));
    const auto& components = getComponentsFunc(input);
    componentSizes.push_back(components.size());
    auto i = componentss.back().rbegin();
    for (auto j = components.rbegin(); j != components.rend(); ++i, ++j) {
      *i = *j;
    }
  }

  for (size_t i = 0; i < maxNest; i++) {
    for (size_t j = 0; j < inputs.size(); j++) {
      if (maxNest - i <= componentSizes[j]) {
        callback(componentss[j][componentSizes[j] - (maxNest - i)]);
      }
    }
  }
}

class TacoLower : public IRVisitor, public taco::ir::IRVisitorStrict {
  util::NameGenerator nameGenerator;

  Storage* storage;
  unordered_map<string, Var> tensorNameMap; // map taco var to simit var by name
  unordered_map<Var, Expr, Hash<Var>> readTemporaries;
  unordered_map<Var, Expr, Hash<Var>> writeTemporaries;
  taco::TensorBase tacoAssignDest;

  //-------------------- Simit to Taco --------------------
  typedef unique_ptr<taco::IndexExpr> Ptr;
  Ptr tacoAccess;
  unordered_map<IndexVar, vector<taco::IndexVar>, Hash<IndexVar>> indexMap;
  // IndexVar.indexDomain unpacks to array of indexSets, traversed column first,
  // and mapped to taco::IndexVar

  taco::TensorBase getTacoVar(Var var) {
    taco::TensorBase tacoTensor;

    const TensorType* type = var.getType().toTensor();

    vector<int> dimensions;
    vector<taco::DimensionType> tacoSparsity;
    flattenIndices(type->getDimensions(),
        [](const IndexDomain& domain) { return domain.getNumIndexSets(); },
        [](const IndexDomain& domain) { return domain.getIndexSets(); },
        [&](const IndexSet& indexSet) {
          if (indexSet.getKind() == IndexSet::Range) {
            dimensions.push_back((int) indexSet.getSize());
            tacoSparsity.push_back(taco::Dense);
          } else {
            dimensions.push_back(1); // dummy dimension
            tacoSparsity.push_back(taco::Sparse);
          }
        });

    // Special case override
    if (tacoSparsity.size() > 0) {
      tacoSparsity[0] = taco::Dense;
    }

    // for example A = A + A is treated with 3 different tensors
    string uniqueVarName = nameGenerator.getName(var.getName());
    tacoTensor = taco::TensorBase(uniqueVarName, taco::ComponentType::Double,
                                  dimensions, taco::Format(tacoSparsity));
    tensorNameMap[uniqueVarName] = var;
    return tacoTensor;
  }

  pair<taco::TensorBase, vector<taco::IndexVar>>
  handleIndexedTensor(Var tensor, const vector<IndexVar>& indexVars) {
    vector<taco::IndexVar> tacoIndices;
    map<size_t, Expr> fixedExprs;
    size_t i = 0;
    flattenIndices(indexVars,
        [](const IndexVar& indexVar) {
          return indexVar.getDomain().getNumIndexSets();
        },
        [&](const IndexVar& indexVar) {
          vector<taco::IndexVar> results;
          auto it = indexMap.find(indexVar);
          if (indexVar.isFixed() || it == indexMap.end()) {
            for (const IndexSet& x : indexVar.getDomain().getIndexSets()) {
              results.push_back(taco::IndexVar(
                  nameGenerator.getName(indexVar.getName())));
            }
            if (!indexVar.isFixed()) {
              indexMap[indexVar] = results;
            }
          } else {
            results = it->second;
          }
          vector<pair<taco::IndexVar, IndexVar>> resultsWithIndexVar;
          for (const taco::IndexVar& tacoIndexVar : results) {
            resultsWithIndexVar.push_back(make_pair(tacoIndexVar, indexVar));
          }
          return resultsWithIndexVar;
        },
        [&](const pair<taco::IndexVar, IndexVar>& indexVar) {
          tacoIndices.push_back(indexVar.first);
          if (indexVar.second.isFixed()) {
            fixedExprs[i] = *indexVar.second.getFixedExpr();
          }
          i++;
        });

    taco::TensorBase tacoTensor = getTacoVar(tensor);
    return make_pair(tacoTensor, tacoIndices);
  }

  Ptr compile(Expr e) {
    if (isa<BinaryExpr>(e) || isa<UnaryExpr>(e) ||
        isa<IndexExpr>(e) || isa<IndexedTensor>(e)) {
      e.accept(this);
    } else { // Handle expr without indices
      Var var;
      if (isa<VarExpr>(e)) {
        var = to<VarExpr>(e)->var;
      } else {
        var = Var(nameGenerator.getName(), e.type());
        readTemporaries[var] = e;
      }
      taco::TensorBase tacoTensor = getTacoVar(var);
      tacoAccess = Ptr(new taco::Access(tacoTensor({})));
    }
    return move(tacoAccess);
  }

  virtual void visit(const IndexedTensor* op) {
    Var var;
    if (isa<VarExpr>(op->tensor)) {
      var = to<VarExpr>(op->tensor)->var;
    } else {
      var = Var(nameGenerator.getName(), op->tensor.type());
      readTemporaries[var] = op->tensor;
    }

    auto pair = handleIndexedTensor(var, op->indexVars);
    tacoAccess = Ptr(new taco::Access(pair.first(pair.second)));
  }

  virtual void visit(const IndexExpr* op) {
    tacoAccess = compile(op->value);
  }

  virtual void visit(const Neg* op) {
    Ptr a = compile(op->a);
    tacoAccess = Ptr(new taco::IndexExpr(-*a));
  }

  virtual void visit(const Add* op) {
    Ptr a = compile(op->a);
    Ptr b = compile(op->b);
    tacoAccess = Ptr(new taco::IndexExpr(*a + *b));
  }

  virtual void visit(const Sub* op) {
    Ptr a = compile(op->a);
    Ptr b = compile(op->b);
    tacoAccess = Ptr(new taco::IndexExpr(*a - *b));
  }

  virtual void visit(const Mul* op) {
    Ptr a = compile(op->a);
    Ptr b = compile(op->b);
    tacoAccess = Ptr(new taco::IndexExpr(*a * *b));
  }

  virtual void visit(const Div* op) {
    Ptr a = compile(op->a);
    Ptr b = compile(op->b);
    tacoAccess = Ptr(new taco::IndexExpr(*a / *b));;
  }

  virtual void visit(const AssignStmt* op) {
    vector<IndexVar> indices;
    if (isa<IndexExpr>(op->value)) {
      indices = to<IndexExpr>(op->value)->resultVars;
    }

    vector<taco::IndexVar> tacoIndices;
    tie(tacoAssignDest, tacoIndices) = handleIndexedTensor(op->var, indices);

    Ptr value = compile(op->value);

    switch (op->cop) {
    case CompoundOperator::None:
      tacoAssignDest(tacoIndices) = *value;
      break;
    case CompoundOperator::Add:
      tacoAssignDest(tacoIndices) = getTacoVar(op->var)(tacoIndices) + *value;
      break;
    case CompoundOperator::Sub:
      tacoAssignDest(tacoIndices) = getTacoVar(op->var)(tacoIndices) - *value;
      break;
    default:
      simit_unreachable;
    }
  }

  virtual void visit(const TensorWrite* op) {
    // pretend lhs to be a var
    Var var(nameGenerator.getName(), op->value.type());
    Expr lhs = TensorRead::make(op->tensor, op->indices);
    writeTemporaries[var] = lhs;
    AssignStmt::make(var, op->value, op->cop).accept(this);
  }

  virtual void visit(const FieldWrite* op) {
    // pretend lhs to be a var
    Var var(nameGenerator.getName(), op->value.type());
    Expr lhs = FieldRead::make(op->elementOrSet, op->fieldName);
    writeTemporaries[var] = lhs;
    AssignStmt::make(var, op->value, op->cop).accept(this);
  }

  //-------------------- Taco to Simit --------------------

  ScalarType componentType;

  Expr expr;
  Stmt stmt;
  vector<Stmt> spilledStmts;

  map<taco::ir::Expr, Var> tacoLocalVars; // taco ExprVar -> simit Var
  unordered_map<Var, Expr, Hash<Var>> loopIndexReplacement; // fixed IndexVar

  void spill(Stmt s) {
    spilledStmts.push_back(s);
  }

  Expr compile(taco::ir::Expr tacoExpr) {
    Expr e;
    if (tacoExpr.defined()) {
      tacoExpr.accept(this);
      e = expr;
    }
    expr = Expr();
    stmt = Stmt();
    return e;
  }

  Stmt compile(taco::ir::Stmt tacoStmt) {
    Stmt s;
    vector<Stmt> previousSpilledStmts;
    swap(spilledStmts, previousSpilledStmts);
    if (tacoStmt.defined()) {
      tacoStmt.accept(this);
      s = stmt;
    }
    expr = Expr();
    stmt = Stmt();

    if (spilledStmts.size()) {
      if (s.defined()) {
        spilledStmts.push_back(s);
      }
      s = Block::make(spilledStmts);
    }
    swap(spilledStmts, previousSpilledStmts);
    return s;
  }

  virtual void visit(const taco::ir::Literal* tacoOp) {
    if (tacoOp->type.isBool()) {
      expr = Literal::make((bool)tacoOp->value);
    } else if (tacoOp->type.isInt() || tacoOp->type.isUInt()) {
      expr = Literal::make((int)tacoOp->value);
    } else {
      simit_iassert(tacoOp->type.isFloat());
      expr = Literal::make(tacoOp->dbl_value);
    }
  }

  virtual void visit(const taco::ir::Var* tacoOp) {
    // tensor should be in GetProperty node
    simit_iassert(!tensorNameMap.count(tacoOp->name));

    Var var;
    auto it = tacoLocalVars.find(tacoOp);
    if (it != tacoLocalVars.end()) {
      var = it->second;
    } else {
      simit_iassert(!tacoOp->is_ptr && !tacoOp->is_tensor);

      string name = tacoOp->name;
      if (tacoOp->type.isBool()) {
        var = Var(name, Boolean);
      } else if (tacoOp->type.isInt() || tacoOp->type.isUInt()) {
        name = name.substr(0, name.rfind("_pos"));
        var = Var(name, Int);
      } else {
        simit_iassert(tacoOp->type.isFloat());
        var = Var(name, Float);
      }
      tacoLocalVars[tacoOp] = var;
    }
    expr = VarExpr::make(var);
    auto it2 = loopIndexReplacement.find(var);
    if (it2 != loopIndexReplacement.end()) {
      expr = it2->second;
    }
  }

  virtual void visit(const taco::ir::Neg* tacoOp) {
    Expr a = compile(tacoOp->a);
    expr = Neg::make(a);
  }

  virtual void visit(const taco::ir::Sqrt* tacoOp) {
    Expr a = compile(tacoOp->a);
    Var temp(nameGenerator.getName(), a.type());
    spill(CallStmt::make({temp}, intrinsics::sqrt(), {a}));
    expr = VarExpr::make(temp);
  }

  virtual void visit(const taco::ir::Add* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Add::make(a, b);
    if (isa<Literal>(a) && a.type() == Int && to<Literal>(a)->getIntVal(0) == 0) {
      expr = b;
    }
    if (isa<Literal>(b) && b.type() == Int && to<Literal>(b)->getIntVal(0) == 0) {
      expr = a;
    }
  }

  virtual void visit(const taco::ir::Sub* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Sub::make(a, b);
  }

  virtual void visit(const taco::ir::Mul* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Mul::make(a, b);
    if (isa<Literal>(a) && a.type() == Int && to<Literal>(a)->getIntVal(0) == 0
     || isa<Literal>(b) && b.type() == Int && to<Literal>(b)->getIntVal(0) == 0) {
      expr = 0;
    }
  }

  virtual void visit(const taco::ir::Div* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Div::make(a, b);
  }

  virtual void visit(const taco::ir::Rem* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Rem::make(a, b);
  }

  virtual void visit(const taco::ir::Min* tacoOp) {
    if (tacoOp->operands.size() == 1) {
      expr = compile(tacoOp->operands[0]);
    } else {
      Expr a = compile(tacoOp->operands[0]);
      Expr b = compile(tacoOp->operands[1]);
      Var temp(nameGenerator.getName(), a.type());
      spill(IfThenElse::make(Lt::make(a, b),
                             AssignStmt::make(temp, a),
                             AssignStmt::make(temp, b)));
      for (size_t i = 2; i < tacoOp->operands.size(); i++) {
        Expr c = compile(tacoOp->operands[i]);
        spill(IfThenElse::make(Gt::make(temp, c),
                               AssignStmt::make(temp,c)));
      }
      expr = VarExpr::make(temp);
    }
  }

  virtual void visit(const taco::ir::Max* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    Var temp(nameGenerator.getName(), a.type());
    spill(IfThenElse::make(Gt::make(a, b),
                           AssignStmt::make(temp, a),
                           AssignStmt::make(temp, b)));
    expr = VarExpr::make(temp);
  }

  virtual void visit(const taco::ir::BitAnd* tacoOp) {
    simit_unreachable; // Only exists in emitAssemble, not in emitCompute
  }

  virtual void visit(const taco::ir::Eq* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Eq::make(a, b);
  }

  virtual void visit(const taco::ir::Neq* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Ne::make(a, b);
  }

  virtual void visit(const taco::ir::Gt* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Gt::make(a, b);
  }

  virtual void visit(const taco::ir::Lt* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Lt::make(a, b);
  }

  virtual void visit(const taco::ir::Gte* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Ge::make(a, b);
  }

  virtual void visit(const taco::ir::Lte* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Le::make(a, b);
  }

  virtual void visit(const taco::ir::And* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = And::make(a, b);
  }

  virtual void visit(const taco::ir::Or* tacoOp) {
    Expr a = compile(tacoOp->a);
    Expr b = compile(tacoOp->b);
    expr = Or::make(a, b);
  }

  virtual void visit(const taco::ir::IfThenElse* tacoOp) {
    Expr cond = compile(tacoOp->cond);
    Stmt then = compile(tacoOp->then);
    if (tacoOp->otherwise.defined()) {
      Stmt otherwise = compile(tacoOp->otherwise);
      stmt = IfThenElse::make(cond, then, otherwise);
    } else {
      stmt = IfThenElse::make(cond, then);
    }
  }

  virtual void visit(const taco::ir::Case* tacoOp) {
    switch (tacoOp->clauses.size()) {
    case 0:
      stmt = Pass::make();
      break;
    case 1:
      stmt = IfThenElse::make(compile(tacoOp->clauses[0].first),
                              compile(tacoOp->clauses[0].second));
      break;
    default:
      Stmt s;
      auto it = tacoOp->clauses.rbegin();
      if (tacoOp->alwaysMatch) {
        s = compile(it->second);
      } else {
        s = IfThenElse::make(compile(it->first), compile(it->second));
      }
      while (++it != tacoOp->clauses.rend()) {
        s = IfThenElse::make(compile(it->first), compile(it->second), s);
      }
      stmt = s;
      break;
    }
  }

  Expr generateTensorRead(Expr buffer, Expr index) {
    simit_iassert(buffer.type().isTensor());
    const TensorType* type = buffer.type().toTensor();

    size_t maxNest = 0;
    for (const IndexDomain& domain : type->getDimensions()) {
      maxNest = max(maxNest, domain.getNumIndexSets());
    }

    vector<vector<IndexSet>> indexSetss;
    for (size_t i = 0; i < maxNest; i++) {
      indexSetss.push_back(vector<IndexSet>());
      for (const IndexDomain& domain : type->getDimensions()) {
        const vector<IndexSet>& indexSets = domain.getIndexSets();
        if (maxNest - i <= indexSets.size()) {
          indexSetss[i].push_back(indexSets[indexSets.size() - (maxNest - i)]);
        }
      }
    }

    simit_iassert(indexSetss.size() > 0);
    if (all_of(indexSetss[0].begin(), indexSetss[0].end(),
               [](const IndexSet& indexSet) {
                 return indexSet.getKind() == IndexSet::Set;
               }) && indexSetss.size() == 1) {
      return TensorRead::make(buffer, {index});
    }
    for (size_t i = 0; i < indexSetss.size() - 1; i++) {
      vector<Expr> indices(indexSetss[i].size(), 0);
      buffer = TensorRead::make(buffer, indices);
    }
    vector<Expr> indices(indexSetss.back().size() - 1, 0);
    indices.push_back(index);
    buffer = TensorRead::make(buffer, indices);
    return buffer;
  }

  virtual void visit(const taco::ir::Load* tacoOp) {
    Expr buffer = compile(tacoOp->arr);
    Expr index = compile(tacoOp->loc);

    if (isa<VarExpr>(buffer)) {
      Var tensor = to<VarExpr>(buffer)->var;

      auto it = readTemporaries.find(tensor);
      if (it != readTemporaries.end()) {
        buffer = it->second;
      }
    }

    if (isScalar(buffer.type())) {
      expr = buffer;
    } else {
      if (buffer.type().isTensor()) {
        expr = generateTensorRead(buffer, index);
      } else {
        expr = Load::make(buffer, index);
      }
    }
  }

  virtual void visit(const taco::ir::Store* tacoOp) {
    Expr buffer = compile(tacoOp->arr);
    Expr index = compile(tacoOp->loc);
    Expr value = compile(tacoOp->data);

    if (isScalar(buffer.type())) {
      if (isa<VarExpr>(buffer)) {
        stmt = AssignStmt::make(to<VarExpr>(buffer)->var, value);
      } else if (isa<TensorRead>(buffer)) {
        const TensorRead* t = to<TensorRead>(buffer);
        stmt = TensorWrite::make(t->tensor, t->indices, value);
      } else if (isa<FieldRead>(buffer)) {
        const FieldRead* f = to<FieldRead>(buffer);
        stmt = FieldWrite::make(f->elementOrSet, f->fieldName, value);
      } else {
        simit_unreachable;
      }
    } else {
      if (buffer.type().isTensor()) {
        Expr e = generateTensorRead(buffer, index);
        const TensorRead* tr = to<TensorRead>(e);
        stmt = TensorWrite::make(tr->tensor, tr->indices, value);
      } else {
        stmt = Store::make(buffer, index, value);
      }
    }
  }

  virtual void visit(const taco::ir::For* tacoOp) {
    simit_iassert(taco::ir::isa<taco::ir::Var>(tacoOp->var));

    Var loopVar(taco::ir::to<taco::ir::Var>(tacoOp->var)->name, Int);
    tacoLocalVars[taco::ir::to<taco::ir::Var>(tacoOp->var)] = loopVar;
    Expr start = compile(tacoOp->start);
    Expr end = compile(tacoOp->end);

    if (isa<Length>(end)) { // iterate over a Set
      Stmt body = compile(tacoOp->contents);
      stmt = For::make(loopVar, to<Length>(end)->indexSet, body);
    } else { // iterate over a constant-sized range
      Stmt body = compile(tacoOp->contents);
      const taco::ir::Literal* increment;
      if (taco::ir::isa<taco::ir::Literal>(tacoOp->increment) &&
          (increment = taco::ir::to<taco::ir::Literal>(tacoOp->increment),
           !increment->type.isFloat() && increment->value == 1)) {
        stmt = ForRange::make(loopVar, start, end, body);
      } else {
        Expr increment = compile(tacoOp->increment);
        spill(AssignStmt::make(loopVar, start));
        Expr cond = Lt::make(VarExpr::make(loopVar), end);
        body = Block::make(body, AssignStmt::make(loopVar,
                               Add::make(VarExpr::make(loopVar), increment)));
        stmt = While::make(cond, body);
      }
    }
  }

  virtual void visit(const taco::ir::While* tacoOp) {
    Expr cond = compile(tacoOp->cond);
    Stmt body = compile(tacoOp->contents);
    stmt = While::make(cond, body);
  }

  virtual void visit(const taco::ir::Block* tacoOp) {
    vector<Stmt> stmts;
    for (taco::ir::Stmt tacoStmt : tacoOp->contents) {
      stmts.push_back(compile(tacoStmt));
    }
    if (stmts.size() > 0) {
      stmt = Block::make(stmts);
    } else {
      stmt = Pass::make();
    }
  }

  virtual void visit(const taco::ir::Scope* tacoOp) {
    Stmt scopedStmt = compile(tacoOp->scopedStmt);
    stmt = Scope::make(scopedStmt);
  }

  virtual void visit(const taco::ir::Function* tacoOp) {
    stmt = compile(tacoOp->body);
  }

  virtual void visit(const taco::ir::VarAssign* tacoOp) {
    simit_iassert(taco::ir::isa<taco::ir::Var>(tacoOp->lhs));
    const taco::ir::Var* lhs = taco::ir::to<taco::ir::Var>(tacoOp->lhs);

    Var var;
    if (!tacoLocalVars.count(lhs) && lhs->type.isFloat()) {
      // must be accumulation var
      var = Var(lhs->name, TensorType::make(componentType));
      tacoLocalVars[lhs] = var;
    } else {
      Expr a = compile(lhs);
      simit_iassert(isa<VarExpr>(a));
      var = to<VarExpr>(compile(lhs))->var;
    }

    Expr value = compile(tacoOp->rhs);
    if (isa<Literal>(value) && var.getType() != value.type()) {
      const Literal* literal = to<Literal>(value);
      double real = 0;
      switch (literal->type.toTensor()->getComponentType().kind) {
      case ScalarType::Float:
        real = literal->getFloatVal(0);
        break;
      case ScalarType::Int:
        real = literal->getIntVal(0);
        break;
      case ScalarType::Boolean:
        real = ((const bool*)literal->data)[0];
        break;
      default:
        simit_unreachable;
      }

      switch (componentType.kind) {
      case ScalarType::Float:
        value = Literal::make(real);
        break;
      case ScalarType::Int:
        value = Literal::make((int) real);
        break;
      case ScalarType::Boolean:
        value = Literal::make((bool) real);
        break;
      case ScalarType::Complex:
        value = Literal::make(double_complex(real, 0));
        break;
      default:
        simit_unreachable;
      }
    }
    stmt = AssignStmt::make(var, value);
  }

  virtual void visit(const taco::ir::Allocate* tacoOp) {
    simit_unreachable;
  }

  virtual void visit(const taco::ir::Comment* tacoOp) {
    stmt = Pass::make();
  }

  virtual void visit(const taco::ir::BlankLine* tacoOp) {
    stmt = Pass::make();
  }

  virtual void visit(const taco::ir::Print* tacoOp) {
    simit_unreachable;
  }

  virtual void visit(const taco::ir::GetProperty* tacoOp) {
    simit_iassert(taco::ir::isa<taco::ir::Var>(tacoOp->tensor));
    Var tensor = tensorNameMap[taco::ir::to<taco::ir::Var>(tacoOp->tensor)->name];
    simit_iassert(tensor.defined());

    expr = VarExpr::make(tensor);
    auto it = writeTemporaries.find(tensor);
    if (it != writeTemporaries.end()) {
      expr = it->second;
    }

    switch (tacoOp->property) {
    case taco::ir::TensorProperty::Indices: {
      simit_iassert(tacoOp->dimension == 0 || tacoOp->dimension == 1);
      simit_iassert(storage->hasStorage(tensor));
      switch (tacoOp->index) {
      case 0:
        expr = VarExpr::make(storage->getStorage(tensor).getTensorIndex().
                             getRowptrArray());
        break;
      case 1:
        expr = VarExpr::make(storage->getStorage(tensor).getTensorIndex().
                             getColidxArray());
        break;
      default:
        not_supported_yet;
      }
    } break;
    case taco::ir::TensorProperty::Dimensions: {      
      simit_iassert(tacoOp->index == 0);
      vector<IndexSet> indexSets;
      vector<IndexDomain> domains = expr.type().toTensor()->getDimensions();
      flattenIndices(domains,
          [](const IndexDomain& domain) { return domain.getNumIndexSets(); },
          [](const IndexDomain& domain) { return domain.getIndexSets(); },
          [&](const IndexSet& indexSet) { indexSets.push_back(indexSet); } );

      if (indexSets[tacoOp->dimension].getKind() == IndexSet::Range) {
        expr = Literal::make((int) indexSets[tacoOp->dimension].getSize());
      } else {
        expr = Length::make(indexSets[tacoOp->dimension]);
      }
    } break;
    case taco::ir::TensorProperty::Values:
      break;
    default:
      simit_unreachable;
    }
  }

  // -------------------- real entry point --------------------
public:
  Stmt rewrite(Stmt stmt, ScalarType componentType, Storage* storage) {
    this->componentType = componentType;
    this->storage = storage;
    stmt.accept(this);
    taco::ir::Stmt tacoFunc;
    try {
      tacoFunc =
        taco::lower::lower(tacoAssignDest, "", {taco::lower::Compute});
    } catch (...) {
      throw TacoException();
    }
    Stmt s = compile(tacoFunc);

    if (spilledStmts.size()) {
      spilledStmts.push_back(s);
      s = Block::make(spilledStmts);
    }

    return s;
  }
};

}

// Any statement with indexVars that cannot form a partial-order graph by
// precedence in each IndexedTensor/IndexExpr is considered a transpose
// For example A(i,j) = B(j,k) * C(k,i) => i-j-k is cyclic
//             A(i,i) = B(j)            => i-i is cyclic
static bool isTranspose(Stmt stmt) {
  struct : public IRVisitor {
    vector<vector<IndexVar>> allIndexVars;

    virtual void visit(const IndexExpr *op) {
      allIndexVars.push_back(op->resultVars);
      IRVisitor::visit(op);
    }

    virtual void visit(const IndexedTensor* op) {
      allIndexVars.push_back(op->indexVars);
      IRVisitor::visit(op);
    }
  } getIndexVars;
  stmt.accept(&getIndexVars);

  for (const vector<IndexVar>& indexVars : getIndexVars.allIndexVars) {
    for (size_t i = 0; i < indexVars.size(); i++) {
      for (size_t j = i + 1; j < indexVars.size(); j++) {
        if (indexVars[i] == indexVars[j]) {
          return true;
        }
      }
    }
  }


  for (size_t i = 0; i < getIndexVars.allIndexVars.size(); i++) {
    for (size_t j = i + 1; j < getIndexVars.allIndexVars.size(); j++) {
      const vector<IndexVar>& a = getIndexVars.allIndexVars[i];
      const vector<IndexVar>& b = getIndexVars.allIndexVars[j];
      for (size_t k = 1; k < a.size(); k++) {
        auto pos1 = find(b.begin(), b.end(), a[k - 1]);
        auto pos2 = find(b.begin(), b.end(), a[k]);
        if (pos1 != b.end() && pos2 != b.end()) {
          if (pos1 > pos2) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

// Problems with write dependency
// For example A(i,j) = A(i,k) + B(k,j)
static bool isSelfAssignmentWithDifferentIndices(Stmt stmt) {
  struct : IRVisitor {
    bool value = false;
    Expr dest;
    vector<IndexVar> destIndices;

    virtual void visit(const IndexedTensor* op) {
      Expr src = op->tensor;
      if (destIndices == op->indexVars) {
        return;
      }
      if (isa<VarExpr>(dest)) {
        if (isa<VarExpr>(src) &&
            to<VarExpr>(dest)->var == to<VarExpr>(src)->var) {
          value = true;
        }
      } else if (isa<TensorRead>(dest)) {
        if (isa<TensorRead>(src) && getMainTensor(dest) == getMainTensor(src)) {
          const TensorRead* d = to<TensorRead>(dest);
          const TensorRead* s = to<TensorRead>(src);
          if (!all_of(d->indices.begin(), d->indices.end(),
                      (bool(&)(Expr))isa<Literal>) ||
              !all_of(s->indices.begin(), s->indices.end(),
                      (bool(&)(Expr))isa<Literal>) ||
              !equal(d->indices.begin(), d->indices.end(), s->indices.begin(),
                     [](const Expr d, const Expr s) {
                       simit_iassert(isa<Literal>(d) && isa<Literal>(s));
                       return *to<Literal>(d) == *to<Literal>(s);
                     })) {
            value = true;
          }
        }
      } else if (isa<FieldRead>(dest)) {
        if (isa<FieldRead>(src) && getMainTensor(dest) == getMainTensor(src) &&
            to<FieldRead>(src)->fieldName == to<FieldRead>(dest)->fieldName) {
          value = true;
        }
      }
    }

    virtual void visit(const AssignStmt* op) {
      dest = op->var;
      if (isa<IndexExpr>(op->value)) {
        destIndices = to<IndexExpr>(op->value)->resultVars;
        op->value.accept(this);
      }
    }

    virtual void visit(const TensorWrite* op) {
      dest = TensorRead::make(op->tensor, op->indices);
      if (isa<IndexExpr>(op->value)) {
        destIndices = to<IndexExpr>(op->value)->resultVars;
        op->value.accept(this);
      }
    }

    virtual void visit(const FieldWrite* op) {
      dest = FieldRead::make(op->elementOrSet, op->fieldName);
      if (isa<IndexExpr>(op->value)) {
        destIndices = to<IndexExpr>(op->value)->resultVars;
        op->value.accept(this);
      }
    }
  } visitor;
  stmt.accept(&visitor);
  return visitor.value;
}

static vector<bool> generateSparsity(Type t) {
  simit_iassert(t.isTensor());
  const TensorType* type = t.toTensor();
  vector<bool> sparsity; // true = sparse
  flattenIndices(type->getDimensions(),
      [](const IndexDomain& domain) { return domain.getNumIndexSets(); },
      [](const IndexDomain& domain) { return domain.getIndexSets(); },
      [&](const IndexSet& indexSet) {
        sparsity.push_back(indexSet.getKind() != IndexSet::Range);
      });
  if (sparsity.size() > 0) {
    sparsity[0] = false;
  }
  return sparsity;
}

// The same index var appears on different dimensions at different expressions,
// while the sparsities of these dimensions do not agree
// For example  A(i,j) = B(i,k)*B(k,j), where B is {Dense, Sparse}
static bool sparsityMismatch(Stmt stmt) {
  struct : public IRVisitor {
    vector<pair<Expr, vector<IndexVar>>> indexVars;

    virtual void visit(const IndexedTensor *op) {
      indexVars.push_back({op->tensor, op->indexVars});
    }
  } visitor;
  stmt.accept(&visitor);

  for (size_t i = 0; i < visitor.indexVars.size(); i++) {
    for (size_t j = i + 1; j < visitor.indexVars.size(); j++) {
      Expr a = visitor.indexVars[i].first;
      Expr b = visitor.indexVars[j].first;

      const vector<IndexVar>& aIndex = visitor.indexVars[i].second;
      const vector<IndexVar>& bIndex = visitor.indexVars[j].second;
      for (size_t k = 0; k < aIndex.size(); k++) {
        for (size_t l = 0; l < bIndex.size(); l++) {
          if (aIndex[k] == bIndex[l]) {
            const vector<bool>& sparseA = generateSparsity(a.type());
            const vector<bool>& sparseB = generateSparsity(b.type());
            if (sparseA[k] != sparseB[l]
                && aIndex.size() > 1 && bIndex.size() > 1) {
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

// Tensor has a sparse dimension outer than a dense dimension
// For example tensor[V,V](tensor[2,2](float))
static bool isSparseFollowedByDense(Stmt stmt) {
  struct : public IRVisitor {
    bool value = false;

    virtual void visit(const IndexedTensor *op) {
      const vector<bool>& sparsity = generateSparsity(op->tensor.type());
      static const vector<bool> dummy = {false};
      if (find(sparsity.begin(), sparsity.end(), true) <
          find_end(sparsity.begin(), sparsity.end(),
                   dummy.begin(), dummy.end())) {
        value = true;
      }
    }
  } visitor;
  stmt.accept(&visitor);
  return visitor.value;
}

// Tensor has a diagonal or stencil storage. There's no represenation in taco
static bool storageKindUnsupported(Stmt stmt, const Storage& storage) {
  struct CheckStorageKind : public IRVisitor {
    bool value = false;
    const Storage& storage;

    CheckStorageKind(const Storage& storage) : storage(storage) {}

    virtual void visit(const VarExpr* op) {
      if (storage.hasStorage(op->var)) {
        TensorStorage::Kind kind = storage.getStorage(op->var).getKind();
        if (kind != TensorStorage::Dense && kind != TensorStorage::Indexed) {
          value = true;
        }
      }
    }

    virtual void visit(const AssignStmt* op) {
      if (storage.hasStorage(op->var)) {
        TensorStorage::Kind kind = storage.getStorage(op->var).getKind();
        if (kind != TensorStorage::Dense && kind != TensorStorage::Indexed) {
          value = true;
        }
      }
      IRVisitor::visit(op);
    }
  } visitor(storage);
  stmt.accept(&visitor);
  return visitor.value;
}

// Tensor has fixed indexVar
static bool hasFixedIndexVar(Stmt stmt) {
  struct : public IRVisitor {
    bool value = false;

    virtual void visit(const IndexExpr* op) {
      for (const IndexVar& index : op->resultVars) {
        if (index.isFixed()) {
          value = true;
        }
      }
      IRVisitor::visit(op);
    }

    virtual void visit(const IndexedTensor* op) {
      for (const IndexVar& index : op->indexVars) {
        if (index.isFixed()) {
          value = true;
        }
      }
    }
  } visitor;
  stmt.accept(&visitor);
  return visitor.value;
}

// Division
static bool hasDivision(Stmt stmt) {
  struct : public IRVisitor {
    bool value = false;

    virtual void visit(const Div* op) {
      value = true;
    }

    virtual void visit(const Rem* op) {
      value = true;
    }
  } visitor;
  stmt.accept(&visitor);
  return visitor.value;
}


Func simit::ir::lowerIndexExprTaco(Func func) {
  class : public IRRewriter {
    Storage storage;
    Environment environment;

    // Let the old lowerIndexExpression pass haneld expressions taco can't
    Stmt lowerIndexExpression(Stmt s) {
      Func f("", {}, {}, s, environment);
      f.setStorage(storage);
      f = lowerIndexExpressions(f);
      s = f.getBody();
      s = removeVarDecls(s).first;
      storage = f.getStorage();
      return s;
    }

    void handleAssignLikeStmt(Stmt op, ScalarType componentType) {
      struct : public IRVisitor {
        bool value = false;

        virtual void visit(const IndexExpr* op) {
          value = true;
          IRVisitor::visit(op);
        }

        virtual void visit(const IndexedTensor* op) {
          value = true;
          IRVisitor::visit(op);
        }
      } hasIndexVar;

      op.accept(&hasIndexVar);
      if (hasIndexVar.value) {
        if (tacoCannotHandle(op)) {
          // taco can't handle transpose yet
          stmt = lowerIndexExpression(op);
        } else {
          try {
            stmt = TacoLower().rewrite(op, componentType, &storage);
          } catch (const TacoException& e) {
            std::cerr << "Warning: taco cannot handle (unknown reason)\n"
                      << util::toString(stmt) << "\n";
            stmt = lowerIndexExpression(op);
          }
          stmt = Comment::make(util::toString(op), stmt, false, true);
        }
      } else {
        stmt = op;
      }
    }

    virtual void visit(const AssignStmt* op) {
      handleAssignLikeStmt(op, op->value.type().toTensor()->getComponentType());
    }

    virtual void visit(const TensorWrite* op) {
      handleAssignLikeStmt(op, op->value.type().toTensor()->getComponentType());
    }

    virtual void visit(const FieldWrite* op) {
      handleAssignLikeStmt(op, op->value.type().toTensor()->getComponentType());
    }

    virtual void visit(const Func* op) {
      storage = op->getStorage();
      environment =op->getEnvironment();
      IRRewriter::visit(op);
      func.setStorage(storage);
    }


    bool tacoCannotHandle(Stmt stmt) {
#define CHECK(x, msg) if (x) {\
    std::cerr << "Warning: taco cannot handle " msg "\n"\
              << util::toString(stmt) << "\n";\
    return true;\
  }

      CHECK(isTranspose(stmt), "transpose");
      CHECK(isSelfAssignmentWithDifferentIndices(stmt),
            "self assignment with different indices");
      CHECK(sparsityMismatch(stmt),
            "same index var but different sparsity");
      CHECK(isSparseFollowedByDense(stmt),
            "sparse dimension followed by dense dimension");
      CHECK(storageKindUnsupported(stmt, storage),
            "diagonal or stencil storage");
      CHECK(hasFixedIndexVar(stmt), "fixed index expression");
      CHECK(hasDivision(stmt), "division");
      return false;
    }
  } rewriter;

  func = rewriter.rewrite(func);
  func = insertVarDecls(func);
  func = insertInitializations(func); /// @todo needed due to bugs in taco
  return func;
}

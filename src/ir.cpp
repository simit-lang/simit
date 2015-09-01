#include "ir.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util.h"
#include "macros.h"
#include "ir_printer.h"
#include "util/compare.h"
#include "util/arrays.h"

using namespace std;

namespace simit {
namespace ir {

// class Expr
Expr::Expr(const Var &var) : Expr(VarExpr::make(var)) {}

Expr::Expr(int val) : IRHandle(Literal::make(val)) {
}

Expr::Expr(double val) : IRHandle(Literal::make(val)) {
}

Expr Expr::operator()(const std::vector<IndexVar> &indexVars) const {
  return IndexedTensor::make(*this, indexVars);
}

Expr operator-(Expr a) {
  return Neg::make(a);
}

Expr operator+(Expr a, Expr b) {
  return Add::make(a, b);
}

Expr operator-(Expr a, Expr b) {
  return Sub::make(a, b);
}

Expr operator*(Expr a, Expr b) {
  return Mul::make(a, b);
}

Expr operator/(Expr a, Expr b) {
  return Div::make(a, b);
}

// Type compute functions
Type getFieldType(Expr elementOrSet, std::string fieldName) {
  iassert(elementOrSet.type().isElement() || elementOrSet.type().isSet());

  Type fieldType;
  if (elementOrSet.type().isElement()) {
    const ElementType *elemType = elementOrSet.type().toElement();
    fieldType = elemType->field(fieldName).type;
  }
  else if (elementOrSet.type().isSet()) {
    const SetType *setType = elementOrSet.type().toSet();
    const ElementType *elemType = setType->elementType.toElement();

    const TensorType *elemFieldType= elemType->field(fieldName).type.toTensor();

    // The type of a set field is:
    // `tensor[set](tensor[elementFieldDimensions](elemFieldComponentType))`
    vector<IndexDomain> dimensions;
    if (elemFieldType->order() == 0) {
      dimensions.push_back(IndexDomain(IndexSet(elementOrSet)));
    }
    else {
      unsigned order = elemFieldType->order();
      dimensions = vector<IndexDomain>(order);
      dimensions[0] = IndexDomain(IndexSet(elementOrSet));

      vector<IndexDomain> elemFieldDimensions = elemFieldType->getDimensions();
      for (size_t i=0; i < order; ++i) {
        dimensions[i] = dimensions[i] * elemFieldDimensions[i];
      }
    }
    fieldType = TensorType::make(elemFieldType->componentType, dimensions);
  }
  return fieldType;
}

Type getBlockType(Expr tensor) {
  iassert(tensor.type().isTensor());
  return tensor.type().toTensor()->getBlockType();
}

Type getIndexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr) {
  iassert(isScalar(expr.type()));
  std::vector<IndexDomain> dimensions;
  for (auto &indexVar : lhsIndexVars) {
    dimensions.push_back(indexVar.getDomain());
  }
  return TensorType::make(expr.type().toTensor()->componentType, dimensions);
}

// struct Literal
void Literal::cast(Type type) {
  iassert(type.isTensor());
  iassert(type.toTensor()->componentType==this->type.toTensor()->componentType);
  iassert(type.toTensor()->size() == this->type.toTensor()->size());
  this->type = type;
}

double Literal::getFloatVal(int index) const {
  return ((double*)data)[index];
}

Expr Literal::make(Type type) {
  return Literal::make(type, nullptr);
}

Expr Literal::make(int val) {
  return make(Int, &val);
}

Expr Literal::make(double val) {
  // Choose appropriate precision
  if (ScalarType::singleFloat()) {
    float floatVal = (float) val;
    return make(Float, &floatVal);
  }
  else {
    return make(Float, &val);
  }
}

Expr Literal::make(bool val) {
  return make(Boolean, &val);
}

Expr Literal::make(Type type, void* values) {
  iassert(type.isTensor()) << "only tensor literals are supported for now";
  const TensorType *ttype = type.toTensor();

  size_t size = 0;
  size_t sizeInBytes = 0;
  switch (type.kind()) {
    case Type::Tensor: {
      size = ttype->size();
      sizeInBytes = size * ttype->componentType.bytes();
      break;
    }
    case Type::Set:
    case Type::Element:
    case Type::Tuple:
    case Type::Array:
      iassert(false) << "Only tensor and scalar literals currently supported";
      break;
  }

  Literal *node = new Literal;
  node->type = type;
  node->size = sizeInBytes;
  node->data = malloc(node->size);
  if (values != nullptr) {
    memcpy(node->data, values, node->size);
  }
  else {
    // Zero array
    switch (ttype->componentType.kind) {
      case ir::ScalarType::Boolean:
        util::zero<bool>(node->data, size);
        break;
      case ir::ScalarType::Int:
        util::zero<int>(node->data, size);
        break;
      case ir::ScalarType::Float:
        if (ir::ScalarType::singleFloat()) {
          iassert(ir::ScalarType::floatBytes == sizeof(float));
          util::zero<float>(node->data, size);
        }
        else {
          iassert(ir::ScalarType::floatBytes == sizeof(double));
          util::zero<double>(node->data, size);
        }
        break;
    }
  }
  return node;
}

Expr Literal::make(Type type, std::vector<double> values) {
  iassert(isScalar(type) || type.toTensor()->size() == values.size());
  iassert(type.toTensor()->componentType.kind == ScalarType::Float)
      << "Float array constructor must use float component type";
  if (ScalarType::singleFloat()) {
    // Convert double vector to float vector
    std::vector<float> floatValues;
    for (double val : values) {
      floatValues.push_back(val);
    }
    return Literal::make(type, floatValues.data());
  }
  else {
    return Literal::make(type, values.data());
  }
}

Literal::~Literal() {
  free(data);
}

inline size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->componentType.bytes();
}

bool operator==(const Literal& l, const Literal& r) {
  iassert(l.type.isTensor() && r.type.isTensor());

  if (l.type != r.type) {
    return false;
  }

  iassert(getTensorByteSize(l.type.toTensor()) ==
          getTensorByteSize(r.type.toTensor()));

  size_t size = l.type.toTensor()->size();
  switch (l.type.toTensor()->componentType.kind) {
    case ir::ScalarType::Int: {
      return util::compare<int>(l.data, r.data, size);
    }
    case ir::ScalarType::Float: {
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        return util::compare<float>(l.data, r.data, size);
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        return util::compare<double>(l.data, r.data, size);
      }
    }
    case ir::ScalarType::Boolean: {
      return util::compare<bool>(l.data, r.data, size);
    }
  }
  return true;
}

bool operator!=(const Literal& l, const Literal& r) {
  return !(l == r);
}

// struct VarExpr
Expr VarExpr::make(Var var) {
  VarExpr *node = new VarExpr;
  node->type = var.getType();
  node->var = var;
  return node;
}

// struct Load
Expr Load::make(Expr buffer, Expr index) {
  iassert(isScalar(index.type()));

  Load  *node = new Load;

  // TODO: Temporary handle loading from TensorType (should only support arrays)
  ScalarType loadType = (buffer.type().isTensor())
                        ? buffer.type().toTensor()->componentType
                        : buffer.type().toArray()->elementType;

  node->type = TensorType::make(loadType);
  node->buffer = buffer;
  node->index = index;
  return node;
}

// struct FieldRead
Expr FieldRead::make(Expr elementOrSet, std::string fieldName) {
  iassert(elementOrSet.type().isElement() || elementOrSet.type().isSet());
  FieldRead *node = new FieldRead;
  node->type = getFieldType(elementOrSet, fieldName);
  node->elementOrSet = elementOrSet;
  node->fieldName = fieldName;
  return node;
}

// struct Call
Expr Call::make(Func func, std::vector<Expr> actuals) {
  iassert(func.getResults().size() == 1)
      << "only calls of function with one results is currently supported.";
  Call *node = new Call;
  node->type = func.getResults()[0].getType();
  node->func = func;
  node->actuals = actuals;
  return node;
}

// struct Length
Expr Length::make(IndexSet indexSet) {
  Length *node = new Length;
  node->type = TensorType::make(ScalarType(ScalarType::Int));
  node->indexSet = indexSet;
  return node;
}

// struct IndexRead
Expr IndexRead::make(Expr edgeSet, Kind kind) {
  iassert(edgeSet.type().isSet());

  IndexRead *node = new IndexRead;
  node->type = TensorType::make(ScalarType(ScalarType::Int),
                                {IndexDomain(IndexSet(edgeSet))});
  node->edgeSet = edgeSet;
  node->kind = kind;
  return node;
}

// struct Neg
Expr Neg::make(Expr a) {
  iassert_scalar(a);

  Neg *node = new Neg;
  node->type = a.type();
  node->a = a;
  return node;
}

// struct Add
Expr Add::make(Expr a, Expr b) {
  iassert_scalar(a);
  iassert_types_equal(a,b);

  Add *node = new Add;
  node->type = a.type();
  node->a = a;
  node->b = b;
  return node;
}

// struct Sub
Expr Sub::make(Expr a, Expr b) {
  iassert_scalar(a);
  iassert_types_equal(a,b);

  Sub *node = new Sub;
  node->type = a.type();
  node->a = a;
  node->b = b;
  return node;
}

// struct Mul
Expr Mul::make(Expr a, Expr b) {
  iassert_scalar(a);
  iassert_types_equal(a,b);

  Mul *node = new Mul;
  node->type = a.type();
  node->a = a;
  node->b = b;
  return node;
}

// struct Div
Expr Div::make(Expr a, Expr b) {
  iassert_scalar(a);
  iassert_types_equal(a,b);

  Div *node = new Div;
  node->type = a.type();
  node->a = a;
  node->b = b;
  return node;
}

// struct Not
Expr Not::make(Expr a) {
  iassert_boolean_scalar(a);

  Not *node = new Not;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  return node;
}

// struct Eq
Expr Eq::make(Expr a, Expr b) {
  iassert_types_equal(a,b);

  Eq *node = new Eq;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Ne
Expr Ne::make(Expr a, Expr b) {
  iassert_types_equal(a,b);

  Ne *node = new Ne;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Gt
Expr Gt::make(Expr a, Expr b) {
  iassert_types_equal(a,b);

  Gt *node = new Gt;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Lt
Expr Lt::make(Expr a, Expr b) {
  iassert_types_equal(a,b);

  Lt *node = new Lt;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Ge
Expr Ge::make(Expr a, Expr b) {
  iassert_types_equal(a,b);

  Ge *node = new Ge;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Le
Expr Le::make(Expr a, Expr b) {
  iassert_types_equal(a,b);

  Le *node = new Le;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct And
Expr And::make(Expr a, Expr b) {
  iassert_boolean_scalar(a);
  iassert_boolean_scalar(b);

  And *node = new And;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Or
Expr Or::make(Expr a, Expr b) {
  iassert_boolean_scalar(a);
  iassert_boolean_scalar(b);

  Or *node = new Or;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct Xor
Expr Xor::make(Expr a, Expr b) {
  iassert_boolean_scalar(a);
  iassert_boolean_scalar(b);

  Xor *node = new Xor;
  node->type = TensorType::make(ScalarType::Boolean);
  node->a = a;
  node->b = b;
  return node;
}

// struct VarDecl
Stmt VarDecl::make(Var var) {
  VarDecl *node = new VarDecl;
  node->var = var;
  return node;
}

// struct AssignStmt
Stmt AssignStmt::make(Var var, Expr value) {
  return make(var, value, CompoundOperator::None);
}

Stmt AssignStmt::make(Var var, Expr value, CompoundOperator cop) {
  AssignStmt *node = new AssignStmt;
  node->var = var;
  node->value = value;
  node->cop = cop;
  return node;
}

// struct Store
Stmt Store::make(Expr buffer, Expr index, Expr value, CompoundOperator cop) {
  Store *node = new Store;
  node->buffer = buffer;
  node->index = index;
  node->value = value;
  node->cop = cop;
  return node;
}

// struct FieldWrite
Stmt FieldWrite::make(Expr elementOrSet, std::string fieldName, Expr value,
                      CompoundOperator cop) {
  FieldWrite *node = new FieldWrite;
  node->elementOrSet = elementOrSet;
  node->fieldName = fieldName;
  node->value = value;
  node->cop = cop;
  return node;
}

// struct CallStmt
Stmt CallStmt::make(std::vector<Var> results,
                    Func callee, std::vector<Expr> actuals) {
  CallStmt *node = new CallStmt;
  node->results = results;
  node->callee = callee;
  node->actuals = actuals;
  return node;
}

// struct Block
Stmt Block::make(Stmt first, Stmt rest, bool scoped) {
  iassert(first.defined() || rest.defined()) << "Empty block";

  // Handle case where first is undefined, to ease codegen in loops
  if (!first.defined()) {
    std::swap(first,rest);
  }

  Block *node = new Block;
  node->first = first;
  node->rest = rest;
  node->scoped = scoped;
  return node;
}

Stmt Block::make(std::vector<Stmt> stmts, bool scoped) {
  iassert(stmts.size() > 0) << "Empty block";
  Stmt node;
  for (size_t i=stmts.size(); i>1; --i) {
    node = Block::make(stmts[i-1], node, false);
  }
  node = Block::make(stmts[0], node, scoped);
  return node;
}

// struct IfThenElse
Stmt IfThenElse::make(Expr condition, Stmt thenBody) {
  IfThenElse *node = new IfThenElse;
  node->condition = condition;
  node->thenBody = Block::make({thenBody}, true);
  return node;
}

Stmt IfThenElse::make(Expr condition, Stmt thenBody, Stmt elseBody) {
  IfThenElse *node = new IfThenElse;
  node->condition = condition;
  node->thenBody = Block::make({thenBody}, true);
  node->elseBody = Block::make({elseBody}, true);
  return node;
}

// struct ForRange
Stmt ForRange::make(Var var, Expr start, Expr end, Stmt body) {
  ForRange *node = new ForRange;
  node->var = var;
  node->start = start;
  node->end = end;
  node->body = Block::make({body}, true);
  return node;
}

// struct For
Stmt For::make(Var var, ForDomain domain, Stmt body) {
  For *node = new For;
  node->var = var;
  node->domain = domain;
  node->body = Block::make({body}, true);
  return node;
}

// struct While
Stmt While::make(Expr condition, Stmt body) {
  While *node = new While;
  node->condition = condition;
  node->body = Block::make({body}, true);
  return node;
}

// struct Kernel
Stmt Kernel::make(Var var, IndexDomain domain, Stmt body) {
  Kernel *node = new Kernel;
  node->var = var;
  node->domain = domain;
  node->body = body;
  return node;
}

// struct Print
Stmt Print::make(Expr expr) {
  Print *node = new Print;
  node->expr = expr;
  return node;
}

// struct Comment
Stmt Comment::make(std::string comment, Stmt commentedStmt,
                   bool footerSpace, bool headerSpace){
  Comment *node = new Comment;
  node->comment = comment;
  node->commentedStmt = commentedStmt;
  node->footerSpace = footerSpace;
  node->headerSpace = headerSpace;
  return node;
}

// struct Pass
Stmt Pass::make() {
  Pass *node = new Pass;
  return node;
}

// struct TupleRead
Expr TupleRead::make(Expr tuple, Expr index) {
  iassert(tuple.type().isTuple());
  TupleRead *node = new TupleRead;
  node->type = tuple.type().toTuple()->elementType;
  node->tuple = tuple;
  node->index = index;
  return node;
}

// struct TensorRead
Expr TensorRead::make(Expr tensor, std::vector<Expr> indices) {
  iassert(tensor.type().isTensor());
#ifdef SIMIT_ASSERTS
  for (auto &index : indices) {
    iassert(isScalar(index.type()) || index.type().isElement());
  }
#endif
  iassert(indices.size() == 1 ||
          indices.size() == tensor.type().toTensor()->order());

  TensorRead *node = new TensorRead;
  node->type = getBlockType(tensor);
  node->tensor = tensor;
  node->indices = indices;
  return node;
}

// struct TensorWrite
Stmt TensorWrite::make(Expr tensor, std::vector<Expr> indices, Expr value,
                       CompoundOperator cop) {
  TensorWrite *node = new TensorWrite;
  node->tensor = tensor;
  node->indices = indices;
  node->value = value;
  node->cop = cop;
  return node;
}

// struct IndexedTensor
Expr IndexedTensor::make(Expr tensor, std::vector<IndexVar> indexVars) {
#ifdef SIMIT_ASSERTS
  iassert(tensor.type().isTensor()) << "Only tensors can be indexed.";
  iassert(indexVars.size() == tensor.type().toTensor()->order());
  std::vector<IndexDomain> dimensions =
  tensor.type().toTensor()->getDimensions();
  for (size_t i=0; i < indexVars.size(); ++i) {
    iassert(indexVars[i].getDomain() == dimensions[i])
    << "IndexVar domain does not match tensor dimension "
    << "for var " << indexVars[i]
    << indexVars[i].getDomain() << " != " << dimensions[i];
  }
#endif

  IndexedTensor *node = new IndexedTensor;
  node->type = TensorType::make(tensor.type().toTensor()->componentType);
  node->tensor = tensor;
  node->indexVars = indexVars;
  return node;
}

// struct IndexExpr
std::vector<IndexVar> IndexExpr::domain() const {
  class DomainGatherer : private IRVisitor {
  public:
    vector<IndexVar> getDomain(const IndexExpr &indexExpr) {
      domain.clear();
      added.clear();
      add(indexExpr.resultVars);
      indexExpr.value.accept(this);
      return domain;
    }

  private:
    vector<IndexVar> domain;
    set<string> added;

    void add(const vector<IndexVar> &indexVars) {
      for (const IndexVar &ivar : indexVars) {
        if (added.find(ivar.getName()) == added.end()) {
          added.insert(ivar.getName());
          domain.push_back(ivar);
        }
      }
    }

    using IRVisitor::visit;

    void visit(const IndexedTensor *op) {
      add(op->indexVars);
    }
  };

  return DomainGatherer().getDomain(*this);
}

Expr IndexExpr::make(std::vector<IndexVar> resultVars, Expr value) {
  iassert(isScalar(value.type())) << value << " : " << value.type();
#ifdef SIMIT_ASSERTS
  for (auto &idxVar : resultVars) {  // No reduction variables on lhs
    iassert(idxVar.isFreeVar());
  }
#endif

  IndexExpr *node = new IndexExpr;
  node->type = getIndexExprType(resultVars, value);
  node->resultVars = resultVars;
  node->value = value;
  return node;
}

// struct Map
Stmt Map::make(std::vector<Var> vars,
               Func function, std::vector<Expr> partial_actuals,
               Expr target, Expr neighbors,
               ReductionOperator reduction) {
  iassert(target.type().isSet());
  iassert(!neighbors.defined() || neighbors.type().isSet());
  iassert(vars.size() == function.getResults().size());
  Map *node = new Map;
  node->vars = vars;
  node->function = function;
  node->partial_actuals = partial_actuals;
  node->target = target;
  node->neighbors = neighbors;
  node->reduction = reduction;
  return node;
}

}}

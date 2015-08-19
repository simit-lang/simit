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

using namespace std;

namespace simit {
namespace ir {

// class Expr
Expr::Expr(const Var &var) : Expr(VarExpr::make(var)) {}

static size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->componentType.bytes();
}

// class Expr
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


// class Intrinsics
Func Intrinsics::mod = Func("mod",
                            {Var("x", Int), Var("y", Int)},
                            {Var("r", Int)},
                            Func::Intrinsic);

Func Intrinsics::sin = Func("sin",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::cos = Func("cos",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::tan = Func("tan",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::asin = Func("asin",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);
  
Func Intrinsics::acos = Func("acos",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);
  
Func Intrinsics::atan2 = Func("atan2",
                              {Var("y", Float), Var("x", Float)},
                              {Var("r", Float)},
                              Func::Intrinsic);

Func Intrinsics::sqrt = Func("sqrt",
                             {Var("x", Float)},
                             {Var("r", Float)},
                             Func::Intrinsic);

Func Intrinsics::log = Func("log",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::exp = Func("exp",
                            {Var("x", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);
  
Func Intrinsics::pow = Func("pow",
                            {Var("x", Float), Var("y", Float)},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::norm = Func("norm",
                             {},
                             {Var("r", Float)},
                             Func::Intrinsic);

Func Intrinsics::dot = Func("dot",
                            {},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::det = Func("det",
                            {},
                            {Var("r", Float)},
                            Func::Intrinsic);

Func Intrinsics::inv = Func("inv",
                            {},
                            {Var("r", mat3f)},
                            Func::Intrinsic);

Func Intrinsics::solve = Func("solve",
                              {},
                              {Var("r", Float)},
                              Func::Intrinsic);

Func Intrinsics::loc = Func("loc",
                            {},
                            {Var("r", Int)},
                            Func::Intrinsic);

std::map<std::string, Func> Intrinsics::byName = {{"mod",mod},
                                                  {"sin",sin},
                                                  {"cos",cos},
                                                  {"tan",tan},
                                                  {"asin",asin},
                                                  {"acos",acos},
                                                  {"atan2",atan2},
                                                  {"sqrt",sqrt},
                                                  {"log",log},
                                                  {"exp",exp},
                                                  {"pow",pow},
                                                  {"norm",norm},
                                                  {"dot",dot},
                                                  {"det",det},
                                                  {"inv",inv},
                                                  {"solve",solve}};

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

// struct IndexStmt
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

std::vector<IndexVar> IndexExpr::domain() const {
  return DomainGatherer().getDomain(*this);
}

}} // namespace simit::ir

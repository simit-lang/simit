#include "ir.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util.h"
#include "macros.h"
#include "ir_printer.h"

using namespace std;

namespace simit {
namespace ir {

// class Expr
Expr::Expr(const Var &var) : Expr(VarExpr::make(var)) {}

static size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->componentType.bytes();
}

// class Expr
Expr::Expr(int val) : IntrusivePtr(Literal::make(Int(), &val)) {
}

Expr::Expr(double val) : IntrusivePtr(Literal::make(Float(), &val)) {
}

bool operator==(const Expr &l, const Expr &r) {
  return l.expr() == r.expr();
}

bool operator!=(const Expr &l, const Expr &r) {
  return !(l == r);
}

// class Intrinsics
Func Intrinsics::sin =
    Func("sin", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::cos =
    Func("cos", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::atan2 =
    Func("atan2", {Var("y", Float()), Var("x", Float())},
                  {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::sqrt =
    Func("sqrt", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::log =
    Func("log", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::exp =
    Func("exp", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);

Func Intrinsics::solve = Func("expr", {}, {Var("r", Float())}, Func::Intrinsic);

std::map<std::string, Func> Intrinsics::byName = {{"sin",sin},
                                                  {"cos",cos},
                                                  {"atan2",atan2},
                                                  {"sqrt",sqrt},
                                                  {"log",log},
                                                  {"exp",exp},
                                                  {"solve",solve}};

// Type compute functions
Type getFieldType(Expr elementOrSet, std::string fieldName) {
  assert(elementOrSet.type().isElement() || elementOrSet.type().isSet());

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
    std::vector<IndexDomain> dimensions;
    if (elemFieldType->order() == 0) {
      dimensions.push_back(IndexDomain(IndexSet(elementOrSet)));
    }
    else {
      unsigned order = elemFieldType->order();
      dimensions = vector<IndexDomain>(order);
      dimensions[0] = IndexDomain(IndexSet(elementOrSet));

      for (size_t i=0; i < order; ++i) {
        dimensions[i] = dimensions[i] * elemFieldType->dimensions[i];
      }
    }
    fieldType = TensorType::make(elemFieldType->componentType, dimensions);
  }
  return fieldType;
}

Type getBlockType(Expr tensor) {
  assert(tensor.type().isTensor());
  return tensor.type().toTensor()->blockType();
}

Type getIndexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr) {
  assert(isScalarTensor(expr.type()));
  std::vector<IndexDomain> dimensions;
  for (auto &indexVar : lhsIndexVars) {
    dimensions.push_back(indexVar.getDomain());
  }
  return TensorType::make(expr.type().toTensor()->componentType, dimensions);
}

// struct Literal
void Literal::cast(Type type) {
  assert(type.isTensor());
  const TensorType *newType = type.toTensor();
  const TensorType *oldType = this->type.toTensor();
  assert(newType->componentType == oldType->componentType);
  assert(newType->size() == oldType->size());

  this->type = type;
}

bool operator==(const Literal& l, const Literal& r) {
  assert(l.type.isTensor() && r.type.isTensor());

  if (l.type != r.type) {
    return false;
  }

  assert(getTensorByteSize(l.type.toTensor()) ==
         getTensorByteSize(r.type.toTensor()));
  size_t tensorDataSize = getTensorByteSize(l.type.toTensor());

  if (memcmp(l.data, r.data, tensorDataSize) != 0) {
    return false;
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

  void visit(const IndexedTensor *op) {
    add(op->indexVars);
  }
};

std::vector<IndexVar> IndexExpr::domain() const {
  return DomainGatherer().getDomain(*this);
}

}} // namespace simit::internal

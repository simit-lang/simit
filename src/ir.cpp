#include "ir.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util/util.h"
#include "macros.h"
#include "ir_printer.h"
#include "util/compare.h"
#include "util/arrays.h"

using namespace std;

namespace simit {
namespace ir {

// class IRNode
std::ostream &operator<<(std::ostream &os, const IRNode &node) {
  IRPrinter printer(os);
  printer.print(node);
  return os;
}

// class Expr
Expr::Expr(const Var &var) : Expr(VarExpr::make(var)) {}

Expr::Expr(int val) : IRHandle(Literal::make(val)) {}

Expr::Expr(double val) : IRHandle(Literal::make(val)) {}

Expr::Expr(double_complex val) : IRHandle(Literal::make(val)) {}

Expr Expr::operator()(const std::vector<IndexVar> &indexVars) const {
  return IndexedTensor::make(*this, indexVars);
}

std::ostream &operator<<(std::ostream &os, const Expr &expr) {
  IRPrinter printer(os);
  printer.print(expr);
  return os;
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

Expr operator%(Expr a, Expr b) {
  return Rem::make(a, b);
}

// class Stmt
std::ostream &operator<<(std::ostream &os, const Stmt &Stmt) {
  IRPrinter printer(os);
  printer.skipTopExprParenthesis();
  printer.print(Stmt);
  return os;
}

// class ForDomain
std::ostream &operator<<(std::ostream &os, const ForDomain &d) {
  switch (d.kind) {
    case ForDomain::IndexSet:
      os << d.indexSet;
      break;
    case ForDomain::Endpoints:
      os << d.set << ".endpoints[" << d.var << "]";
      break;
    case ForDomain::Edges:
      os << d.set << ".edges[" << d.var << "]";
      break;
    case ForDomain::Lattice:
      os << "lattice[";
      for (const Var& v : d.latticeVars) {
        os << v << ",";
      }
      os << "]";
      break;
    case ForDomain::NeighborsOf:
      os << d.set << ".neighborsOf[" << d.var << "]";
    case ForDomain::Neighbors:
      os << d.set << ".neighbors[" << d.var << "]";
      break;
    case ForDomain::Diagonal:
      os << d.set << ".diagonal[" << d.var << "]";
      break;
  }
  return os;
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
    const ScalarType componentType = elemFieldType->getComponentType(); 

    // The type of a set field is 
    // `tensor[set](tensor[elementFieldDimensions](elemFieldComponentType))[']`
    // If the element field is a row vector, then the set field is also a row 
    // vector. Otherwise, the set field is a column vector.
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

    const bool isColumnVector = (elemFieldType->getDimensions().size() == 0 || 
                                elemFieldType->isColumnVector);
    fieldType = TensorType::make(componentType, dimensions, isColumnVector);
  }
  return fieldType;
}

Type getBlockType(Expr tensor) {
  iassert(tensor.type().isTensor());
  return tensor.type().toTensor()->getBlockType();
}

Type getIndexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr, 
                      bool isColumnVector) {
  iassert(isScalar(expr.type()));
  std::vector<IndexDomain> dimensions;
  for (auto &indexVar : lhsIndexVars) {
    dimensions.push_back(indexVar.getDomain());
  }

  const auto componentType = expr.type().toTensor()->getComponentType();
  return TensorType::make(componentType, dimensions, isColumnVector);
}

// enum CompoundOperator
std::ostream &operator<<(std::ostream &os, const CompoundOperator &cop) {
  switch (cop) {
    case CompoundOperator::None: {
      break;
    }
    case CompoundOperator::Add: {
      os << "+";
      break;
    }
  }
  return os;
}

// struct Literal
void Literal::cast(Type type) {
  iassert(type.isTensor());
  iassert(type.toTensor()->getComponentType() ==
          this->type.toTensor()->getComponentType());
  iassert(type.toTensor()->size() == this->type.toTensor()->size());
  this->type = type;
}

int Literal::getIntVal(int index) const {
  iassert(type.toTensor()->getComponentType().isInt())
      << "getIntVal only valid for literals with int components";
  return ((int*)data)[index];
}

double Literal::getFloatVal(int index) const {
  if (ScalarType::singleFloat()) {
    return ((float*)data)[index];
  }
  else {
    return ((double*)data)[index];
  }
}

double_complex Literal::getComplexVal(int index) const {
  if (ScalarType::singleFloat()) {
    return ((float_complex*)data)[index];
  }
  else {
    return ((double_complex*)data)[index];
  }
}

bool Literal::isAllZeros() const {
  for (unsigned i = 0; i < size; ++i) {
    if (((uint8_t*)data)[i] != 0) {
      return false;
    }
  }
  return true;
}

Expr Literal::make(Type type) {
  return Literal::make(type, nullptr, 0);
}

Expr Literal::make(int val) {
  return make(Int, &val, sizeof(int));
}

Expr Literal::make(double val) {
  // Choose appropriate precision
  if (ScalarType::singleFloat()) {
    float floatVal = (float) val;
    return make(Float, &floatVal, sizeof(float));
  }
  else {
    return make(Float, &val, sizeof(double));
  }
}

Expr Literal::make(bool val) {
  return make(Boolean, &val, sizeof(bool));
}

Expr Literal::make(std::string val) {
  Literal *node = new Literal;
  node->type = String;
  node->size = sizeof(char) * (val.length() + 1);
  node->data = malloc(node->size);
  val.copy((char *)node->data, val.length());
  ((char *)(node->data))[node->size - 1] = '\0';
  return node;
}

Expr Literal::make(double_complex val) {
  // Choose appropriate precision
  if (ScalarType::singleFloat()) {
    float_complex floatVal;
    floatVal.real = (float) val.real;
    floatVal.imag = (float) val.imag;
    return make(Complex, &floatVal, sizeof(float_complex));
  }
  else {
    return make(Complex, &val, sizeof(double_complex));
  }
}

Expr Literal::make(Type type, void* values, size_t bufSize) {
  iassert(type.isTensor()) << "only tensor literals are supported for now";
  const TensorType *ttype = type.toTensor();

  size_t size = 0;
  size_t sizeInBytes = 0;
  switch (type.kind()) {
    case Type::Tensor: {
      size = ttype->size();
      sizeInBytes = size * ttype->getComponentType().bytes();
      break;
    }
    case Type::Set:
    case Type::Element:
    case Type::Tuple:
    case Type::Array:
    case Type::Opaque:
      iassert(false) << "only tensor and scalar literals currently supported";
      break;
    case Type::Undefined:
      ierror << "attempting to create literal of undefined type";
      break;
  }

  Literal *node = new Literal;
  node->type = type;
  node->size = sizeInBytes;
  node->data = malloc(node->size);
  if (values != nullptr) {
    iassert(node->size <= bufSize)
        << "bufSize too small for desired type: " << type
        << ", needed " << sizeInBytes << ", got " << bufSize;
    memcpy(node->data, values, node->size);
  }
  else {
    // Zero array
    switch (ttype->getComponentType().kind) {
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
      case ir::ScalarType::Complex:
        if (ir::ScalarType::singleFloat()) {
          iassert(ir::ScalarType::floatBytes == sizeof(float));
          util::zero<float>(node->data, size);
        }
        else {
          iassert(ir::ScalarType::floatBytes == sizeof(double));
          util::zero<double>(node->data, size);
        }
        break;
      case ir::ScalarType::String:
        unreachable;
    }
  }
  return node;
}

Expr Literal::make(Type type, std::vector<double> values) {
  iassert(isScalar(type) || type.toTensor()->getComponentType().isFloat() && 
          type.toTensor()->size() == values.size() || 
          type.toTensor()->getComponentType().isComplex() && 
          2 * type.toTensor()->size() == values.size());
  iassert(type.toTensor()->getComponentType().isFloat() || 
          type.toTensor()->getComponentType().isComplex())
      << "Float array constructor must use float or complex component type";
  if (ScalarType::singleFloat()) {
    // Convert double vector to float vector
    std::vector<float> floatValues;
    for (double val : values) {
      floatValues.push_back(val);
    }
    return Literal::make(type, floatValues.data(),
                         util::getVectorSize(floatValues));
  }
  else {
    return Literal::make(type, values.data(),
                         util::getVectorSize(values));
  }
}

Expr Literal::make(Type type, std::vector<double_complex> values) {
  iassert(isScalar(type) || 
          type.toTensor()->getComponentType().isComplex() && 
          2 * type.toTensor()->size() == values.size());
  iassert(type.toTensor()->getComponentType().isComplex())
      << "Complex array constructor must use complex component type";
  if (ScalarType::singleFloat()) {
    // Convert double vector to float vector
    std::vector<float_complex> floatValues;
    for (double_complex val : values) {
      floatValues.push_back(float_complex(val.real, val.imag));
    }
    return Literal::make(type, floatValues.data(),
                         util::getVectorSize(floatValues));
  }
  else {
    return Literal::make(type, values.data(),
                         util::getVectorSize(values));
  }
}

Literal::~Literal() {
  free(data);
}

inline size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->getComponentType().bytes();
}

bool operator==(const Literal& l, const Literal& r) {
  iassert(l.type.isTensor() && r.type.isTensor());

  if (l.type != r.type) {
    return false;
  }

  iassert(getTensorByteSize(l.type.toTensor()) ==
          getTensorByteSize(r.type.toTensor()));

  size_t size = l.type.toTensor()->size();
  switch (l.type.toTensor()->getComponentType().kind) {
    case ir::ScalarType::Int: {
      return util::compare<int>(l.data, r.data, size);
    }
    case ir::ScalarType::Float: {
      if (ir::ScalarType::singleFloat()) {
        return util::compare<float>(l.data, r.data, size);
      }
      else {
        return util::compare<double>(l.data, r.data, size);
      }
    }
    case ir::ScalarType::Boolean: {
      return util::compare<bool>(l.data, r.data, size);
    }
    case ir::ScalarType::Complex: {
      if (ir::ScalarType::singleFloat()) {
        return util::compare<float_complex>(l.data, r.data, size);
      }
      else {
        return util::compare<double_complex>(l.data, r.data, size);
      }
    }
    case ir::ScalarType::String: {
      return (std::strcmp((const char *)l.data, (const char *)r.data) == 0);
    }
    default: {
      not_supported_yet;
      return false;
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
                        ? buffer.type().toTensor()->getComponentType()
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

Expr IndexRead::make(Expr edgeSet, Kind kind, int index) {
  iassert(edgeSet.type().isLatticeLinkSet());
  iassert(kind == LatticeDim);

  IndexRead *node = new IndexRead;
  node->type = TensorType::make(ScalarType(ScalarType::Int));

  node->edgeSet = edgeSet;
  node->kind = kind;
  node->index = index;
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

// struct Rem
Expr Rem::make(Expr a, Expr b) {
  iassert_int_scalar(a);
  iassert_types_equal(a,b);

  Rem *node = new Rem;
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
Stmt Store::make(Expr buf, Expr index, Expr value, CompoundOperator cop) {
  iassert(isScalar(value.type()));
  // TODO: Change to only allow stores to arrays, not tensors
//  iassert(buffer.type().isArray()) << "Can only store to arrays";
//  iassert(value.type()==TensorType::make(buff().toArray()->elementType))
//            << "Stored value type " << util::quote(value.type())
//            << " does not match the element type of array "
//            << util::quote(buff().toArray()->elementType);
  iassert(buf.type().isArray() || buf.type().isTensor())
      << "Can only store to arrays and tensors";
  iassert(!buf.type().isTensor() ||
          TensorType::make(buf.type().toTensor()->getComponentType())==value.type())
      << "Stored value type " << util::quote(value.type())
      << " does not match the component type of tensor "
      << util::quote(buf.type().toTensor()->getBlockType()) ;
  Store *node = new Store;
  node->buffer = buf;
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

// struct Scope
Stmt Scope::make(Stmt scopedStmt) {
  iassert(scopedStmt.defined());
  Scope *node = new Scope;
  node->scopedStmt = scopedStmt;
  return node;
}

// struct IfThenElse
Stmt IfThenElse::make(Expr condition, Stmt thenBody) {
  IfThenElse *node = new IfThenElse;
  node->condition = condition;
  node->thenBody = Scope::make(thenBody);
  node->elseBody = Stmt();
  return node;
}

Stmt IfThenElse::make(Expr condition, Stmt thenBody, Stmt elseBody) {
  IfThenElse *node = new IfThenElse;
  node->condition = condition;
  node->thenBody = Scope::make(thenBody);
  node->elseBody = Scope::make(elseBody);
  return node;
}

// struct ForRange
Stmt ForRange::make(Var var, Expr start, Expr end, Stmt body) {
  iassert(var.defined());
  iassert(body.defined());
  iassert(start.defined());
  iassert(end.defined());

  ForRange *node = new ForRange;
  node->var = var;
  node->start = start;
  node->end = end;
  node->body = Scope::make(body);
  return Scope::make(node);  // Put loop variable in a scope
}

// struct For
Stmt For::make(Var var, ForDomain domain, Stmt body) {
  For *node = new For;
  node->var = var;
  node->domain = domain;
  node->body = Scope::make(body);
  return Scope::make(node);  // Put loop variable in a scope
}

// struct While
Stmt While::make(Expr condition, Stmt body) {
  While *node = new While;
  node->condition = condition;
  node->body = Scope::make(body);
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

// struct Block
Stmt Block::make(Stmt first, Stmt rest) {
  iassert(first.defined() || rest.defined()) << "Empty block";

  // Handle case where first is undefined, to ease codegen in loops
  if (!first.defined()) {
    std::swap(first,rest);
  }

  Block *node = new Block;
  node->first = first;
  node->rest = rest;
  return node;
}

Stmt Block::make(std::vector<Stmt> stmts) {
  iassert(stmts.size() > 0) << "Empty block";
  Stmt node;
  for (size_t i=stmts.size(); i>0; --i) {
    node = Block::make(stmts[i-1], node);
  }
  return node;
}

// struct Print
Stmt Print::make(Expr expr, std::string format) {
  Print *node = new Print;
  node->expr = expr;
  node->format = format;
  return node;
}

Stmt Print::make(std::string str) {
  return Print::make(Literal::make(str));
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

// struct SetRead
Expr SetRead::make(Expr set, std::vector<Expr> indices) {
#ifdef SIMIT_ASSERTS
  iassert(set.type().isSet());
  for (const Expr &index : indices) {
    iassert(isScalar(index.type()));
  }
#endif

  if (set.type().isUnstructuredSet() &&
      set.type().toUnstructuredSet()->getCardinality() == 0) {
    // TODO: Can't check dimensions of a free-standing unstructured
    // set, we probably need to track this globally at some point.
    // For now, this is checked during map lowering.
  }
  else if (set.type().isLatticeLinkSet()) {
    // Indices should index both the source offset and sink offset
    // giving 2*dims indices.
    iassert(indices.size() == set.type().toLatticeLinkSet()->dimensions*2);
  }
  else {
    not_supported_yet;
  }

  SetRead *node = new SetRead;
  node->type = set.type().toSet()->elementType;
  node->set = set;
  node->indices = indices;
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
        << "for var " << indexVars[i] << ": "
        << indexVars[i].getDomain() << " != " << dimensions[i];
  }
#endif

  IndexedTensor *node = new IndexedTensor;
  node->type = TensorType::make(tensor.type().toTensor()->getComponentType());
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

Expr IndexExpr::make(std::vector<IndexVar> resultVars, Expr value,
                     bool isColumnVector) {
  iassert(isScalar(value.type())) << value << " : " << value.type();
#ifdef SIMIT_ASSERTS
  for (auto &idxVar : resultVars) {  // No reduction variables on lhs
    iassert(idxVar.isFreeVar());
  }
#endif

  IndexExpr *node = new IndexExpr;
  node->type = getIndexExprType(resultVars, value, isColumnVector);
  node->resultVars = resultVars;
  node->value = value;
  return node;
}

// struct Map
Stmt Map::make(std::vector<Var> vars,
               Func function, std::vector<Expr> partial_actuals,
               Expr target, Expr neighbors, Expr through,
               ReductionOperator reduction) {
  iassert(target.type().isSet());
  iassert(!neighbors.defined() || neighbors.type().isSet());
  //iassert(vars.size() == function.getResults().size());
  Map *node = new Map;
  node->vars = vars;
  node->function = function;
  node->partial_actuals = partial_actuals;
  node->target = target;
  node->neighbors = neighbors;
  node->through = through;
  node->reduction = reduction;
  return node;
}

}}

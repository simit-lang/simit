#include "ir.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util.h"
#include "macros.h"

using namespace std;

namespace simit {
namespace ir {

// class IRNode
IRNode::~IRNode() {}

// class Literal
Literal::Literal(const std::shared_ptr<Type> &type) : Expression(type) {
  this->dataSize = type->getByteSize();
  this->data = malloc(dataSize);
}

Literal::Literal(const std::shared_ptr<Type> &type, void *values) : Literal(type) {
  memcpy(this->data, values, this->dataSize);
}

Literal::~Literal() {
  free(data);
}

void Literal::clear() {
  memset(data, 0, dataSize);
}

void Literal::cast(const std::shared_ptr<TensorType> &type) {
  assert(type->getKind() == Type::Kind::Tensor);
  TensorType *litType = tensorTypePtr(this->getType());
  assert(litType->getComponentType() == type->getComponentType() &&
         litType->getSize() == type->getSize());

  setType(type);
}

void Literal::print(std::ostream &os) const {
  // TODO: Fix value printing to print matrices and tensors properly
  switch (getType()->getKind()) {
    case Type::Kind::Set:
      NOT_SUPPORTED_YET;
      break;
    case Type::Kind::Tensor: {
      TensorType *ttype = tensorTypePtr(getType());
      assert(isValidComponentType(ttype->getComponentType()));
      switch (ttype->getComponentType()) {
        case ComponentType::INT: { {
          int *idata = (int*)data;
          if (ttype->getSize() == 1) {
            os << idata[0];
          }
          else {
            os << "[" << idata[0];
            for (size_t i=0; i < ttype->getSize(); ++i) {
              os << ", " << idata[i];
            }
            os << "]";
          }
          break;
        }
        case ComponentType::FLOAT: {
          double *fdata = (double*)data;
          if (ttype->getSize() == 1) {
            os << fdata[0];
          }
          else {
            os << "[" << to_string(fdata[0]);
            for (size_t i=1; i < ttype->getSize(); ++i) {
              os << ", " + to_string(fdata[i]);
            }
            os << "]";
          }
          break;
        }
        }
      }
    }
      
      os << " : " << *getType();
  }
}

bool operator==(const Literal& l, const Literal& r) {
  if (*l.getType() != *r.getType()) {
    return false;
  }

  assert(l.getType()->getByteSize() == r.getType()->getByteSize());
  if (memcmp(l.getData(), r.getData(), l.getType()->getByteSize()) != 0) {
    return false;
  }
  return true;
}

bool operator!=(const Literal& l, const Literal& r) {
  return !(l == r);
}


// class IndexVar
std::string IndexVar::operatorString(Operator op) {
  switch (op) {
    case IndexVar::FREE:
      return "free";
    case IndexVar::SUM:
      return "sum";
    case IndexVar::PRODUCT:
      return "product";
    default:
      UNREACHABLE;
  }
}

std::string IndexVar::operatorSymbol(Operator op) {
  switch (op) {
    case IndexVar::FREE:
      return "";
    case IndexVar::SUM:
      return "+";
    case IndexVar::PRODUCT:
      return "*";
    default:
      UNREACHABLE;
  }
}

std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  return os << IndexVar::operatorSymbol(var.getOperator()) << var.getName();
}


// class IndexedTensor
typedef std::vector<std::shared_ptr<IndexVar>> IndexVariables;
IndexedTensor::IndexedTensor(const std::shared_ptr<Expression> &tensor,
                             const IndexVariables &indexVars){
  assert(tensor->getType()->getKind() == Type::Kind::Tensor &&
         "Only tensors can be indexed.");
  TensorType *ttype = tensorTypePtr(tensor->getType());
  assert(indexVars.size() == ttype->getOrder());
  for (size_t i=0; i < indexVars.size(); ++i) {
    assert(indexVars[i]->getIndexSet() == ttype->getDimensions()[i]
           && "IndexVar domain does not match tensordimension");
  }

  this->tensor = tensor;
  this->indexVariables = indexVars;
}

std::ostream &operator<<(std::ostream &os, const IndexedTensor &t) {
  os << t.getTensor()->getName();

  if (t.getIndexVariables().size() > 0) {
    os << "(";
    auto it = t.getIndexVariables().begin();
    if (it != t.getIndexVariables().end()) {
      os << (*it)->getName();
      ++it;
    }
    while (it != t.getIndexVariables().end()) {
      os << "," << (*it)->getName();
      ++it;
    }
    os << ")";
  }
  return os;
}


// class IndexExpr
int IndexExpr::numOperands(Operator op) {
  return (op == NONE || op == NEG) ? 1 : 2;
}

IndexExpr::IndexExpr(const std::vector<std::shared_ptr<IndexVar>> &indexVars,
                     Operator op, const std::vector<IndexedTensor> &operands)
    : Expression(NULL), indexVars(indexVars), op(op), operands(operands) {
  initType();

  // Can't have reduction variables on rhs
  for (auto &idxVar : indexVars) {
    assert(idxVar->getOperator() == IndexVar::Operator::FREE);
  }

  // Operand typechecks
  assert(operands.size() == (size_t)IndexExpr::numOperands(op));
  assert(operands[0].getTensor()->getType()->getKind() == Type::Kind::Tensor &&
         "Only tensors can be indexed.");
  TensorType *firstType = tensorTypePtr(operands[0].getTensor()->getType());
  ComponentType first = firstType->getComponentType();
  for (auto &operand : operands) {
    assert(operand.getTensor()->getType()->getKind() == Type::Kind::Tensor &&
           "Only tensors can be indexed.");
  TensorType *ttype = tensorTypePtr(operand.getTensor()->getType());
    assert(first == ttype->getComponentType() &&
           "Operand component types differ");
  }
}

IndexExpr::IndexExpr(IndexExpr::Operator op,
                     const vector<IndexedTensor> &operands)
    : IndexExpr(std::vector<std::shared_ptr<IndexVar>>(), op, operands) {

}

void IndexExpr::setIndexVariables(const vector<shared_ptr<IndexVar>> &ivs) {
  this->indexVars = ivs;
  initType();
}

void IndexExpr::setOperator(IndexExpr::Operator op) {
  this->op = op;
}

void IndexExpr::setOperands(const std::vector<IndexedTensor> &operands) {
  assert(operands.size() > 0);
  TensorType *newType = tensorTypePtr(operands[0].getTensor()->getType());
  TensorType *oldType = tensorTypePtr(this->operands[0].getTensor()->getType());

  bool reinit = (newType->getComponentType() != oldType->getComponentType());
  this->operands = operands;
  if (reinit) {
    initType();
  }
}

vector<shared_ptr<IndexVar>> IndexExpr::getDomain() const {
  vector<shared_ptr<IndexVar>> domain;
  set<shared_ptr<IndexVar>> added;
  for (auto &iv : indexVars) {
    if (added.find(iv) == added.end()) {
      added.insert(iv);
      domain.push_back(iv);
    }
  }
  for (auto &operand : operands) {
    for (auto &iv : operand.getIndexVariables()) {
      if (added.find(iv) == added.end()) {
        assert(iv->getOperator() != IndexVar::FREE
               && "freevars not used on lhs");
        added.insert(iv);
        domain.push_back(iv);
      }
    }
  }
  return domain;

}

void IndexExpr::initType() {
  assert(operands.size() > 0);
  TensorType *ttype = tensorTypePtr(operands[0].getTensor()->getType());
  ComponentType ctype = ttype->getComponentType();
  std::vector<IndexSetProduct> dimensions;
  for (auto &iv : indexVars) {
    dimensions.push_back(iv->getIndexSet());
  }
  setType(std::shared_ptr<TensorType>(new TensorType(ctype, dimensions)));
}

static std::string opString(IndexExpr::Operator op) {
  std::string opstr;
  switch (op) {
    case IndexExpr::Operator::NONE:
      return "";
    case IndexExpr::Operator::NEG:
      return "-";
    case IndexExpr::Operator::ADD:
      return "+";
    case IndexExpr::Operator::SUB:
      return "-";
    case IndexExpr::Operator::MUL:
      return "*";
    case IndexExpr::Operator::DIV:
      return "//";
    default:
      UNREACHABLE;
  }
}

void IndexExpr::print(std::ostream &os) const {
  std::string idxVarStr =
      (indexVars.size()!=0) ? "(" + simit::util::join(indexVars,",") + ")" : "";
  os << getName() << idxVarStr << " = ";

  std::set<std::shared_ptr<IndexVar>> rvars;
  for (auto &operand : operands) {
    for (auto &iv : operand.getIndexVariables()) {
      if (iv->getOperator() != IndexVar::Operator::FREE &&
          rvars.find(iv) == rvars.end()) {
        rvars.insert(iv);
        os << *iv << " ";
      }
    }
  }

  unsigned int numOperands = operands.size();
  auto opit = operands.begin();
  if (numOperands == 1) {
    os << opString(op) << *opit++;
  }
  else if (numOperands == 2) {
    os << *opit++ << " " << opString(op) << " " << *opit++;
  } else {
    assert(false && "Not supported yet");
  }
}


// class Call
void Call::print(std::ostream &os) const {
  os << getName() << "(" << util::join(arguments, ", ") << ")";
}


// class FieldRead
namespace {
// The type of a set field is:
// `Tensor[set][elementFieldDimensions](elemFieldComponentType)`.
std::shared_ptr<Type> fieldType(const std::shared_ptr<Expression> &setExpr,
                                const std::string &fieldName){
  assert(setExpr->getType()->isSet());

  const shared_ptr<SetType> &setType =
      static_pointer_cast<SetType>(setExpr->getType());
  const shared_ptr<TensorType> &elemFieldType =
      setType->getElementType()->getFields().at(fieldName);

  std::vector<IndexSetProduct> dimensions;
  if (elemFieldType->getOrder() == 0) {
    IndexSet dim(setExpr->getName());
    dimensions.push_back(IndexSetProduct(dim));
  }
  else {
    NOT_SUPPORTED_YET;
  }

  TensorType *fieldType = new TensorType(elemFieldType->getComponentType(),
                                         dimensions);

  return std::shared_ptr<TensorType>(fieldType);
}}

FieldRead::FieldRead(const std::shared_ptr<Expression> &set,
                     const std::string &fieldName)
    : Read(set->getName()+"."+fieldName, fieldType(set, fieldName)),
      set(set), fieldName(fieldName) {}

void FieldRead::print(std::ostream &os) const {
  os << getName();
}


// class FieldWrite
void FieldWrite::print(std::ostream &os) const {
  os << *getValue();
}


// class Argument
void Argument::print(std::ostream &os) const {
  os << getName() << " : " << *getType();
}


// class Function
void Function::addStatements(const std::vector<std::shared_ptr<IRNode>> &stmts){
  body.insert(body.end(), stmts.begin(), stmts.end());
}

namespace {
class FunctionBodyPrinter : public IRVisitor {
 public:
  FunctionBodyPrinter(std::ostream &os) : IRVisitor(), os(os) {}

  void handle(Function *f)   { UNUSED(f); }
  void handle(Argument *t)   { UNUSED(t); }
  void handle(Result *t)     { UNUSED(t); }
  void handle(FieldRead *t)  { UNUSED(t); }
  void handle(FieldWrite *t) { UNUSED(t); }

  void handleDefault(IRNode *t) { os << "  " << *t << endl; }

 private:
  std::ostream &os;
};

} // unnamed namespace

void Function::print(std::ostream &os) const {
  string argumentString = "(" + util::join(this->arguments, ", ") + ")";
  string resultString = (results.size() == 0)
      ? "" : " -> (" + util::join(this->results, ", ") + ")";
  os << "func " << getName() << argumentString << resultString << endl;
  FunctionBodyPrinter fp(os);
  fp.visit((Function*)this);
  os << "end";
}


// class Test
void Test::print(std::ostream &os) const {
  std::vector<std::shared_ptr<Expression>> args;
  args.insert(args.end(), arguments.begin(), arguments.end());
  Call call(callee, args);
  os << call << " == " << util::join(expected, ", ");
}

}} // namespace simit::internal

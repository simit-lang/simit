#include "ir.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util.h"

using namespace std;

namespace simit {
namespace internal {

// class IRNode
IRNode::~IRNode() {}

std::ostream &operator<<(std::ostream &os, const IRNode &node){
  node.print(os);
  return os;
}


// class TensorNode
TensorNode::~TensorNode() {
  delete type;
}

void TensorNode::print(std::ostream &os) const {
  os << getName() << " : " << *type;
}


// class Literal
Literal::Literal(TensorType *type) : TensorNode(type) {
  int componentSize = TensorType::componentSize(type->getComponentType());
  this->dataSize = type->getSize() * componentSize;
  this->data = malloc(dataSize);
}

Literal::Literal(TensorType *type, void *values) : Literal(type) {
  memcpy(this->data, values, this->dataSize);
}

Literal::~Literal() {
  free(data);
}

void Literal::clear() {
  memset(data, 0, dataSize);
}

void Literal::cast(TensorType *type) {
  assert(this->type->getComponentType() == type->getComponentType() &&
         this->type->getSize() == type->getSize());
  delete this->type;
  this->type = type;
}

void Literal::print(std::ostream &os) const {
  for (auto &dim : type->getDimensions()) {
    assert(dim.getFactors().size() == 1 && "literals can't be hierarchical");
  }

  // TODO: Fix value printing to print matrices and tensors properly
  switch (type->getComponentType()) {
    case Type::INT: {
      int *idata = (int*)data;
      if (type->getSize() == 1) {
        os << idata[0];
      }
      else {
        os << "[" << idata[0];
        for (int i=0; i<type->getSize(); ++i) {
          os << ", " << idata[i];
        }
        os << "]";
      }
      break;
    }
    case Type::FLOAT: {
      double *fdata = (double*)data;
      if (type->getSize() == 1) {
        os << fdata[0];
      }
      else {
        os << "[" << to_string(fdata[0]);
        for (int i=1; i<type->getSize(); ++i) {
          os << ", " + to_string(fdata[i]);
        }
        os << "]";
      }
      break;
    }
    case Type::ELEMENT:
      assert(false && "Unsupported (TODO)");
      break;
    default:
      UNREACHABLE;
  }

  os << " : " << *type;
}

bool operator==(const Literal& l, const Literal& r) {
  if (*l.getType() != *r.getType()) {
    return false;
  }
  assert(l.getType()->getSize() == r.getType()->getSize());
  simit::Type ctype = l.getType()->getComponentType();
  int byteSize = l.getType()->getSize() * TensorType::componentSize(ctype);

  if (memcmp(l.getData(), r.getData(), byteSize) != 0) {
    return false;
  }
  return true;
}

bool operator!=(const Literal& l, const Literal& r) {
  return !(l == r);
}


// class IndexVar
std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  switch (var.getOperator()) {
    case IndexVar::FREE:
      break;
    case IndexVar::SUM:
      os << "+";
      break;
    case IndexVar::PRODUCT:
      os << "*";
      break;
    default:
      UNREACHABLE;
  }
  return os << var.getName();
}


// class IndexExpr
int IndexExpr::numOperands(Operator op) {
  return (op == NONE || op == NEG) ? 1 : 2;
}

IndexExpr::IndexedTensor::IndexedTensor(const std::shared_ptr<TensorNode> &t,
                                        const IndexVarPtrVector &ivs) {
  assert(ivs.size() == t->getOrder());
  auto titer = t->getType()->getDimensions().begin();
  auto iviter = ivs.begin();
  for (; iviter != ivs.end(); ++iviter, ++titer) {
    assert((*iviter)->getIndexSet() == (*titer));
  }
  this->tensor = t;
  this->indexVariables = ivs;
}

namespace {
TensorType *
computeIndexExprType(const std::vector<IndexExpr::IndexVarPtr> &indexVars,
                     const std::vector<IndexExpr::IndexedTensor> &operands) {
  Type ctype = operands[0].getTensor()->getType()->getComponentType();
  std::vector<IndexSetProduct> dimensions;
  for (auto &iv : indexVars) {
    dimensions.push_back(iv->getIndexSet());
  }
  return new TensorType(ctype, dimensions);
}
}

IndexExpr::IndexExpr(const std::vector<IndexVarPtr> &indexVars,
                     Operator op,
                     const std::vector<IndexedTensor> &operands)
    : TensorNode(computeIndexExprType(indexVars, operands)),
      indexVars{indexVars}, op{op}, operands{operands} {
  assert(operands.size() == (size_t)IndexExpr::numOperands(op));
  Type firstType = operands[0].getTensor()->getType()->getComponentType();
  for (auto &operand : operands) {
    assert(firstType == operand.getTensor()->getType()->getComponentType() &&
           "Operand component types differ");
  }
}

const std::vector<IndexExpr::IndexVarPtr> &IndexExpr::getDomain() const {
  return indexVars;
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

static inline
std::string indexVarString(const std::vector<IndexExpr::IndexVarPtr> &idxVars) {
  return (idxVars.size()!=0) ? "(" + simit::util::join(idxVars,",") + ")" : "";
}

static inline
std::string indexedTensorString(const IndexExpr::IndexedTensor &it) {
  return it.getTensor()->getName() + indexVarString(it.getIndexVariables());
}

void IndexExpr::print(std::ostream &os) const {
  os << getName() << indexVarString(indexVars) << " = ";

  unsigned int numOperands = operands.size();
  auto opit = operands.begin();
  if (numOperands == 1) {
    os << opString(op) + indexedTensorString(*opit++);
  }
  else if (numOperands == 2) {
    os << indexedTensorString(*opit++) << opString(op) <<
          indexedTensorString(*opit++);
  } else {
    assert(false && "Not supported yet");
  }
}


// class Call
void Call::print(std::ostream &os) const {
  os << getName() << "(" << util::join(arguments, ", ") << ")";
}


// class VariableStore
void VariableStore::print(std::ostream &os) const {
  os << getName() << " = " << value->getName();
}


// class Function
void Function::addStatements(const std::vector<std::shared_ptr<IRNode>> &stmts){
  body.insert(body.end(), stmts.begin(), stmts.end());
}

namespace {
class FunctionBodyPrinter : public IRVisitor {
 public:
  FunctionBodyPrinter(std::ostream &os) : IRVisitor(), os(os) {}

  void handle(Function *f) { UNUSED(f); }
  void handle(Argument *t) { UNUSED(t); }
  void handle(Result *t)   { UNUSED(t); }

  void handleDefault(IRNode *t) { os << "  " << *t << endl; }

 private:
  std::ostream &os;
};

} // unnamed namespace

void Function::print(std::ostream &os) const {
  string argumentString = "(" + util::join(this->arguments, ", ") + ")";
  string resultString = (results.size() == 0)
      ? "" : " -> (" + util::join(this->results, ", ") + ")";
  os << "func " << name << argumentString << resultString << endl;
  FunctionBodyPrinter fp(os);
  fp.visit((Function*)this);
  os << "end";
}


// class Argument
void Argument::print(std::ostream &os) const {
  TensorNode::print(os);
}


// class Result
void Result::print(std::ostream &os) const {
  TensorNode::print(os);
}


// class Test
void Test::print(std::ostream &os) const {
  std::vector<std::shared_ptr<TensorNode>> args;
  args.insert(args.end(), arguments.begin(), arguments.end());
  Call call(callee, args);
  os << call << " == " << util::join(expected, ", ");
}

}} // namespace simit::internal

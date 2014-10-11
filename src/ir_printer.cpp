#include "ir_printer.h"

#include "ir.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const Function &function) {
  IRPrinter printer(os);
  printer.print(function);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Expression &expression) {
  IRPrinter printer(os);
  printer.print(expression);
  return os;
}


// class IRPrinter
IRPrinter::IRPrinter(std::ostream &os, signed indent)
    : os(os), indentation(0), printingFunctionBody(false), id(1) {
}

void IRPrinter::print(const Function &func) {
  os << "func " << func.getName() << "(";
  auto argIt = func.getArguments().begin();
  auto argEnd = func.getArguments().end();
  if (argIt != argEnd) {
    handle(argIt->get());
    ++argIt;
  }
  while (argIt != argEnd) {
    os << ", ";
    handle(argIt->get());
    ++argIt;
  }
  os << ")";

  if (func.getResults().size() > 0) {
    os << " -> (";
    auto resIt = func.getResults().begin();
    auto resEnd = func.getResults().end();
    if (resIt != resEnd) {
      handle(resIt->get());
      ++resIt;
    }
    while (resIt != resEnd) {
      os << ", ";
      handle(resIt->get());
      ++resIt;
    }
    os << ")";
  }

  os << "\n";
  ++indentation;
  printingFunctionBody = true;

  for (auto &stmt : func.getBody()) {
    stmt->accept(this);
  }

  printingFunctionBody = false;
  --indentation;
  os << "end";
}

void IRPrinter::print(const Expression &expression) {
  expression.accept(this);
}

void IRPrinter::print(const IndexedTensor &indexedTensor) {
  os << getName(indexedTensor.getTensor());

  if (indexedTensor.getIndexVariables().size() > 0) {
    os << "(";
    auto it = indexedTensor.getIndexVariables().begin();
    auto end = indexedTensor.getIndexVariables().end();
    if (it != end) {
      os << (*it)->getName();
      ++it;
    }
    while (it != end) {
      os << "," << (*it)->getName();
      ++it;
    }
    os << ")";
  }
}

void IRPrinter::handle(const Argument *op) {
  if(!printingFunctionBody) {
    os << op->getName() << " : " << op->getType();
    names.insert(op->getName());
  }
}

void IRPrinter::handle(const Result *op) {
  handle(static_cast<const Argument*>(op));
  if(!printingFunctionBody) {
    for (auto &value : op->getValues()) {
      nodeToName[value.get()] = op->getName();
    }
  }
}

void IRPrinter::handle(const Literal *op) {
  indent();
  // TODO: Fix value printing to print matrices and tensors properly
  os << getName(op) << " = ";
  switch (op->getType().getKind()) {
    case Type::Tensor: {
      const TensorType *ttype = op->getType().toTensor();
      size_t tsize = ttype->size();
      switch (ttype->componentType.toScalar()->kind) {
        case ScalarType::Int: { {
          const int *idata = static_cast<const int*>(op->getConstData());
          if (tsize == 1) {
            os << idata[0];
          }
          else {
            os << "[" << idata[0];
            for (size_t i=0; i < tsize; ++i) {
              os << ", " << idata[i];
            }
            os << "]";
          }
          break;
        }
        case ScalarType::Float: {
          const double *fdata = static_cast<const double*>(op->getConstData());
          if (tsize == 1) {
            os << fdata[0];
          }
          else {
            os << "[" << to_string(fdata[0]);
            for (size_t i=1; i < tsize; ++i) {
              os << ", " + to_string(fdata[i]);
            }
            os << "]";
          }
          break;
        }
        }
      }
      break;
    }
    case Type::Scalar:
      NOT_SUPPORTED_YET;
      break;
    case Type::Element:
      NOT_SUPPORTED_YET;
    case Type::Set:
      NOT_SUPPORTED_YET;
      break;
    case Type::Tuple:
      NOT_SUPPORTED_YET;
      break;
  }
  os << "\n";
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

void IRPrinter::handle(const IndexExpr *op) {
  indent();
  os << getName(op);
  if (op->getIndexVariables().size() != 0) {
    os << "(" + simit::util::join(op->getIndexVariables(), ",") + ")";
  }
  os << " = ";

  std::set<std::shared_ptr<IndexVar>> rvars;
  for (auto &operand : op->getOperands()) {
    for (auto &iv : operand.getIndexVariables()) {
      if (iv->isReductionVar() && rvars.find(iv) == rvars.end()) {
        rvars.insert(iv);
        os << *iv << " ";
      }
    }
  }

  unsigned int numOperands = op->getOperands().size();
  auto opIt = op->getOperands().begin();
  if (numOperands == 1) {
    os << opString(op->getOperator());
    print(*opIt);
    ++opIt;
  }
  else if (numOperands == 2) {
    print(*opIt);
    ++opIt;
    os << " " << opString(op->getOperator()) << " ";
    print(*opIt);
    ++opIt;
  } else {
    assert(false && "Not supported yet");
  }
  os << "\n";
}

void IRPrinter::handle(const Call *op) {
  indent();
  os << "Call";
  os << "\n";
}

void IRPrinter::handle(const FieldRead *op) {
  indent();
  os << getName(op) << " = " <<
        getName(op->getTarget()) << "." << op->getFieldName();
  os << "\n";
}

void IRPrinter::handle(const FieldWrite *op) {
  indent();
  os << getName(op->getTarget()) << "." << op->getFieldName() << " = " <<
        getName(op->getValue());
  os << "\n";
}

void IRPrinter::handle(const TensorRead *op) {
  indent();
  os << getName(op) << " = ";
  os << getName(op->getTensor()) << "(";
  auto it = op->getIndices().begin();
  auto end = op->getIndices().end();
  if (it != end) {
    os << getName(*it);
    ++it;
  }
  for (; it != end; ++it) {
    os << "," << getName(*it);
  }
  os << ")";
  os << "\n";
}

void IRPrinter::handle(const TensorWrite *op) {
  indent();
  os << getName(op) << "(";
  auto it = op->getIndices().begin();
  auto end = op->getIndices().end();
  if (it != end) {
    os << getName(*it);
    ++it;
  }
  for (; it != end; ++it) {
    os << "," << getName(*it);
  }
  os << ")" << " = " << getName(op->getValue());
  os << "\n";
}

void IRPrinter::indent() {
  for (unsigned i=0; i<indentation; ++i) {
    os << "  ";
  }
}

std::string IRPrinter::getName(const Expression *expr) {
  if (nodeToName.find(expr) != nodeToName.end()) {
    return nodeToName.at(expr);
  }

  std::string name = expr->getName();
  if (name == "") {
    name = "tmp";
  }

  if (names.find(name) == names.end()) {
    names.insert(name);
  }
  else {
    name += to_string(id++);
  }

  nodeToName[expr] = name;
  return name;
}

std::string IRPrinter::getName(const std::shared_ptr<Expression> &expr) {
  return getName(expr.get());
}

}} //namespace simit::ir

#include "ir_printer.h"

#include "ir.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const Func &function) {
  IRPrinter printer(os);
  printer.print(function);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Expr &expr) {
  IRPrinter printer(os);
  printer.print(expr);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Stmt &Stmt) {
  IRPrinter printer(os);
  printer.print(Stmt);
  return os;
}


// class IRPrinter
IRPrinter::IRPrinter(std::ostream &os, signed indent) : os(os), indentation(0) {
}

void IRPrinter::print(const Func &func) {
  os << "func " << func.getName() << "(";
  if (func.getArguments().size() > 0) {
    Expr arg = func.getArguments()[0];
    print(arg);
    os << " : " << arg.type();
  }
  for (size_t i=1; i < func.getArguments().size(); ++i) {
    Expr arg = func.getArguments()[i];
    os << ", ";
    print(arg);
    os << " : " << arg.type();
  }
  os << ")";

  if (func.getResults().size() > 0) {
    os << " -> (";
    print(func.getResults()[0]);
    os << " : " << func.getResults()[0].type();

    for (size_t i=1; i < func.getResults().size(); ++i) {
      Expr res = func.getResults()[i];
      os << ", ";
      print(res);
      os << " : " << res.type();
    }
    os << ")";
  }

  os << "\n";
  ++indentation;
  print(func.getBody());
  --indentation;
  os << "end";
}

void IRPrinter::print(const Expr &expr) {
  expr.accept(this);
}

void IRPrinter::print(const Stmt &stmt) {
  stmt.accept(this);
}

void IRPrinter::visit(const Literal *op) {
  // TODO: Fix value printing to print matrices and tensors properly
  switch (op->type.getKind()) {
    case Type::Tensor: {
      const TensorType *ttype = op->type.toTensor();
      size_t tsize = ttype->size();
      switch (ttype->componentType.toScalar()->kind) {
        case ScalarType::Int: { {
          const int *idata = static_cast<const int*>(op->data);
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
          const double *fdata = static_cast<const double*>(op->data);
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

void IRPrinter::visit(const Variable *op) {
  os << op->name;
}

void IRPrinter::visit(const Result *) {
  os << "result";
}

void IRPrinter::visit(const FieldRead *op) {
  print(op->elementOrSet);
  os << "." << op->fieldName;
}

void IRPrinter::visit(const TensorRead *op) {
  print(op->tensor);
  os << "(";
  auto indices = op->indices;
  if (indices.size() > 0) {
    print(indices[0]);
  }
  for (size_t i=1; i < indices.size(); ++i) {
    os << ",";
    print(indices[i]);
  }
  os << ")";
}

void IRPrinter::visit(const TupleRead *op) {
  print(op->tuple);
  os << "(";
  print(op->index);
  os << ")";
}

void IRPrinter::visit(const Map *op) {
  os << "map " << op->function;
  os << " to ";
  print(op->target);
  os << " with ";
  print(op->neighbors);
  os << " reduce " << op->reductionOp;
}

void IRPrinter::visit(const IndexedTensor *op) {
  print(op->tensor);
  if (op->indexVars.size() > 0) {
    os << "(" << util::join(op->indexVars,",") << ")";
  }
}

void IRPrinter::visit(const IndexExpr *op) {
  if (op->lhsIndexVars.size() != 0) {
    os << "(" + simit::util::join(op->lhsIndexVars, ",") + ") ";
  }
  print(op->expr);
}

void IRPrinter::visit(const Call *op) {
  os << "Call";
}

void IRPrinter::visit(const Neg *op) {
  os << "-";
  print(op->a);
}

void IRPrinter::visit(const Add *op) {
  os << "(";
  print(op->a);
  os << " + ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const Sub *op) {
  os << "(";
  print(op->a);
  os << " - ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const Mul *op) {
  os << "(";
  print(op->a);
  os << " * ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const Div *op) {
  os << "(";
  print(op->a);
  os << " / ";
  print(op->b);
  os << ")";
}

void IRPrinter::visit(const AssignStmt *op) {
  indent();
  os << util::join(op->lhs) << " = ";
  print(op->rhs);
  os << ";\n";
}

void IRPrinter::visit(const FieldWrite *op) {
  indent();
  print(op->elementOrSet);
  os << "." << op->fieldName << " = ";
  print(op->value);
  os << ";\n";
}

void IRPrinter::visit(const TensorWrite *op) {
  indent();
  print(op->tensor);
  os << "(";
  auto indices = op->indices;
  if (indices.size() > 0) {
    print(indices[0]);
  }
  for (size_t i=1; i < indices.size(); ++i) {
    os << ",";
    print(indices[i]);
  }
  os << ") = ";
  print(op->value);
  os << ";\n";
}

void IRPrinter::visit(const For *op) {
  os << "for";
}

void IRPrinter::visit(const IfThenElse *op) {
  os << "ifthenelse";
}

void IRPrinter::visit(const Block *op) {
  os << "block";
}

void IRPrinter::visit(const Pass *op) {
  os << "pass";
}

void IRPrinter::indent() {
  for (unsigned i=0; i<indentation; ++i) {
    os << "  ";
  }
}

}} //namespace simit::ir

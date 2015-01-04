#include "ir_printer.h"

#include "ir.h"
#include "util.h"

#ifdef GPU
#include "gpu_backend/gpu_ir.h"
#endif

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

std::ostream &operator<<(std::ostream &os, const IRNode &node) {
  IRPrinter printer(os);
  printer.print(node);
  return os;
}

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
    case ForDomain::Neighbors:
      os << d.set << ".neighbors[" << d.var << "]";
      break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const CompoundOperator &cop) {
  switch (cop.kind) {
    case CompoundOperator::None: break;
    case CompoundOperator::Add: {
      os << "+";
      break;
    }
    default: not_supported_yet;
  }
  return os;
}


// class IRPrinter
IRPrinter::IRPrinter(std::ostream &os, signed indent) : os(os), indentation(0) {
}

void IRPrinter::print(const Func &func) {
  if (func.defined()) {
    func.accept(this);
  }
  else {
    os << "Func()";
  }
}

void IRPrinter::print(const Expr &expr) {
  if (expr.defined()) {
    expr.accept(this);
  }
  else {
    os << "Expr()";
  }
}

void IRPrinter::print(const Stmt &stmt) {
  if (stmt.defined()) {
    stmt.accept(this);
  }
  else {
    indent();
    os << "Stmt()";
  }
}

void IRPrinter::print(const IRNode &node) {
  node.accept(this);
}

static inline string bool_to_string(bool value) {
  if (value)
    return "true";
  else
    return "false";
}

void IRPrinter::visit(const Literal *op) {
  // TODO: Fix value printing to print matrices and tensors properly
  switch (op->type.kind()) {
    case Type::Tensor: {
      size_t size;
      ScalarType::Kind componentType;

      iassert(op->type.kind() == Type::Tensor);
      const TensorType *type = op->type.toTensor();
      size = type->size();
      componentType = type->componentType.kind;

      switch (componentType) {
        case ScalarType::Int:  {
          const int *idata = static_cast<const int*>(op->data);
          if (size == 1) {
            os << idata[0];
          }
          else {
            os << "[" << idata[0];
            for (size_t i=0; i < size; ++i) {
              os << ", " << idata[i];
            }
            os << "]";
          }
          break;
        }
        case ScalarType::Float: {
          if (size == 1) {
            os << op->getFloatVal(0);
          }
          else {
            os << "[" << to_string(op->getFloatVal(0));
            for (size_t i=1; i < size; ++i) {
              os << ", " + to_string(op->getFloatVal(i));
            }
            os << "]";
          }
          break;
        }
        case ScalarType::Boolean: {
          const bool *bdata = static_cast<const bool*>(op->data);
          if (size == 1) {
            os << bool_to_string(bdata[0]);
          }
          else {
            os << "[" + bool_to_string(bdata[0]);
            for (size_t i=1; i < size; ++i) {
              os << ", " + bool_to_string(bdata[i]);
            }
            os << "]";
          }
          break;
        }

        }
      }
      break;
    
    case Type::Element:
      not_supported_yet;
    case Type::Set:
      not_supported_yet;
      break;
    case Type::Tuple:
      not_supported_yet;
      break;
  }
}

void IRPrinter::visit(const VarExpr *op) {
  os << op->var;
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

void IRPrinter::visit(const IndexRead *op) {
  print(op->edgeSet);
  os << ".";
  switch (op->kind) {
    case IndexRead::Endpoints:
      os << "endpoints";
      break;
    case IndexRead::NeighborsStart:
      os << "neighbors.start";
      break;
    case IndexRead::Neighbors:
      os << "neighbors";
      break;
  }
}

void IRPrinter::visit(const Length *op) {
  os << op->indexSet;
}

void IRPrinter::visit(const Load *op) {
  print(op->buffer);
  os << "[";
  print(op->index);
  os << "]";
}

void IRPrinter::visit(const IndexedTensor *op) {
  print(op->tensor);
  if (op->indexVars.size() > 0) {
    os << "{" << util::join(op->indexVars,",") << "}";
  }
}

void IRPrinter::visit(const IndexExpr *op) {
  os << "{";
  if (op->resultVars.size() > 0) {
    os << "\u2200" + simit::util::join(op->resultVars, ",") + " ";
  }
  print(op->value);
  os << "}";
}

void IRPrinter::visit(const Call *op) {
  os << op->func.getName() << "(" << util::join(op->actuals) << ")";
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

#define PRINT_VISIT_BINARY_OP(type, symbol, op) \
  void IRPrinter::visit(const type *op) {\
  os << "(";\
  print(op->a);\
  os << " " << #symbol << " ";\
  print(op->b);\
  os << ")";\
}

PRINT_VISIT_BINARY_OP(Eq, ==, op)
PRINT_VISIT_BINARY_OP(Ne, !=, op)
PRINT_VISIT_BINARY_OP(Gt, >, op)
PRINT_VISIT_BINARY_OP(Lt, <, op)
PRINT_VISIT_BINARY_OP(Ge, >=, op)
PRINT_VISIT_BINARY_OP(Le, <=, op)
PRINT_VISIT_BINARY_OP(And, and, op)
PRINT_VISIT_BINARY_OP(Or, or, op)
PRINT_VISIT_BINARY_OP(Xor, xor, op)

void IRPrinter::visit(const Not *op) {
  os << "(not ";
  print(op->a);
  os << ")";
}

void IRPrinter::visit(const AssignStmt *op) {
  indent();
  os << op->var << " " << op->cop << "= ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const Map *op) {
  indent();
  os << util::join(op->vars) << " = ";
  os << "map " << op->function.getName();
  os << "(" << util::join(op->partial_actuals) << ")";
  os << " to ";
  print(op->target);
  if (op->neighbors.defined()) {
    os << " with ";
    print(op->neighbors);
  }
  if (op->reduction.getKind() != ReductionOperator::Undefined) {
    os << " reduce " << op->reduction;
  }
  os << ";";
}

void IRPrinter::visit(const FieldWrite *op) {
  indent();
  print(op->elementOrSet);
  os << "." << op->fieldName << " " << op->cop << "= ";
  print(op->value);
  os << ";";
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
  os << ") " << op->cop << "= ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const Store *op) {
  indent();
  print(op->buffer);
  os << "[";
  print(op->index);
  os << "] " << op->cop << "= ";
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const ForRange *op) {
  indent();
  os << "for " << op->var << " in " << op->start << ":" << op->end;
  os << ":" << endl;
  ++indentation;
  print(op->body);
  --indentation;
}

void IRPrinter::visit(const For *op) {
  indent();
  os << "for " << op->var << " in " << op->domain;
  os << ":" << endl;
  ++indentation;
  print(op->body);
  --indentation;
}

#ifdef GPU
void IRPrinter::visit(const GPUFor *op) {
  indent();
  os << "gpufor " << op->var << " in " << op->domain;
  os << ":" << endl;
  ++indentation;
  print(op->body);
  --indentation;
}
#endif

void IRPrinter::visit(const While *op) {
  indent();
  os << "while";
  print(op->condition);
  os << endl;
  ++indentation;
  print(op->body);
  --indentation;
}

void IRPrinter::visit(const IfThenElse *op) {
  indent();
  os << "if ";
  print(op->condition);
  os << endl;
  ++indentation;
  print(op->thenBody);
  --indentation;
  os << "else" << endl;
  ++indentation;
  print(op->elseBody);
  --indentation;
}

void IRPrinter::visit(const Block *op) {
  print(op->first);
  if (op->rest.defined()) {
    os << endl;
    print(op->rest);
  }
}

void IRPrinter::visit(const Pass *op) {
  indent();
  os << "pass;";
}

void IRPrinter::visit(const Print *op) {
  indent();
  os << "print ";
  print(op->expr);
  os << ";";
}

void IRPrinter::visit(const Func *func) {
  os << "func " << func->getName() << "(";
  if (func->getArguments().size() > 0) {
    const Var &arg = func->getArguments()[0];
    os << arg << " : " << arg.getType();
  }
  for (size_t i=1; i < func->getArguments().size(); ++i) {
    const Var &arg = func->getArguments()[i];
    os << ", " << arg << " : " << arg.getType();
  }
  os << ")";

  if (func->getResults().size() > 0) {
    os << " -> (";
    const Var &res = func->getResults()[0];
    os << res << " : " << res.getType();

    for (size_t i=1; i < func->getResults().size(); ++i) {
      const Var &res = func->getResults()[i];
      os << ", " << res << " : " << res.getType();
    }
    os << ")";
  }

  if (func->getBody().defined()) {
    os << ":" << endl;

    ++indentation;
    print(func->getBody());
    --indentation;
  }
  else {
    os << ";";
  }
}

void IRPrinter::indent() {
  for (unsigned i=0; i<indentation; ++i) {
    os << "  ";
  }
}

}} //namespace simit::ir

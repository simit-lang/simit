#include "ir_printer.h"

#include "ir.h"
#include "util/util.h"

#include <regex>
#include <string>
#include <sstream>

#ifdef GPU
#include "backend/gpu/gpu_ir.h"
#endif

using namespace std;

namespace simit {
namespace ir {

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

void IRPrinter::skipTopExprParenthesis() {
  skipTopExprParen = true;
}

static inline string bool_to_string(bool value) {
  if (value) {
    return "true";
  }
  else {
    return "false";
  }
}

static inline string complex_to_string(double_complex c) {
  stringstream ss;
  ss << "<" << c.real << "," << c.imag << ">";
  return ss.str();
}

void IRPrinter::visit(const Literal *op) {
  clearSkipParen();

  // TODO: Fix value printing to print matrices and tensors properly
  switch (op->type.kind()) {
    case Type::Tensor: {
      size_t size;
      ScalarType::Kind componentType;

      iassert(op->type.kind() == Type::Tensor);
      const TensorType *type = op->type.toTensor();
      size = type->size();
      componentType = type->getComponentType().kind;

      switch (componentType) {
        case ScalarType::Int:  {
          const int *idata = static_cast<const int*>(op->data);
          if (size == 1) {
            os << idata[0];
          }
          else {
            os << "[" << idata[0];
            for (size_t i=1; i < size; ++i) {
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
        case ScalarType::Complex: {
          if (size == 1) {
            os << complex_to_string(op->getComplexVal(0));
          }
          else {
            os << "[" + complex_to_string(op->getComplexVal(0));
            for (size_t i=1; i < size; ++i) {
              os << ", " + complex_to_string(op->getComplexVal(i));
            }
            os << "]";
          }
          break;
        }
        case ScalarType::String: {
          const char *sdata = static_cast<const char*>(op->data);
          os << "\"" << util::escape(std::string(sdata)) << "\"";
          break;
        }

        }
      }
      break;
    
    case Type::Element:
    case Type::Set:
    case Type::Tuple:
    case Type::NamedTuple:
    case Type::Array:
    case Type::Opaque:
      not_supported_yet;
      break;
    case Type::Undefined:
      os << "undefined type";
  }
}

void IRPrinter::visit(const VarExpr *op) {
  clearSkipParen();
  os << op->var;
}

void IRPrinter::visit(const Load *op) {
  clearSkipParen();
  print(op->buffer);
  os << "[";
  skipTopExprParenthesis();
  print(op->index);
  os << "]";
}

void IRPrinter::visit(const FieldRead *op) {
  clearSkipParen();
  print(op->elementOrSet);
  os << "." << op->fieldName;
}

void IRPrinter::visit(const Length *op) {
  clearSkipParen();
  os << "length(" << op->indexSet << ")";
}

void IRPrinter::visit(const IndexRead *op) {
  clearSkipParen();
  print(op->edgeSet);
  os << ".";
  switch (op->kind) {
    case IndexRead::Endpoints:
      os << "endpoints";
      break;
    case IndexRead::LatticeDim:
      os << "latticedim[" << op->index << "]";
      break;
    default:
      not_supported_yet;
      break;
  }
}

#define PRINT_VISIT_BINARY_OP(type, symbol, op)                                \
void IRPrinter::visit(const type *op) {                                        \
  auto p = paren();                                                            \
  print(op->a);                                                                \
  os << " " << #symbol << " ";                                                 \
  print(op->b);                                                                \
}

void IRPrinter::visit(const Neg *op) {
  clearSkipParen();
  os << "-";
  print(op->a);
}

PRINT_VISIT_BINARY_OP(Add, +, op)
PRINT_VISIT_BINARY_OP(Sub, -, op)
PRINT_VISIT_BINARY_OP(Mul, *, op)
PRINT_VISIT_BINARY_OP(Div, /, op)
PRINT_VISIT_BINARY_OP(Rem, %, op)

void IRPrinter::visit(const Not *op) {
  auto p = paren();
  os << "not ";
  print(op->a);
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


// Statements

void IRPrinter::visit(const VarDecl *op) {
  indent();
  os << "var " << op->var << " : " << op->var.getType();
  os << ";";
}

void IRPrinter::visit(const AssignStmt *op) {
  indent();
  os << op->var << " " << op->cop << "= ";
  skipTopExprParenthesis();
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const CallStmt *op) {
  indent();
  if (!op->results.empty()) {
    os << util::join(op->results) << " = "; 
  }
  os << op->callee.getName() << "(" << 
    util::join(op->actuals) << ")" << ";";
}

void IRPrinter::visit(const TupleRead *op) {
  clearSkipParen();
  print(op->tuple);
  os << "(";
  print(op->index);
  os << ")";
}

void IRPrinter::visit(const NamedTupleRead *op) {
  clearSkipParen();
  print(op->tuple);
  os << "." << op->elementName;
}

void IRPrinter::visit(const SetRead *op) {
  clearSkipParen();
  print(op->set);
  os << "[";
  auto indices = op->indices;
  if (indices.size() > 0) {
    print(indices[0]);
  }
  for (size_t i=1; i < indices.size(); ++i) {
    os << ",";
    print(indices[i]);
  }
  os << "]";
}

void IRPrinter::visit(const TensorRead *op) {
  clearSkipParen();
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

void IRPrinter::visit(const IndexedTensor *op) {
  clearSkipParen();
  print(op->tensor);
  if (op->indexVars.size() > 0) {
    os << "(" << util::join(op->indexVars,",") << ")";
  }
}

void IRPrinter::visit(const IndexExpr *op) {
  clearSkipParen();
  os << "(";
  if (op->resultVars.size() > 0) {
    os << simit::util::join(op->resultVars, ",") + " ";
  }
  skipTopExprParenthesis();
  print(op->value);
  os << ")";
}

void IRPrinter::visit(const Map *op) {
  indent();
  if (op->vars.size() > 0) {
    os << util::join(op->vars) << " = ";
  }

  os << "map " << op->function.getName();

  if (op->partial_actuals.size() > 0) {
    os << "(" << util::join(op->partial_actuals) << ")";
  }

  os << " to ";
  print(op->target);
  if (op->neighbors.defined()) {
    os << " with ";
    print(op->neighbors);
  }
  if (op->reduction.getKind() != ReductionOperator::Undefined) {
    os << " reduce " << op->reduction;
  }
  if (op->through.defined()) {
    os << " through ";
    print(op->through);
  }
  os << ";";
}

void IRPrinter::visit(const FieldWrite *op) {
  indent();
  print(op->elementOrSet);
  os << "." << op->fieldName << " " << op->cop << "= ";
  skipTopExprParenthesis();
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
  skipTopExprParenthesis();
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const Store *op) {
  indent();
  print(op->buffer);
  os << "[";
  print(op->index);
  os << "] " << op->cop << "= ";
  skipTopExprParenthesis();
  print(op->value);
  os << ";";
}

void IRPrinter::visit(const Scope *op) {
  print(op->scopedStmt);
}

void IRPrinter::visit(const IfThenElse *op) {
  indent();
  os << "if ";
  skipTopExprParenthesis();
  print(op->condition);
  os << endl;
  ++indentation;
  print(op->thenBody);
  --indentation;
  if (op->elseBody.defined()) {
    os << endl;
    indent();
    os << "else" << endl;
    ++indentation;
    print(op->elseBody);
    --indentation;
  }
  os << endl;
  indent();
  os << "end";
}

void IRPrinter::visit(const ForRange *op) {
  indent();
  os << "for " << op->var << " in " << op->start << ":" << op->end << endl;
  ++indentation;
  print(op->body);
  --indentation;
  os << endl;
  indent();
  os << "end";
}

void IRPrinter::visit(const For *op) {
  indent();
  os << "for " << op->var << " in " << op->domain << endl;
  ++indentation;
  print(op->body);
  --indentation;
  os << endl;
  indent();
  os << "end";
}

void IRPrinter::visit(const While *op) {
  indent();
  os << "while ";
  skipTopExprParenthesis();
  print(op->condition);
  os << endl;
  ++indentation;
  print(op->body);
  --indentation;
  os << endl;
  indent();
  os << "end";
}

void IRPrinter::visit(const Kernel *op) {
  indent();
  os << "kernel for " << op->var << " in " << op->domain << endl;
  ++indentation;
  print(op->body);
  --indentation;
  indent();
  os << "end";
}

void IRPrinter::visit(const Block *op) {
  print(op->first);
  if (op->rest.defined()) {
    os << endl;
    print(op->rest);
  }
}

void IRPrinter::visit(const Print *op) {
  indent();
  os << "print ";
  if (op->format != "") {
    os << "(";
    print(op->expr);
    os << ", \"" << op->format << "\")";
  } else {
    print(op->expr);
  }
  os << ";";
}

void IRPrinter::visit(const Comment *op) {
  if (op->headerSpace) {
    os << "\n";
  }
  indent();
  os << "% " << op->comment << endl;
  if (op->commentedStmt.defined()) {
    print(op->commentedStmt);
  }
  if (op->footerSpace) {
    os << "\n";
  }
}

void IRPrinter::visit(const Pass *op) {
  indent();
  os << "pass;";
}

#ifdef GPU
void IRPrinter::visit(const GPUKernel *op) {
  indent();
  os << "gpukernel [";
  bool first = true;
  for (Var read : op->reads) {
    if (!first) os << ",";
    else first = false;
    os << read;
  }
  os << "|";
  first = true;
  for (Var write : op->writes) {
    if (!first) os << ",";
    else first = false;
    os << write;
  }
  os << "] ";
  backend::GPUSharding sharding = op->sharding;
  if (sharding.xSharded) {
    os << "x{" << sharding.xVar << " in " << sharding.xDomain << "} ";
  }
  if (sharding.ySharded) {
    os << "y{" << sharding.yVar << " in " << sharding.yDomain << "} ";
  }
  if (sharding.zSharded) {
    os << "z{" << sharding.zVar << " in " << sharding.zDomain << "} ";
  }
  os << ":" << endl;
  ++indentation;
  print(op->body);
  --indentation;
}
#endif

string toString(const Var& arg) {
  stringstream ss;
  ss << arg;
  if (arg.defined()) {
    ss << " : " << arg.getType();
  }
  return ss.str();
}

void IRPrinter::visit(const Func *func) {
  const Environment& env = func->getEnvironment();
  if (!env.isEmpty()) {
    os << env << endl << endl;
  }

  if (func->getKind() == Func::External) {
    os << "extern ";
  }

  os << "func " << func->getName() << "(";
  if (func->getArguments().size() > 0) {
    const Var &arg = func->getArguments()[0];
    if (arg.defined()) {
      os << toString(arg);
    }
  }
  for (size_t i=1; i < func->getArguments().size(); ++i) {
    const Var &arg = func->getArguments()[i];
    os << ", " << toString(arg);
  }
  os << ")";

  if (func->getResults().size() > 0) {
    os << " -> (";
    const Var &res = func->getResults()[0];
    os << toString(res);

    for (size_t i=1; i < func->getResults().size(); ++i) {
      const Var &res = func->getResults()[i];
      os << ", " << toString(res);
    }
    os << ")";
  }

  if (func->getBody().defined()) {
    os << ":" << endl;

    ++indentation;
    print(func->getBody());
    --indentation;
    os << endl;
    indent();
    os << "end";
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


// class IRPrinterCallGraph
void IRPrinterCallGraph::print(const Func &func) {
  if (func.defined()) {
    func.accept(this);
  }
  else {
    os << "Func()";
  }
}

void IRPrinterCallGraph::visit(const CallStmt *op) {
  op->callee.accept(this);
}

void IRPrinterCallGraph::visit(const Map *op) {
  op->function.accept(this);
}

void IRPrinterCallGraph::visit(const Func *op) {
  if (op->getKind() == Func::Intrinsic) return;

  if (visited.find(*op) == visited.end()) {
    visited.insert(*op);
    IRVisitor::visit(op);
    os << *op << endl << endl;
  }
}



}} //namespace simit::ir

#include "sir_printer.h"

#include "sir.h"
#include <iostream>

using namespace std;

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const Stmt &stmt) {
  SetIRPrinter printer(os);
  printer.print(stmt);
  return os;
}

std::ostream &operator<<(std::ostream &os, const Expr &expr) {
  SetIRPrinter printer(os);
  printer.print(expr);
  return os;
}

SetIRPrinter::~SetIRPrinter() {
}

void SetIRPrinter::indent() {
  for (unsigned i=0; i<indentation; ++i) {
    os << "  ";
  }
}

void SetIRPrinter::print(const Expr &expr) {
  expr.accept(this);
}

void SetIRPrinter::print(const Stmt &stmt) {
  stmt.accept(this);
}

void SetIRPrinter::visit(const IntLiteral *op) {
  os << op->val;
}

void SetIRPrinter::visit(const Variable *op) {
  os << op->name;
}

void SetIRPrinter::visit(const Load *op) {
  print(op->target);
  os << "[";
  print(op->index);
  os << "]";
}

void SetIRPrinter::visit(const Neg *op) {
  os << "(-";
  print(op->a);
  os << ")";
}

void SetIRPrinter::visit(const Add *op) {
  os << "(";
  print(op->a);
  os << " + ";
  print(op->b);
  os << ")";
}

void SetIRPrinter::visit(const Sub *op) {
  os << "(";
  print(op->a);
  os << " - ";
  print(op->b);
  os << ")";
}

void SetIRPrinter::visit(const Mul *op) {
  os << "(";
  print(op->a);
  os << " * ";
  print(op->b);
  os << ")";
}

void SetIRPrinter::visit(const Div *op) {
  os << "(";
  print(op->a);
  os << " / ";
  print(op->b);
  os << ")";
}

void SetIRPrinter::visit(const Block *op) {
  print(op->first);
  if (op->rest.defined()) {
    print(op->rest);
  }
}

void SetIRPrinter::visit(const Foreach *op) {
  indent();
  os << "for " << op->name << " in ";
  os << op->domain;
  os << ":\n";
  ++indentation;
  print(op->body);
  --indentation;
}

void SetIRPrinter::visit(const Store *op) {
  indent();
  print(op->target);
  os << "[";
  print(op->index);
  os << "] = ";
  print(op->value);
}

void SetIRPrinter::visit(const StoreMatrix *op) {
  indent();
}

void SetIRPrinter::visit(const Pass *op) {
  indent();
  os << "pass";
}

}} //namespace simit::ir

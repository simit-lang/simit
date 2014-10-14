#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include "scopedmap.h"
#include "types.h"
#include "ir.h"
#include "ir_printer.h"

namespace simit {
namespace internal {

class RWExprPair {
public:
  RWExprPair() {}
  RWExprPair(ir::Expr read, ir::Expr write)
      : read(read), write(write) {}

  bool isReadable() const { return read.defined(); }
  bool isWritable() const { return write.defined(); }

  ir::Expr getReadExpr() const { return read; }
  ir::Expr getWriteExpr() const { return write; }

private:
  ir::Expr read;
  ir::Expr write;
};

inline std::ostream &operator<<(std::ostream &os, const RWExprPair &rwExpr) {
  os << "(";
  if (rwExpr.isReadable()) {
    os << rwExpr.getReadExpr();
  }
  else {
    os << "none";
  }
  os << ", ";
  if (rwExpr.isWritable()) {
    os << rwExpr.getWriteExpr();
  }
  else {
    os << "none";
  }
  return os << ")";
}


class ProgramContext {
public:
  ProgramContext() {}

  ~ProgramContext() {
    for (auto &test : tests) {
      delete test;
    }
  }

  void scope()   {
    exprSymtable.scope();
  }

  void unscope() {
    exprSymtable.unscope();
  }

  void addSymbol(const std::string &name, ir::Expr readWrite) {
    addSymbol(name, readWrite, readWrite);
  }

  void addSymbol(const std::string &name, ir::Expr read, ir::Expr write) {
    exprSymtable.insert(name, RWExprPair(read,write));
  }

  const RWExprPair &getSymbol(const std::string &name) {
    assert(hasSymbol(name));
    return exprSymtable.get(name);
  }

  bool hasSymbol(const std::string &name) {
    return exprSymtable.contains(name);
  }

  void addFunction(ir::Func f) {
    functions[f.getName()] = f;
  }

  bool containsFunction(const std::string &name) const {
    return functions.find(name) != functions.end();
  }

  ir::Func getFunction(const std::string &name) {
    return functions[name];
  }

  std::map<std::string, ir::Func> getFunctions() {
    return functions;
  }

  void addElementType(ir::Type elemType) {
    elementTypes[elemType.toElement()->name] = elemType;
  }

  bool containsElementType(const std::string &name) {
    return elementTypes.find(name) != elementTypes.end();
  }

  ir::Type getElementType(const std::string &name) {
    return elementTypes[name];
  }

  void addExtern(ir::Expr externVariable) {
    externs[toVariable(externVariable)->name] = externVariable;
  }

  bool containsExtern(const std::string &name) {
    return externs.find(name) != externs.end();
  }

  ir::Expr getExtern(const std::string &name) {
    return externs[name];
  }

  const std::map<std::string,ir::Expr> &getExterns() {
    return externs;
  }

  void addTest(ir::Test *test) { tests.push_back(test); }

  const std::vector<ir::Test*> &getTests() const { return tests; }

private:
  std::map<std::string, ir::Func>    functions;
  std::map<std::string, ir::Expr>    externs;

  std::map<std::string, ir::Type>    elementTypes;
  ScopedMap<std::string, RWExprPair> exprSymtable;

  std::vector<ir::Test*>             tests;
};

}} // namespace simit::internal

#endif

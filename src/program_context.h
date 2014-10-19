#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility>

#include "scopedmap.h"
#include "types.h"
#include "ir.h"
#include "ir_builder.h"

namespace simit {
namespace internal {

class Symbol {
public:
  Symbol() {}
  Symbol(ir::Expr read, ir::Expr write)
      : read(read), write(write) {}

  bool isReadable() const { return read.defined(); }
  bool isWritable() const { return write.defined(); }

  ir::Expr getReadExpr() const { return read; }
  ir::Expr getWriteExpr() const { return write; }

private:
  ir::Expr read;
  ir::Expr write;
};

inline std::ostream &operator<<(std::ostream &os, const Symbol &symbol) {
  os << "(";
  if (symbol.isReadable()) {
    os << symbol.getReadExpr();
  }
  else {
    os << "none";
  }
  os << ", ";
  if (symbol.isWritable()) {
    os << symbol.getWriteExpr();
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

  void scope() {
    exprSymtable.scope();
    statements.push_front(std::vector<ir::Stmt>());
    builder.setInsertionPoint(&statements.front());
  }

  void unscope() {
    exprSymtable.unscope();
    statements.pop_front();
    builder.setInsertionPoint(statements.size() > 0
                              ? &statements.front() : nullptr);
  }

  void addSymbol(const std::string &name, ir::Expr readWrite) {
    addSymbol(name, readWrite, readWrite);
  }

  void addSymbol(const std::string &name, ir::Expr read, ir::Expr write) {
    exprSymtable.insert(name, Symbol(read, write));
  }

  const Symbol &getSymbol(const std::string &name) {
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
    externs[ir::to<ir::Variable>(externVariable)->name] = externVariable;
  }

  bool containsExtern(const std::string &name) {
    return externs.find(name) != externs.end();
  }

  ir::Expr getExtern(const std::string &name) {return externs[name];}

  const std::map<std::string,ir::Expr> &getExterns() {return externs;}

  void addTest(ir::Test *test) {tests.push_back(test);}

  const std::vector<ir::Test*> &getTests() const {return tests;}

  ir::IRBuilder *getBuilder() {return &builder;}

  void addStatement(ir::Stmt stmt) {statements.front().push_back(stmt);}
  
  std::vector<ir::Stmt> *getStatements() {return &statements.front();}

private:
  ir::IRBuilder builder;

  std::map<std::string, ir::Func>    functions;
  std::map<std::string, ir::Expr>    externs;

  std::map<std::string, ir::Type>    elementTypes;
  ScopedMap<std::string, Symbol>     exprSymtable;

  std::list<std::vector<ir::Stmt>>   statements;

  std::vector<ir::Test*>             tests;
};

}} // namespace simit::internal

#endif

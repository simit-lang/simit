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
  enum Access { None, Read, Write, ReadWrite };

  Symbol() : access(None) {}
  Symbol(ir::Var var, Access access)
      : var(var), expr(ir::VarExpr::make(var)), access(access) {}

  bool isReadable() const {return access ==  Read || access == ReadWrite;}
  bool isWritable() const {return access ==  Write || access == ReadWrite;}

  ir::Var getVar() const {return var;}
  ir::Expr getExpr() const {return expr;}

private:
  ir::Var var;
  ir::Expr expr;

  Access access;
};

inline std::ostream &operator<<(std::ostream &os, const Symbol &symbol) {
  os << symbol.getVar() << "(";

  if (symbol.isReadable()) {
    os << "r";
  }
  if (symbol.isWritable()) {
    os << "w";
  }
  return os;
}


class ProgramContext {
public:
  ProgramContext() {
    functions.insert(ir::Intrinsics::byName.begin(),
                     ir::Intrinsics::byName.end());
  }

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

  void addSymbol(ir::Var var) {
    addSymbol(var.name, var, Symbol::ReadWrite);
  }

  void addSymbol(const std::string &name, ir::Var var, Symbol::Access access) {
    exprSymtable.insert(name, Symbol(var, access));
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

  void addExtern(ir::Var externVariable) {
    externs[externVariable.name] = externVariable;
  }

  bool containsExtern(const std::string &name) {
    return externs.find(name) != externs.end();
  }

  ir::Var getExtern(const std::string &name) {
    return externs[name];
  }

  ir::Var getExternExpr(const std::string &name) {return externs[name];}

  const std::map<std::string,ir::Var> &getExterns() {return externs;}

  void addConstant(ir::Var var, ir::Expr val) {
    constants[var] = val;
  }

  ir::Expr getConstant(ir::Var var) {return constants[var];}


  void addTest(ir::Test *test) {tests.push_back(test);}

  const std::vector<ir::Test*> &getTests() const {return tests;}

  ir::IRBuilder *getBuilder() {return &builder;}

  void addStatement(ir::Stmt stmt) {statements.front().push_back(stmt);}
  
  std::vector<ir::Stmt> *getStatements() {return &statements.front();}

private:
  ir::IRBuilder builder;

  std::map<std::string, ir::Type>  elementTypes;

  std::map<std::string, ir::Var>   externs;
  std::map<ir::Var,ir::Expr>       constants;

  std::map<std::string, ir::Func>  functions;
  std::list<std::vector<ir::Stmt>> statements;

  ScopedMap<std::string, Symbol>   exprSymtable;

  std::vector<ir::Test*>           tests;
};

}} // namespace simit::internal

#endif

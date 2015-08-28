#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility>

#include "types.h"
#include "ir.h"
#include "func.h"
#include "intrinsics.h"
#include "ir_builder.h"
#include "ir_codegen.h"
#include "util/scopedmap.h"

#include "test.h"

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
  os << symbol.getVar() << " : ";

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
    auto bynames = ir::intrinsics::byNames();
    functions.insert(ir::intrinsics::byNames().begin(),
                     ir::intrinsics::byNames().end());
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
    addSymbol(var.getName(), var, Symbol::ReadWrite);
  }

  void addSymbol(const std::string &name, ir::Var var, Symbol::Access access) {
    exprSymtable.insert(name, Symbol(var, access));
  }

  const Symbol &getSymbol(const std::string &name) {
    iassert(hasSymbol(name));
    return exprSymtable.get(name);
  }

  bool hasSymbol(const std::string &name) {
    return exprSymtable.contains(name);
  }

  void addFunction(ir::Func f) {
    class FunctionEnvironmentBuilder : private ir::IRVisitor {
    public:
      FunctionEnvironmentBuilder(const ProgramContext &ctx) : ctx(ctx) {}

      ir::Func buildEnvironment(const ir::Func& func) {
        func.accept(this);
        return ir::Func(func, env);
      }
      
    private:
      const ProgramContext &ctx;
      ir::Environment env;
      using IRVisitor::visit;
      void visit(const ir::VarExpr *op) {
        if (ctx.isConstant(op->var)) {
          env.addConstant(op->var, ctx.getConstantInitializer(op->var));
        }
      }
    };

    f = FunctionEnvironmentBuilder(*this).buildEnvironment(f);
    f = insertVarDecls(f);

    functions[f.getName()] = f;
  }

  bool containsFunction(const std::string &name) const {
    return functions.find(name) != functions.end();
  }

  ir::Func getFunction(const std::string &name) {
    return functions[name];
  }

  const std::map<std::string, ir::Func> &getFunctions() {
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
    externs[externVariable.getName()] = externVariable;
  }

  bool containsExtern(const std::string &name) {
    return externs.find(name) != externs.end();
  }

  ir::Var getExtern(const std::string &name) { return externs[name];}

  ir::Var getExternExpr(const std::string &name) {return externs[name];}

  const std::map<std::string,ir::Var> &getExterns() {return externs;}


  void addConstant(ir::Var var, ir::Expr val) {
    iassert(ir::isa<ir::Literal>(val));
    constants[var] = val;
  }

  bool isConstant(ir::Var var) const {
    return constants.find(var) != constants.end();
  }

  ir::Expr getConstantInitializer(ir::Var var) const {return constants.at(var);}

  const std::map<ir::Var,ir::Expr> &getConstants() const {return constants;}


  void addTest(Test *test) {tests.push_back(test);}

  const std::vector<Test*> &getTests() const {return tests;}

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
  std::vector<Test*>               tests;

  util::ScopedMap<std::string, Symbol> exprSymtable;
};

}} // namespace simit::internal

#endif

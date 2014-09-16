#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include "scopedmap.h"
#include "ir.h"

namespace simit {
namespace internal {

class ProgramContext {
public:
  typedef std::pair<std::string,bool> ExprSymbolName;

  ProgramContext() {}

  void scope()   {
    exprSymtable.scope();
    columnVectors.scope();
  }

  void unscope() {
    exprSymtable.unscope();
    columnVectors.unscope();
  }

  void addSymbol(const std::string &name,
                 const std::shared_ptr<ir::Expression> &expression,
                 bool isResult=false) {
    assert(!exprSymtable.contains(ExprSymbolName(name,isResult)));

    exprSymtable.insert(ExprSymbolName(name,isResult), expression);
  }

  const std::shared_ptr<ir::Expression> &getSymbol(const std::string &name,
                                                   bool write=false) {
    if (!write) {
      return (exprSymtable.contains(ExprSymbolName(name,false)))
             ? exprSymtable.get(ExprSymbolName(name,false))
             : exprSymtable.get(ExprSymbolName(name,true));
    }
    else {
      return (exprSymtable.contains(ExprSymbolName(name,true)))
             ? exprSymtable.get(ExprSymbolName(name,true))
             : exprSymtable.get(ExprSymbolName(name,false));
      return exprSymtable.get(ExprSymbolName(name,write));
    }
  }

  bool hasSymbol(const std::string &name, bool write=false) {
    return exprSymtable.contains(ExprSymbolName(name,true)) |
           exprSymtable.contains(ExprSymbolName(name,false));
  }

  void addFunction(ir::Function *f) {
    functions[f->getName()] = f;
  }

  bool containsFunction(const std::string &name) const {
    return functions.find(name) != functions.end();
  }

  ir::Function *getFunction(const std::string &name) {
    return functions[name];
  }

  std::vector<ir::Function*> getFunctions() {
    std::vector<ir::Function*> funcVector(functions.size());
    size_t i=0;
    for (auto &func : functions) {
      funcVector[i] = func.second;
    }
    return funcVector;
  }

  void addElementType(std::shared_ptr<ir::ElementType> elemType) {
    elementTypes[elemType->getName()] = elemType;
  }

  bool containsElementType(const std::string &name) {
    return elementTypes.find(name) != elementTypes.end();
  }

  std::shared_ptr<ir::ElementType> getElementType(std::string name) {
    return elementTypes[name];
  }

  void addTest(ir::Test *test) { tests.push_back(test); }

  const std::vector<ir::Test*> &getTests() const { return tests; }

  void toggleColumnVector(const ir::Type &type) {
    if (columnVectors.contains(&type)) {
      bool &val = columnVectors.get(&type);
      val = !val;
    }
    else {
      columnVectors.insert(&type, true);
    }
  }

  bool isColumnVector(const ir::Type &type) {
    if (!columnVectors.contains(&type)) {
      return false;
    }
    return columnVectors.get(&type);
  }

private:
  std::map<std::string, ir::Function *>                   functions;
  std::map<std::string, std::shared_ptr<ir::ElementType>> elementTypes;
  std::vector<ir::Test*>                                  tests;

  ScopedMap<ExprSymbolName, std::shared_ptr<ir::Expression>> exprSymtable;
  ScopedMap<const ir::Type*, bool>                               columnVectors;
};

}} // namespace simit::internal

#endif

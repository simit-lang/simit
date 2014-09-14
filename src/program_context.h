#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "scopedmap.h"
#include "ir.h"

namespace simit {
namespace internal {

class ProgramContext {
public:
  ProgramContext() {}

  void scope()   {
    symtable.scope();
    columnVectors.scope();
  }

  void unscope() {
    symtable.unscope();
    columnVectors.unscope();
  }

  void addSymbol(const std::string &name,
                 const std::shared_ptr<Expression> &expression) {
    symtable.insert(name, expression);
  }

  const std::shared_ptr<Expression> &getSymbol(const std::string &name) {
    return symtable.get(name);
  }

  bool hasSymbol(const std::string &name) {
    return symtable.contains(name);
  }

  void addFunction(Function *f) {
    functions[f->getName()] = f;
  }

  bool containsFunction(const std::string &name) const {
    return functions.find(name) != functions.end();
  }

  Function *getFunction(const std::string &name) {
    return functions[name];
  }

  std::vector<Function*> getFunctions() {
    std::vector<Function*> funcVector(functions.size());
    size_t i=0;
    for (auto &func : functions) {
      funcVector[i] = func.second;
    }
    return funcVector;
  }

  void addElementType(std::shared_ptr<ElementType> elemType) {
    elementTypes[elemType->getName()] = elemType;
  }

  bool containsElementType(const std::string &name) {
    return elementTypes.find(name) != elementTypes.end();
  }

  std::shared_ptr<ElementType> getElementType(std::string name) {
    return elementTypes[name];
  }

  void addTest(Test *test) { tests.push_back(test); }

  const std::vector<Test*> &getTests() const { return tests; }

  void toggleColumnVector(const Type &type) {
    if (columnVectors.contains(&type)) {
      bool &val = columnVectors.get(&type);
      val = !val;
    }
    else {
      columnVectors.insert(&type, true);
    }
  }

  bool isColumnVector(const Type &type) {
    if (!columnVectors.contains(&type)) {
      return false;
    }
    return columnVectors.get(&type);
  }

private:
  std::map<std::string, Function *>                   functions;
  std::map<std::string, std::shared_ptr<ElementType>> elementTypes;
  std::vector<Test*>                                  tests;

  ScopedMap<std::string, std::shared_ptr<Expression>> symtable;
  ScopedMap<const Type*, bool>                        columnVectors;
};

}} // namespace simit::internal

#endif

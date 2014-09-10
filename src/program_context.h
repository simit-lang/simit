#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "scopedmap.h"
#include "ir.h"
#include "errors.h"

namespace simit {
namespace internal {

typedef ScopedMap<std::string, std::shared_ptr<IRNode>> ParserSymtableType;

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

  void addTensorSymbol(const std::string &name,
                       const std::shared_ptr<TensorNode> &tensor) {
    symtable.insert(name, tensor);
  }

  const std::shared_ptr<IRNode> &getSymbol(const std::string &name) {
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

  void toggleColumnVector(const TensorType *type) {
    if (columnVectors.contains(type)) {
      bool &val = columnVectors.get(type);
      val = !val;
    }
    else {
      columnVectors.insert(type, true);
    }
  }

  bool isColumnVector(const TensorType *type) {
    if (!columnVectors.contains(type)) {
      return false;
    }
    return columnVectors.get(type);
  }

  void addTest(Test *test) { tests.push_back(test); }

  const std::vector<Test*> &getTests() const { return tests; }

private:
  ParserSymtableType                  symtable;
  ScopedMap<const TensorType*, bool>  columnVectors;

  std::map<std::string, Function *>   functions;
  //    std::map<std::string, ElementType*> &elementTypes;

  std::vector<Test*>                  tests;
};

}} // namespace simit::internal

#endif

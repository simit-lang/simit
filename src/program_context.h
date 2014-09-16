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

class RWExprPair {
public:
  RWExprPair() : read(NULL), write(NULL) {}
  RWExprPair(const std::shared_ptr<ir::Expression> &read,
                          const std::shared_ptr<ir::Expression> &write)
      : read(read), write(write) {}

  bool isReadable() const { return read != NULL; }
  bool isWritable() const { return write != NULL; }

  const std::shared_ptr<ir::Expression> &getReadExpr() const { return read; }
  const std::shared_ptr<ir::Expression> &getWriteExpr() const { return write; }

private:
  std::shared_ptr<ir::Expression> read;
  std::shared_ptr<ir::Expression> write;
};


class ProgramContext {
public:
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
                 const std::shared_ptr<ir::Expression> &readWrite) {
    addSymbol(name, readWrite, readWrite);
  }

  void addSymbol(const std::string &name,
                 const std::shared_ptr<ir::Expression> &read,
                 const std::shared_ptr<ir::Expression> &write) {
    exprSymtable.insert(name, RWExprPair(read,write));
  }

  const RWExprPair &getSymbol(const std::string &name) {
    assert(hasSymbol(name));
    return exprSymtable.get(name);
  }

  bool hasSymbol(const std::string &name) {
    return exprSymtable.contains(name);
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

  ScopedMap<std::string, RWExprPair> exprSymtable;
  ScopedMap<const ir::Type*, bool>   columnVectors;
};

}} // namespace simit::internal

#endif

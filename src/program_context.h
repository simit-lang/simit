#ifndef SIMIT_PROGRAM_CONTEXT_H
#define SIMIT_PROGRAM_CONTEXT_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include "scopedmap.h"
#include "ir.h"
#include "ir_printer.h"

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

inline std::ostream &operator<<(std::ostream &os, const RWExprPair &rwExpr) {
  os << "(";
  if (rwExpr.isReadable()) {
    os << *rwExpr.getReadExpr();
  }
  else {
    os << "none";
  }
  os << ", ";
  if (rwExpr.isWritable()) {
    os << *rwExpr.getWriteExpr();
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
    for (auto &function : functions) {
      delete function.second;
    }
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

  std::map<std::string, ir::Function *> getFunctions() {
    return functions;
  }

  void addElementType(std::shared_ptr<ir::ElementType> elemType) {
    elementTypes[elemType->getName()] = elemType;
  }

  bool containsElementType(const std::string &name) {
    return elementTypes.find(name) != elementTypes.end();
  }

  std::shared_ptr<ir::ElementType> getElementType(const std::string &name) {
    return elementTypes[name];
  }

  void addExtern(const std::shared_ptr<ir::Argument> &externArgument) {
    externs[externArgument->getName()] = externArgument;
  }

  bool containsExtern(const std::string &name) {
    return externs.find(name) != externs.end();
  }

  std::shared_ptr<ir::Argument> getExtern(const std::string &name) {
    return externs[name];
  }

  const std::map<std::string, std::shared_ptr<ir::Argument>> &getExterns() {
    return externs;
  }

  void addTest(ir::Test *test) { tests.push_back(test); }

  const std::vector<ir::Test*> &getTests() const { return tests; }

private:
  std::map<std::string, ir::Function *>                   functions;
  std::map<std::string, std::shared_ptr<ir::Argument>>    externs;

  std::map<std::string, std::shared_ptr<ir::ElementType>> elementTypes;
  ScopedMap<std::string, RWExprPair>                      exprSymtable;

  std::vector<ir::Test*>                                  tests;
};

}} // namespace simit::internal

#endif

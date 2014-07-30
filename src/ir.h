#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <assert.h>
#include <string>

#include "types.h"

namespace simit {

/** The base class of all nodes in the Simit Intermediate Representation
  * (Simit IR) */
class IRNode {
 public:
  IRNode() {}
  IRNode(const std::string &name) : name(name) {}
  virtual ~IRNode() {}

  virtual std::string toString() const = 0;

  void setName(std::string name) { this->name = name; }
  std::string getName() const    { return name; }

  friend std::ostream &operator<<(std::ostream &os, const IRNode &node);

 protected:
  std::string name;
};


/** The base class that represents computed and loaded tensors.  Note that
  * scalars and objects are considered tensors of order 0. */
class Tensor : public IRNode {
 public:
  Tensor(const TensorType *type) : Tensor("", type) {}
  Tensor(const std::string &name, const TensorType *type)
      : IRNode(name), type(type) {}

  virtual const TensorType *getType() const { return type; }
  virtual unsigned int getOrder() const { return type->getOrder(); }

 protected:
  const TensorType *type;
};


/** Represents a  \ref Tensor that is loaded or defined as a constant. */
class LiteralTensor : public Tensor {
 public:
  LiteralTensor(const TensorType *type) : Tensor(type) {}
  virtual ~LiteralTensor() { delete type; }

  void cast(TensorType *type);
};


/** Represents a dense \ref Tensor that is loaded or defined as a constant. */
class DenseLiteralTensor : public LiteralTensor {
 public:
  DenseLiteralTensor(TensorType *type, void *data);
  virtual ~DenseLiteralTensor();

  virtual std::string toString() const;

 private:
  void  *data;
};


class IndexVariable : public IRNode {
 public:
  IndexVariable(const std::string &name) : IRNode(name) {}
  virtual ~IndexVariable() {}
};


class FreeIndexVariable : public IndexVariable {
 public:
  FreeIndexVariable(const std::string &name) : IndexVariable(name) {}
  virtual std::string toString() const { return getName(); }
};

std::list<std::shared_ptr<IndexVariable>> makeFreeIndexVariables(int n);


class ReductionIndexVariable : public IndexVariable {
 public:
  enum Operator {ADD, MUL};

  ReductionIndexVariable(Operator op, const std::string &name)
      : IndexVariable(name), op(op) {}
  virtual std::string toString() const;

 private:
  Operator op;
};


/** Instruction that combines one or more tensors.  Merge nodes must be created
  * through the \ref createMerge factory function. */
class Merge : public Tensor {
 public:
  using IndexVariablePtr = std::shared_ptr<IndexVariable>;

  struct IndexedTensor {
    std::shared_ptr<Tensor> tensor;
    std::list<std::shared_ptr<IndexVariable>> indexVariables;
    IndexedTensor(const std::shared_ptr<Tensor> &tensor,
                  const std::list<Merge::IndexVariablePtr> &indexVars)
        : tensor(tensor), indexVariables(indexVars) {}
    friend std::ostream &operator<<(std::ostream &os, const IndexedTensor &o) {
      return os << o.toString();
    }
    std::string toString() const;
   private:
  };

  enum Operator { NEG, ADD, SUB, MUL, DIV };

  static Merge *make(Operator op,
                     const std::list<IndexVariablePtr> &indexVariables,
                     const std::list<IndexedTensor> &operands);

  virtual TensorType *getTensorType();
  virtual std::string toString() const;

 private:
  Operator op;
  std::list<IndexVariablePtr> indexVariables;
  std::list<IndexedTensor> operands;

  Merge(Operator op, const std::list<IndexVariablePtr> &indexVariables,
        const std::list<IndexedTensor> &operands)
      : Tensor(NULL), op(op), indexVariables(indexVariables),
        operands(operands) { }
};


/** Instruction that stores a value to a tensor or an object. */
class Store : public Tensor {
 public:
  Store(const std::string &name, const TensorType *type)
      : Tensor(name, type) {}
};


/** Instruction that stores a value to a tensor or an object. */
class VariableStore : public Store {
 public:
  VariableStore(const std::string &varName, const TensorType *type)
      : Store(varName, type) {}
  virtual std::string toString() const;
};


/** A formal input or result argument of a function. */
class Formal : public Tensor {
 public:
  Formal(const std::string &name, const TensorType *type)
      : Tensor(name, type) {}
  virtual std::string toString() const;
};


/** A Simit function. */
class Function : public IRNode {
 public:
  typedef std::list<std::shared_ptr<simit::Formal>> FormalList;
  typedef std::list<std::shared_ptr<simit::IRNode>> StatementList;

  Function(const std::string &name, const FormalList &arguments,
           const FormalList &results)
      : IRNode(name), arguments(arguments), results(results) {}

  void addStatements(const std::list<std::shared_ptr<IRNode>> &stmts);

  virtual std::string toString() const;

 private:
  FormalList arguments;
  FormalList results;
  StatementList body;
};


/** A Simit test case. Simit test cases can be declared in language comments
  * and can subsequently be picked up by a test framework. */
class Test : public IRNode {
 public:
  Test(std::string name) : IRNode(name) {}
  virtual ~Test() {}

  virtual std::string toString() const { return "Test"; }

 private:
};


}

#endif

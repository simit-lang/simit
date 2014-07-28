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

  void setName(std::string name) { this->name = name; }
  std::string getName() const    { return name; }

  virtual std::string toString() const = 0;
  friend std::ostream &operator<<(std::ostream &os, const IRNode &node) {
    return os << node.toString();
  }

 protected:
  std::string name;
};


/** The base class of IR nodes that represent computed or loaded values. */
class Value : public IRNode {
 public:
  Value() {}
  Value(const std::string &name) : IRNode(name) {}

  virtual Type *getType() = 0;
};


/** The base class of \ref Value nodes that represent a computed or loaded
  * tensor. */
class Tensor : public Value {
 public:
  virtual unsigned int getOrder() { return getTensorType()->getOrder(); }

  virtual TensorType *getTensorType() = 0;
  virtual Type *getType() { return getTensorType(); }
};


/** Represents a  \ref Tensor that is loaded or defined as a constant. */
class LiteralTensor : public Tensor {
 public:
  LiteralTensor(TensorType *type) : type(type) {}
  virtual ~LiteralTensor() { delete type; }

  void cast(TensorType *type);
  virtual TensorType *getTensorType() { return type; }

 protected:
  TensorType *type;
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


class IndexVariable {
 public:
  IndexVariable(const std::string &name) : name(name) {}
  virtual ~IndexVariable() {}

  std::string getName() const { return name; }
  virtual std::string toString() const = 0;
  friend std::ostream &operator<<(std::ostream &os, const IndexVariable &var) {
    return os << var.toString();
  }

 private:
  std::string name;
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
      : op(op), indexVariables(indexVariables), operands(operands) { }
};


/** A Simit function. */
class Function : public IRNode {
 public:
};


/** Instruction that stores a value to a tensor or an object. */
class Store : public Value {
 public:
  Store(const std::string &name) : Value(name) {}

  virtual Type *getType() { return type; }

 private:
  Type *type;
};


/** Instruction that stores a value to a tensor or an object. */
class VariableStore : public Store {
 public:
  VariableStore(const std::string &varName) : Store(varName) {}
  virtual std::string toString() const;
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

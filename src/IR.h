#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <assert.h>
#include <string>

#include "Types.h"

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


/** The base class of \ref Value nodes that represent a computer or loaded 
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


/** Instruction that combines one or more tensors. */
class Merge : public Tensor {
 public:
  Merge(const std::list<Tensor*> &tensors) : tensors(tensors) {}

  virtual TensorType *getTensorType() { return NULL; }

 private:
  std::list<Tensor*> tensors;
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
class Test : IRNode {
 public:
  Test(std::string name) : IRNode(name) {}
  virtual ~Test() {}

  virtual std::string toString() const { return "Test"; }

 private:
};


}

#endif

#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <assert.h>
#include <string>

#include "Types.h"

namespace simit {

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


class Value : public IRNode {
 public:
  Value() {}
  Value(const std::string &name) : IRNode(name) {}

  virtual Type *getType() = 0;
};


class Tensor : public Value {
 public:
  Tensor(TensorType *type) : type(type) {}
  virtual ~Tensor();

  virtual unsigned int getOrder() { return type->getOrder(); };
  virtual Type *getType() { return type; };

 protected:
  TensorType *type;
};


class LiteralTensor : public Tensor {
 public:
  LiteralTensor(TensorType *type) : Tensor(type) {}

  void cast(TensorType *type);
};


class DenseLiteralTensor : public LiteralTensor {
 public:
  DenseLiteralTensor(TensorType *type, void *data);
  virtual ~DenseLiteralTensor();

  virtual std::string toString() const;

 private:
  void  *data;
};


class Function : public IRNode {
 public:
};


/** An instruction that stores a value to a tensor or an object */
class Store : public Value {
 public:
  Store(const std::string &name) : Value(name) {}

  virtual Type *getType() { return type; }

 private:
  Type *type;
};


/** An instructin that stores a value to a local bariable */
class VariableStore : public Store {
 public:
  VariableStore(const std::string &varName) : Store(varName) {}
  virtual std::string toString() const;
};

}

#endif

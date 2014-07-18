#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <assert.h>
#include <string>

#include "Types.h"

namespace simit {

class IRNode {
 public:
  IRNode() {}
  virtual ~IRNode() {}

  virtual operator std::string() const = 0;

  void setName(std::string name) { this->name = name; }
  std::string getName()          { return this->name; }

 protected:
  std::string name;
};


class Value : public IRNode {
 public:
  Value() {}
  virtual ~Value() {}

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
  virtual ~LiteralTensor() {}

  void cast(TensorType *type);
};


class DenseLiteralTensor : public LiteralTensor {
 public:
  DenseLiteralTensor(TensorType *type, void *data);
  virtual ~DenseLiteralTensor();

  virtual operator std::string() const;

 private:
  void  *data;
};


class Function {
 public:
  Function();
  virtual ~Function();

  virtual operator std::string() const;
};

}

#endif

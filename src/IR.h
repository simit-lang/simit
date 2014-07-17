#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <string>

namespace simit {

class TensorType;

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
};


class Tensor : public Value {
 public:
  Tensor(const TensorType *type) : type(type) {}
  virtual ~Tensor();

 protected:
  const TensorType *type;
};


class LiteralTensor : public Tensor {
 public:
  LiteralTensor(const TensorType *type) : Tensor(type) {}
  virtual ~LiteralTensor() {}
};


class DenseLiteralTensor : public LiteralTensor {
 public:
  DenseLiteralTensor(const TensorType *type, void *data);
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

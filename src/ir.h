#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <cassert>
#include <string>
#include <list>

#include "types.h"
#include "irvisitors.h"

namespace simit {
namespace internal {

class IndexVar;
class FreeIndexVar;
class ReductionIndexVar;


/** The base class of all nodes in the Simit Intermediate Representation
  * (Simit IR) */
class IRNode {
 public:
  IRNode() {}
  IRNode(const std::string &name) : name(name) {}
  virtual ~IRNode();

  void setName(std::string name) { this->name = name; }

  std::string getName() const { return name; }
  virtual void print(std::ostream &os) const = 0;

 protected:
  std::string name;
};

std::ostream &operator<<(std::ostream &os, const IRNode &node);


/** The base IRNode that represents all computed and loaded tensors.  Note that
  * both scalars and elements are considered tensors of order 0. */
class TensorNode : public IRNode {
 public:
  TensorNode(const Type *type) : TensorNode("", type) {}
  TensorNode(const std::string &name, const Type *type)
      : IRNode(name), type(type) {}
  virtual ~TensorNode();

  virtual void accept(IRVisitor *visitor) = 0;

  const Type *getType() const { return type; }
  unsigned int getOrder() const { return type->getOrder(); }
  virtual void print(std::ostream &os) const = 0;

 protected:
  const Type *type;
};


/** Represents a \ref Tensor that is defined as a constant or loaded.  Note
  * that it is only possible to define dense tensor literals.  */
class LiteralTensor : public TensorNode {
 public:
  LiteralTensor(Type *type, void *data);
  ~LiteralTensor();

  void cast(Type *type);
  void accept(IRVisitor *visitor) { visitor->visit(this); };

  void print(std::ostream &os) const;

 private:
  void  *data;
};


/** Instruction that combines one or more tensors.  Merge nodes must be created
  * through the \ref createMerge factory function. */
class IndexExpr : public TensorNode {
 public:
  enum Operator { NEG, ADD, SUB, MUL, DIV };

  using IndexVarPtr = std::shared_ptr<internal::IndexVar>;
  struct IndexedTensor {
    std::shared_ptr<TensorNode> tensor;
    std::vector<IndexVarPtr> indexVars;

    IndexedTensor(const std::shared_ptr<TensorNode> &tensor,
                  const std::vector<IndexExpr::IndexVarPtr> &indexVars)
        : tensor(tensor), indexVars(indexVars) {}
    friend std::ostream &operator<<(std::ostream &os, const IndexedTensor &o) {
      return os << o.toString();
    }
    std::string toString() const;
  };

  IndexExpr(const std::vector<IndexVarPtr> &indexVars,
            Operator op, const std::vector<IndexedTensor> &operands);

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const std::vector<IndexVarPtr> &getDomain() const;

  // TODO: Fix this interface by making IndexedTensor a class that is a part
  //       of Merge's interface, or by returning the tensor operands put
  //       together in a list, or by storing tensors and their indexvars
  //       separately. We shoudln't return a struct.
  const std::vector<IndexedTensor> &getOperands() const { return operands; }
  void print(std::ostream &os) const;

 private:
  std::vector<IndexVarPtr> indexVars;
  Operator op;
  std::vector<IndexedTensor> operands;
};


/** Instruction that stores a value to a tensor or an object. */
class Store : public TensorNode {
 public:
  Store(const std::string &name, const Type *type)
      : TensorNode(name, type) {}
};


/** Instruction that stores a value to a tensor or an object. */
// TODO: Remove this class (move it into parser and don't inherit from tensor)
class VariableStore : public Store {
 public:
  VariableStore(const std::string &varName, const Type *type)
      : Store(varName, type) {}

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  void print(std::ostream &os) const;
};


/** A formal argument to a function. */
class Argument : public TensorNode {
 public:
  Argument(const std::string &name, const Type *type)
      : TensorNode(name, type) {}

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  void print(std::ostream &os) const;
};


/** A formal result of a function. */
class Result : public TensorNode {
 public:
  Result(const std::string &name, const Type *type)
      : TensorNode(name, type) {}

  void setValue(const std::shared_ptr<TensorNode> &value) {
    this->value = value;
  }
  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const std::shared_ptr<TensorNode> &getValue() const { return value; }
  void print(std::ostream &os) const;

 private:
  std::shared_ptr<TensorNode> value;
};

/** A Simit function. */
class Function : public IRNode {
 public:
  Function(const std::string &name,
           const std::vector<std::shared_ptr<Argument>> &arguments,
           const std::vector<std::shared_ptr<Result>> &results)
      : IRNode(name), arguments(arguments), results(results) {}

  void addStatements(const std::vector<std::shared_ptr<IRNode>> &stmts);


  const std::vector<std::shared_ptr<Argument>> &getArguments() const {
    return arguments;
  }

  const std::vector<std::shared_ptr<Result>> &getResults() const {
    return results;
  }

  const std::vector<std::shared_ptr<IRNode>> &getBody() const {
    return body;
  }

  void print(std::ostream &os) const;

 private:
  std::vector<std::shared_ptr<Argument>> arguments;
  std::vector<std::shared_ptr<Result>> results;
  std::vector<std::shared_ptr<IRNode>> body;
};


/** A Simit test case. Simit test cases can be declared in language comments
  * and can subsequently be picked up by a test framework. */
class Test : public IRNode {
 public:
  Test(std::string name) : IRNode(name) {}

  void print(std::ostream &os) const;

 private:
};

}} // namespace simit::internal

#endif

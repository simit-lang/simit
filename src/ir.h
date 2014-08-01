#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <assert.h>
#include <string>

#include "types.h"
#include "irvisitors.h"

namespace simit {

namespace internal {
class IndexVariable;
class FreeIndexVariable;
class ReductionIndexVariable;
}

/** The base class of all nodes in the Simit Intermediate Representation
  * (Simit IR) */
class IRNode {
 public:
  IRNode() {}
  IRNode(const std::string &name) : name(name) {}
  virtual ~IRNode() {}

  void setName(std::string name) { this->name = name; }

  std::string getName() const { return name; }

  virtual void print(std::ostream &os) const = 0;

 protected:
  std::string name;
};

std::ostream &operator<<(std::ostream &os, const IRNode &node);


/** The base class that represents all computed and loaded tensors.  Note that
  * both scalars and elements are considered tensors of order 0. */
class Tensor : public IRNode {
 public:
  Tensor(const TensorType *type) : Tensor("", type) {}
  Tensor(const std::string &name, const TensorType *type)
      : IRNode(name), type(type) {}

  const TensorType *getType() const { return type; }
  unsigned int getOrder() const { return type->getOrder(); }
  void print(std::ostream &os) const = 0;

 protected:
  const TensorType *type;
};


/** Represents a \ref Tensor that is defined as a constant or loaded.  Note
  * that it is only possible to define dense tensor literals.  */
class LiteralTensor : public Tensor {
 public:
  LiteralTensor(TensorType *type, void *data);
  ~LiteralTensor();

  void cast(TensorType *type);

  void print(std::ostream &os) const;

 private:
  void  *data;
};


/** Instruction that combines one or more tensors.  Merge nodes must be created
  * through the \ref createMerge factory function. */
class Merge : public Tensor {
 public:
  using IndexVariablePtr = std::shared_ptr<internal::IndexVariable>;

  struct IndexedTensor {
    std::shared_ptr<Tensor> tensor;
    std::list<IndexVariablePtr> indexVariables;
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

  void print(std::ostream &os) const;

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
  void print(std::ostream &os) const;
};


/** A formal argument to a function. */
class Argument : public Tensor {
 public:
  Argument(const std::string &name, const TensorType *type)
      : Tensor(name, type) {}
  void print(std::ostream &os) const;
};


/** A formal result of a function. */
class Result : public Tensor {
 public:
  Result(const std::string &name, const TensorType *type)
      : Tensor(name, type) {}
  void print(std::ostream &os) const;
};

/** A Simit function. */
class Function : public IRNode {
 public:
  Function(const std::string &name,
           const std::list<std::shared_ptr<simit::Argument>> &arguments,
           const std::list<std::shared_ptr<simit::Result>> &results)
      : IRNode(name), arguments(arguments), results(results) {}

  void addStatements(const std::list<std::shared_ptr<IRNode>> &stmts);


  const std::list<std::shared_ptr<simit::Argument>> &getArguments() const {
    return arguments;
  }

  const std::list<std::shared_ptr<simit::Result>> &getResults() const {
    return results;
  }

  const std::list<std::shared_ptr<simit::IRNode>> &getBody() const {
    return body;
  }

  void print(std::ostream &os) const;

 private:
  std::list<std::shared_ptr<simit::Argument>> arguments;
  std::list<std::shared_ptr<simit::Result>> results;
  std::list<std::shared_ptr<simit::IRNode>> body;
};


/** A Simit test case. Simit test cases can be declared in language comments
  * and can subsequently be picked up by a test framework. */
class Test : public IRNode {
 public:
  Test(std::string name) : IRNode(name) {}

  void print(std::ostream &os) const;

 private:
};
}

#endif

#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <cassert>
#include <string>
#include <list>

#include "interfaces.h"
#include "types.h"
#include "irvisitors.h"

namespace simit {
namespace internal {

/// The base class of all nodes in the Simit Intermediate Representation
/// (Simit IR)
class IRNode : simit::util::Uncopyable {
public:
  IRNode() {}
  IRNode(const std::string &name) : name(name) {}
  virtual ~IRNode();

  void setName(std::string name) { this->name = name; }

  std::string getName() const { return name; }
  virtual void print(std::ostream &os) const = 0;

private:
  std::string name;
};
std::ostream &operator<<(std::ostream &os, const IRNode &node);


/// The base IRNode that represents all computed and loaded tensors.  Note that
/// both scalars and elements are considered tensors of order 0.
class TensorNode : public IRNode {
public:
  TensorNode(const TensorType *type) : TensorNode("", type) {}
  TensorNode(const std::string &name, const TensorType *type)
      : IRNode(name), type(type) {}
  virtual ~TensorNode();

  void setType(const TensorType *type) {
    delete this->type;
    this->type = type;
  }

  const TensorType *getType() const { return type; }
  unsigned int getOrder() const { return type->getOrder(); }

  virtual void accept(IRVisitor *visitor) = 0;
  virtual void print(std::ostream &os) const = 0;

protected:
  const TensorType *type;
};


/// Represents a \ref Tensor that is defined as a constant or loaded.  Note
/// that it is only possible to define dense tensor literals.
class Literal : public TensorNode {
public:
  Literal(TensorType *type);
  Literal(TensorType *type, void *values);
  ~Literal();

  void clear();
  void cast(TensorType *type);
  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const void *getData() const { return data; }
  void print(std::ostream &os) const;

private:
  void  *data;
  int dataSize;
};
bool operator==(const Literal& l, const Literal& r);
bool operator!=(const Literal& l, const Literal& r);


/// An index variable describes iteration over an index set.  There are two
/// types of index variables, free index variables and reduction index
/// variables and both types are represented by the IndexVar class.
///
/// Free index variables simply describe iteration across an index set and do
/// not have a reduction operation (op=FREE).
///
/// Reduction variables have an associated reduction operation that is
/// performed for each index in the index set.  Examples are SUM, which not
/// surprisingly sums over the index variable (\sum_{i} in latex speak) and
/// product which takes the product over the index variable (\prod_{i}).
class IndexVar {
public:
  enum Operator {FREE, SUM, PRODUCT};
  static std::string operatorSymbol(Operator op);
  static std::string operatorString(Operator op);

  IndexVar(const std::string &name) : name(name), op(FREE) {}

  IndexVar(const std::string &name, const IndexSetProduct &indexSet,Operator op)
      : name(name), indexSet(indexSet), op(op) {}

  void setIndexSet(IndexSetProduct indexSet) { this->indexSet = indexSet; }
  void setOperator(IndexVar::Operator op) { this->op = op; }

  const IndexSetProduct &getIndexSet() const { return indexSet; }
  Operator getOperator() const { return op; }

  bool isFreeVariable() { return (op != Operator::FREE); }
  bool isReductionVariable() { return (op != Operator::FREE); }

  std::string getName() const { return name; }

private:
  std::string name;
  IndexSetProduct indexSet;
  Operator op;
};
std::ostream &operator<<(std::ostream &os, const IndexVar &var);


class IndexedTensor {
public:
  IndexedTensor(const std::shared_ptr<TensorNode> &tensor,
                const std::vector<std::shared_ptr<IndexVar>> &indexVariables);

  std::shared_ptr<TensorNode> getTensor() const { return tensor; };
  const std::vector<std::shared_ptr<IndexVar>> &getIndexVariables() const {
    return indexVariables;
  }

private:
  std::shared_ptr<TensorNode>            tensor;
  std::vector<std::shared_ptr<IndexVar>> indexVariables;
};


/// Expression that combines one or more tensors.  Merge nodes must be created
/// through the \ref createMerge factory function.
class IndexExpr : public TensorNode {
public:
  enum Operator { NONE, NEG, ADD, SUB, MUL, DIV };
  static int numOperands(Operator op);

  IndexExpr(const std::vector<std::shared_ptr<IndexVar>> &ivs,
            IndexExpr::Operator op, const std::vector<IndexedTensor> &operands);

  IndexExpr(IndexExpr::Operator op, const std::vector<IndexedTensor> &operands);

  void setIndexVariables(const std::vector<std::shared_ptr<IndexVar>> &ivs);
  void setOperator(IndexExpr::Operator op);
  void setOperands(const std::vector<IndexedTensor> &operands);

  /// Get the domain of the index expression, which is the set index variables
  /// used by any sub-expression.
  std::vector<std::shared_ptr<IndexVar>> getDomain() const;

  /// Get the index variables used to assemble the result of this expression.
  const std::vector<std::shared_ptr<IndexVar>> &getIndexVariables() const {
    return indexVars;
  }

  /// Get the operator that is applied to the operands to compute the result of
  /// this expression.
  IndexExpr::Operator getOperator() const { return op; }

  /// Get the operands inputs to this index expressions.
  const std::vector<IndexedTensor> &getOperands() const { return operands; }

  void accept(IRVisitor *visitor) { visitor->visit(this); };
  void print(std::ostream &os) const;

private:
  std::vector<std::shared_ptr<IndexVar>> indexVars;
  Operator op;
  std::vector<IndexedTensor> operands;

  void initType();
};

std::ostream &operator<<(std::ostream &os, const IndexedTensor &t);


/// Calls a Simit function.
class Call : public TensorNode {
public:
  Call(const std::string &name,
       const std::vector<std::shared_ptr<TensorNode>> &arguments)
      : TensorNode(name, NULL), arguments(arguments) {}

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const std::vector<std::shared_ptr<TensorNode>> &getArguments() const {
    return arguments;
  }
  void print(std::ostream &os) const;

private:
  std::vector<std::shared_ptr<TensorNode>> arguments;
};


/// Instruction that stores a value to a tensor or an object.
class Store : public TensorNode {
public:
  Store(const std::string &name, const TensorType *type)
      : TensorNode(name, type) {}
};


/// Instruction that stores a value to a tensor or an object.
class VariableStore : public Store {
public:
  VariableStore(const std::shared_ptr<TensorNode> &target,
                const std::shared_ptr<TensorNode> &value)
      : Store(target->getName(), new TensorType(*target->getType())),
        target{target}, value{value} {}

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  std::shared_ptr<TensorNode> getTarget() const { return target; }
  std::shared_ptr<TensorNode> getValue() const { return value; }

  void print(std::ostream &os) const;

private:
  std::shared_ptr<TensorNode> target;
  std::shared_ptr<TensorNode> value;
};


/// A formal argument to a function.
class Argument : public TensorNode {
public:
  Argument(const std::string &name, const TensorType *type)
      : TensorNode(name, type) {}

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  void print(std::ostream &os) const;
};


/// A formal result of a function.
class Result : public TensorNode {
public:
  Result(const std::string &name, const TensorType *type)
      : TensorNode(name, type) {}

  void setValue(const std::shared_ptr<TensorNode> &value) {
    assert(*getType() == *value->getType() && "type missmatch");
    this->value = value;
  }
  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const std::shared_ptr<TensorNode> &getValue() const { return value; }
  void print(std::ostream &os) const;

private:
  std::shared_ptr<TensorNode> value;
};


/// A Simit function.
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


/// A Simit test case. Simit test cases can be declared in language comments
/// and can subsequently be picked up by a test framework.
class Test : public IRNode {
public:
  Test(const std::string &callee,
       const std::vector<std::shared_ptr<Literal>> &arguments,
       const std::vector<std::shared_ptr<Literal>> &expected)
      : callee(callee), arguments(arguments), expected(expected) {}

  std::string getCallee() { return callee; }
  std::vector<std::shared_ptr<Literal>> getArguments() { return arguments; }
  std::vector<std::shared_ptr<Literal>> getExpectedResults() { return expected;}

  void print(std::ostream &os) const;

private:
  std::string callee;
  std::vector<std::shared_ptr<Literal>> arguments;
  std::vector<std::shared_ptr<Literal>> expected;
};

}} // namespace simit::internal

#endif

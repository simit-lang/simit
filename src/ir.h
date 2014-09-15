#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <cassert>
#include <string>
#include <list>

#include "interfaces.h"
#include "types.h"
#include "ir_visitors.h"

namespace simit {
namespace ir {

/// The base class of all nodes in the Simit Intermediate Representation
/// (Simit IR)
class IRNode : public simit::interfaces::Printable,
                      simit::interfaces::Uncopyable {
public:
  IRNode() {}
  IRNode(const std::string &name) : name(name) {}
  virtual ~IRNode();

  void setName(std::string name) { this->name = name; }

  std::string getName() const { return name; }

private:
  std::string name;
};


class Expression : public IRNode {
public:
  Expression(const std::shared_ptr<Type> &type) : Expression("", type) {}

  Expression(const std::string &name, const std::shared_ptr<Type> &type)
      : IRNode(name), type(type) {}

  virtual ~Expression() {}

  void setType(const std::shared_ptr<Type> &type) {
    this->type.reset();
    this->type = type;
  }

  const std::shared_ptr<Type> getType() const { return type; }

  virtual void accept(IRVisitor *visitor) = 0;

private:
  std::shared_ptr<Type> type;
};


/// Represents a \ref Tensor that is defined as a constant or loaded.  Note
/// that it is only possible to define dense tensor literals.
class Literal : public Expression {
public:
  Literal(const std::shared_ptr<Type> &type);
  Literal(const std::shared_ptr<Type> &type, void *values);
  ~Literal();

  void clear();
  void cast(const std::shared_ptr<TensorType> &type);
  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const void *getData() const { return data; }

private:
  void  *data;
  int dataSize;

  void print(std::ostream &os) const;
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
  IndexedTensor(const std::shared_ptr<Expression> &tensor,
                const std::vector<std::shared_ptr<IndexVar>> &indexVariables);

  std::shared_ptr<Expression> getTensor() const { return tensor; };
  const std::vector<std::shared_ptr<IndexVar>> &getIndexVariables() const {
    return indexVariables;
  }

private:
  std::shared_ptr<Expression>            tensor;
  std::vector<std::shared_ptr<IndexVar>> indexVariables;
};


/// Expression that combines one or more tensors.  Merge nodes must be created
/// through the \ref createMerge factory function.
class IndexExpr : public Expression {
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

private:
  std::vector<std::shared_ptr<IndexVar>> indexVars;
  Operator op;
  std::vector<IndexedTensor> operands;

  void initType();
  void print(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const IndexedTensor &t);


/// Calls a Simit function.
class Call : public Expression {
public:
  Call(const std::string &name,
       const std::vector<std::shared_ptr<Expression>> &arguments)
      : Expression(name, NULL), arguments(arguments) {}

  void accept(IRVisitor *visitor) { visitor->visit(this); };

  const std::vector<std::shared_ptr<Expression>> &getArguments() const {
    return arguments;
  }

private:
  std::vector<std::shared_ptr<Expression>> arguments;

  void print(std::ostream &os) const;
};


/// Abstract class for expressions that read values from tensors and sets.
class Read : public Expression {
protected:
  Read(const std::string &name, const std::shared_ptr<Type> &type)
      : Expression(name, type) {}
};


namespace {
std::shared_ptr<Type> fieldType(const std::shared_ptr<Expression> &set,
                                const std::string &fieldName) {
  assert(set->getType()->isSet());
  SetType *setType = setTypePtr(set->getType());
  return setType->getElementType()->getFields().at(fieldName);
}
}

/// An expression that reads a tensor from a set field.
class FieldRead : public Read {
public:
  FieldRead(const std::shared_ptr<Expression> &set,const std::string &fieldName)
    : Read(set->getName()+"."+fieldName, fieldType(set, fieldName)),
      set(set), fieldName(fieldName) {}

  ~FieldRead() {}

  const std::shared_ptr<Expression> &getSet() const { return set; }
  const std::string &getFieldName() const { return fieldName; }

  void accept(IRVisitor *visitor) { visitor->visit(this); };

private:
  std::shared_ptr<Expression> set;
  std::string fieldName;

  void print(std::ostream &os) const;
};


/// An expression that reads a tensor from a tensor location.
class TensorRead : public Read {

};


/// Instruction that stores a value to a tensor or an object.
class Write : public Expression {
public:
  Write(const std::string &name, const std::shared_ptr<Type> &type)
      : Expression(name, type) {}
};


/// An instruction that writes a tensor to a set field.
class FieldWrite : public Write {

};


/// An instruction that writes a tensor to a tensor location.
class TensorWrite : public Write {

};


/// A formal argument to a function.
class Argument : public Expression {
public:
  Argument(const std::string &name, const std::shared_ptr<Type> &type)
      : Expression(name, type) {}
  virtual ~Argument() {};

  virtual void accept(IRVisitor *visitor) { visitor->visit(this); };

private:
  void print(std::ostream &os) const;
};


/// A formal result of a function.
class Result : public Argument {
public:
  Result(const std::string &name, const std::shared_ptr<Type> &type)
      : Argument(name, type) {}

  void setValue(const std::shared_ptr<Expression> &value) {
    assert(*getType() == *value->getType() && "type missmatch");
    this->value = value;
  }

  const std::shared_ptr<Expression> &getValue() const { return value; }

  void accept(IRVisitor *visitor) { visitor->visit(this); };

private:
  std::shared_ptr<Expression> value;
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

private:
  std::vector<std::shared_ptr<Argument>> arguments;
  std::vector<std::shared_ptr<Result>> results;
  std::vector<std::shared_ptr<IRNode>> body;

  void print(std::ostream &os) const;
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

  const std::vector<std::shared_ptr<Literal>> &getActuals() {
    return arguments;
  }

  const std::vector<std::shared_ptr<Literal>> &getExpectedResults() {
    return expected;
  }

private:
  std::string callee;
  std::vector<std::shared_ptr<Literal>> arguments;
  std::vector<std::shared_ptr<Literal>> expected;

  void print(std::ostream &os) const;
};

}} // namespace simit::internal

#endif

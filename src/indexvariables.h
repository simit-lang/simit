#ifndef SIMIT_INDEXVARIABLES_H
#define SIMIT_INDEXVARIABLES_H

#include <string>
#include <list>
#include <memory>

namespace simit {
namespace internal {

/** Index variables describe the iteration domains of tensor operations.*/
class IndexVariable {
 public:
  IndexVariable(const std::string &name) : name(name) {}

  std::string getName() const { return name; }
  virtual void print(std::ostream &os) const = 0;

 private:
  std::string name;
};

std::ostream &operator<<(std::ostream &os, const IndexVariable &var);


/** A free index causes an operation to be performed for each location in the
  * iteration domain. */
class FreeIndexVariable : public IndexVariable {
 public:
  FreeIndexVariable(const std::string &name) : IndexVariable(name) {}

  void print(std::ostream &os) const { os << getName(); };
};


/** A reduction index causes the values at each location in the iteration domain
  * to be reduced by the given operation. */
class ReductionIndexVariable : public IndexVariable {
 public:
  enum Operator {ADD, MUL};

  ReductionIndexVariable(Operator op, const std::string &name)
      : IndexVariable(name), op(op) {}

  void print(std::ostream &os) const;

 private:
  Operator op;
};


/** A factory for creating index variables with unique names. */
class IndexVariableFactory {
 public:
  IndexVariableFactory() : nameID(0) {}

  std::list<std::shared_ptr<IndexVariable>> makeFreeVariables(unsigned int n);

  std::shared_ptr<IndexVariable>
  makeReductionVariable(ReductionIndexVariable::Operator op);

 private:
  int nameID;
  std::string makeName();
};

}
}

#endif

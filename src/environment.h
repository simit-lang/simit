#ifndef SIMIT_ENVIRONMENT_H
#define SIMIT_ENVIRONMENT_H

#include <vector>
#include <map>
#include <ostream>
#include "var.h"

namespace simit {
namespace pe {
class PathExpression;
}
namespace ir {
class Expr;
class TensorIndex;

/// An Environment keeps track of global constants, externs and temporaries.
/// It also keeps track of the data arrays and indices that implement tensors
/// as the IR is lowered and tensors are replaced by arrays.
class Environment {
public:
  Environment();
  ~Environment();
  Environment(const Environment& other);
  Environment(Environment&& other) noexcept;
  Environment& operator=(const Environment& other);
  Environment& operator=(Environment&& other) noexcept;

  /// Returns true if the environment is empty, false otherwise.
  bool isEmpty() const;

  /// Get global constants and their initializer expressions.
  const std::vector<std::pair<Var, Expr>>& getConstants() const;

  /// Get global externs. Externs must be bound to a simit::Function before it
  /// is run.
  const std::vector<Var>& getExterns() const;

  /// Get global temporaries. Global temporaries are inserted by compiler
  /// lowering passes, and must be initialized before a simit::Function is run.
  const std::vector<Var>& getTemporaries() const;

//  /// Get the path expression of a tensor variable in the environment.
//  const pe::PathExpression& getPathExpression(const Var& tensorVar) const;
//
//  /// Get the data array of a tensor variable.
//  const Var& getDataArray(const Var& tensorVar) const;

  /// Get the tensor index of a tensor variable.
  const TensorIndex& getTensorIndex(const Var& tensor,
                                    unsigned sourceDim, unsigned sinkDim);

  /// Get a pointer to the allocated memory of the data array of a temporary
  /// tensor variable.
  void* getTemporaryDataPointer(const Var& tensorVar) const;

  /// Get the environment's bindable variables. A bindable variable is a
  /// variable that must be bound by a user. These are initially the same as the
  /// externs, but compiler lowering passes may turn bindable variables into
  /// multiple extern arrays. For example, a bindable sparse tensor can get
  /// turned into one extern data array and two extern index arrays. Use
  /// Environment::getExternsOfBindable to Expr::accept.
  const std::vector<std::string>& getBindableNames() const;

  /// True if a bindable of the given name is part of the environment, false
  /// otherwise.
  bool hasBindable(const std::string& name) const;

  /// Get the bindable variable with the given name. A bindable variable is a
  /// variable that must be bound by a user.
  const Var& getBindable(const std::string& name) const;

  /// Get the extern variables that correspond to a given bindable variable.
  /// See Environment::getBindables.
  const std::vector<Var>& getExternsOfBindable(const Var& bindable) const;

  /// Insert a constant into the environment.
  void addConstant(const Var& var, const Expr& initializer);

  /// Insert an extern into the environement.
  void addExtern(const Var& var, const Var& bindable=Var());

  /// Insert a temporary into the environment.
  void addTemporary(const Var& var);

  /// Associate the tensor variable with a tensor index.
  void addTensorIndex(Var tensor, TensorIndex ti);

private:
  struct Content;
  Content* content;
};

std::ostream& operator<<(std::ostream&, const Environment&);

}}
#endif

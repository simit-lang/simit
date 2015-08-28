#ifndef SIMIT_ENVIRONMENT_H
#define SIMIT_ENVIRONMENT_H

#include <vector>
#include <map>
#include <ostream>

namespace simit {
namespace pe {
class PathExpression;
}
namespace ir {
class Var;
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

  /// Get global constants and their initializer expressions.
  const std::vector<std::pair<Var, Expr>>& getConstants() const;

  /// Get global externs. Externs must be bound to a simit::Function before it
  /// is run.
  const std::vector<Var>& getExterns() const;

  /// Get global temporaries. Global temporaries are inserted by compiler
  /// lowering passes, and must be initialized before a simit::Function is run.
  const std::vector<Var>& getTemporaries() const;

  /// Get the path expression of a tensor variable in the environment.
  const pe::PathExpression& getPathExpression(const Var& tensorVar) const;

  /// Get the data array of a tensor variable.
  const Var& getDataArray(const Var& tensorVar) const;

  /// Get the tensor index of a tensor variable.
  const TensorIndex& getTensorIndex(const Var& tensorVar) const;

  /// Get a pointer to the allocated memory of the data array of a temporary
  /// tensor variable.
  void* getTemporaryDataPointer(const Var& tensorVar) const;

  /// Add a constant to the environment.
  void addConstant(const Var& var, const Expr& initializer);

  /// Add an extern to the environement.
  void addExtern(const Var& var);

  /// Add a temporary to the environment.
  void addTemporary(const Var& var);

private:
  struct Content;
  Content* content;
};

std::ostream& operator<<(std::ostream&, const Environment&);

}}
#endif

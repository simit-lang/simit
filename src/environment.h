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

/// A VarMapping is a mapping from a Var to a vector of Vars that implement it.
/// For example, a dense tensor Var may be implemented by an array, while a
/// sparse matrix Var may be implemented by three arrays storing it's values
/// and indices.
class VarMapping {
public:
  VarMapping(const Var& var) : var(var) {}

  const Var& getVar() const {return var;}
  const std::vector<Var>& getMappings() const {return mappings;}
  void addMapping(const Var& mapping) {mappings.push_back(mapping);}

private:
  Var var;
  std::vector<Var> mappings;
};

std::ostream& operator<<(std::ostream&, const VarMapping&);

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

  /// Get global externs. Externs must be bound before a simit::Function is run.
  /// Each extern consist of a Var together with a vector of the Vars that were
  /// emitted during lowering to implement that Var. For example, a sparse
  /// matrix Var in the original program may be replaced by three array Vars
  /// that store it's values and CSR indices.
  const std::vector<VarMapping>& getExterns() const;

  /// Returns true if the environment has an extern variable of the given name.
  bool hasExtern(const std::string& name) const;

  /// Returns the extern variable with the given name.
  const VarMapping& getExtern(const std::string& name) const;

  /// Returns all the extern variables. In cases where an extern has been mapped
  /// the mapped variables are included in the result. If two variables have
  /// been mapped to the same variable, then the variable is only included once
  /// in the returned vector.
  std::vector<Var> getExternVars() const;

  /// Get global temporaries. Global temporaries are inserted by compiler
  /// lowering passes, and must be initialized before a simit::Function is run.
  const std::vector<Var>& getTemporaries() const;

  /// Returns true if the environment has a temporary of the given name.
  bool hasTemporary(const Var& var) const;

  /// Retrieve all the tensor indices in the environment.
  const std::vector<TensorIndex>& getTensorIndices() const;

  /// True of the environment has a tensor index for the given path expression.
  bool hasTensorIndex(const pe::PathExpression& pexpr) const;

  /// Retrieve the tensor index of the given path expression.
  const TensorIndex& getTensorIndex(const pe::PathExpression& pexpr) const;

  /// True of the environment contains the tensor index of var.
  bool hasTensorIndex(const Var& var) const;

  /// Retrieve the tensor index of var.
  const TensorIndex& getTensorIndex(const Var& var) const;

  /// Insert a constant into the environment.
  void addConstant(const Var& var, const Expr& initializer);

  /// Insert an extern into the environment. Note that externs are bindable by
  /// the user, so they must be unique by name (rather than by identity).
  void addExtern(const Var& var);

  /// Map the extern Var to another Var that implements it. A Var can have
  /// multiple mappings. For example, a sparse matrix Var can be mapped to three
  /// arrays that store its values and CSR indices.
  void addExternMapping(const Var& var, const Var& mapping);

  /// Insert a temporary into the environment.
  void addTemporary(const Var& var);

  /// Add a tensor index described by the given path expression to the
  /// environment, and associate it with var.
  void addTensorIndex(const pe::PathExpression& pexpr, const Var& var);

private:
  struct Content;
  Content* content;
};

std::ostream& operator<<(std::ostream&, const Environment&);

}}
#endif

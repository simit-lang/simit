#ifndef SIMIT_STORAGE_H
#define SIMIT_STORAGE_H

#include <map>
#include <memory>

namespace simit {
namespace pe {
class PathExpression;
}
namespace ir {
class Func;
class Var;
class Stmt;
class Expr;
class TensorIndex;


/// The storage arrangement of a tensor (e.g. dense or stored on a set).
class TensorStorage {
public:
  enum Kind {
    /// Undefined storage.
    Undefined,

    /// The dense tensor stored row major.
    Dense,

    /// A diagonal matrix.
    Diagonal,

    /// A sparse matrix whose non-zero components are accessible through a
    /// tensor index.
    Indexed
  };

  /// Create an undefined tensor storage
  TensorStorage();

  /// Create a tensor storage descriptor.
  TensorStorage(Kind kind);

  /// Retrieve the tensor storage type.
  Kind getKind() const;

  /// True if the tensor is dense, which means all values are stored without an
  /// index.
  bool isDense() const;

  /// True if the tensor is stored on a system, false otherwise.
  bool isSystem() const;

  bool hasPathExpression() const;
  const pe::PathExpression& getPathExpression() const;
  void setPathExpression(const pe::PathExpression& pathExpression);

  bool hasTensorIndex() const;
  const TensorIndex& getTensorIndex() const;
  void setTensorIndex(Var tensor);

private:
  struct Content;
  std::shared_ptr<Content> content;
};
std::ostream &operator<<(std::ostream&, const TensorStorage&);

/// The storage of a set of tensors.
class Storage {
public:
  Storage();

  /// Add storage for a tensor variable.
  void add(const Var &tensor, TensorStorage tensorStorage);

  /// Add the variables from the `other` storage to this storage.
  void add(const Storage &other);

  /// True if the tensor has a storage descriptor, false otherwise.
  bool hasStorage(const Var &tensor) const;

  /// Retrieve the storage of a tensor variable to modify it.
  TensorStorage &getStorage(const Var &tensor);

  /// Retrieve the storage of a tensor variable to inspect it.
  const TensorStorage &getStorage(const Var &tensor) const;

  /// Iterator over storage Vars in this Storage descriptor.
  class Iterator {
  public:
    struct Content;
    Iterator(Content *content);
    ~Iterator();
    const Var &operator*();
    const Var *operator->();
    Iterator& operator++();
    friend bool operator!=(const Iterator&, const Iterator&);
  private:
    Content *content;
  };

  /// Get an iterator pointing to the first Var in this Storage.
  Iterator begin() const;

  /// Get an iterator pointing to the last Var in this Storage.
  Iterator end() const;

private:
  struct Content;
  std::shared_ptr<Content> content;
};
std::ostream &operator<<(std::ostream&, const Storage&);


/// Retrieve a storage descriptor for each tensor used in `func`.
Storage getStorage(const Func &func);

/// Retrieve a storage descriptor for each tensor used in `stmt`.
Storage getStorage(const Stmt &stmt);

/// Adds storage descriptors for each tensor in `func` not already described.
void updateStorage(const Func &func, Storage *storage);

/// Adds storage descriptors for each tensor in `stmt` not already described.
void updateStorage(const Stmt &stmt, Storage *storage);

}}

#endif

#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>
#include <memory>

namespace simit {
namespace pe {
class PathExpression;
}

namespace ir {
class Var;

/// A tensor index is a map source->coordinate->sink described by a path
/// expression.
class TensorIndex {
public:
  TensorIndex() {}
  TensorIndex(std::string name, pe::PathExpression pexpr);

  const std::string getName() const;

  const pe::PathExpression& getPathExpression() const;

  const Var& getCoordArray() const;
  const Var& getSinkArray() const;

  bool defined() const {return content != nullptr;}

private:
  struct Content;
  std::shared_ptr<Content> content;
};

std::ostream& operator<<(std::ostream&, const TensorIndex&);

}}

#endif

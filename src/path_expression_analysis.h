#ifndef SIMIT_PATH_EXPRESSION_ANALYSIS_H
#define SIMIT_PATH_EXPRESSION_ANALYSIS_H

#include <map>
#include <vector>

namespace simit {
class Set;
namespace pe {
class PathExpression;
class Var;
class Set;
}
namespace ir {
class Var;
class Func;
struct IndexExpr;
struct Map;

class PathExpressionBuilder {
public:
  /// Construct a path expression from the given map statement.
  void computePathExpression(const Map* map);

  /// Construct a path expression from the index expressionfor the given target.
  void computePathExpression(Var target, const IndexExpr* iexpr);

  pe::PathExpression getPathExpression(Var target);

  void bind(ir::Var var, const Set* set);

private:
  std::map<ir::Var, pe::PathExpression> pathExpressions;

  // Maps each set variable to its path expression variables to support binding.
  std::map<ir::Var, pe::Set> pathExpressionSets;

  /// Add a path expression to the builder. This path expression that will be
  /// used to compute derived path expressions.
  void addPathExpression(Var target, const pe::PathExpression& pe);

  const pe::Set& getPathExpressionSet(Var irSetVar);
};

/// Associates tensor variables with path expressions. Variables that are not
/// assigned a path expression must be managed in another way, for example,
/// using dense storage or with dynamically updated indices.
std::map<Var, pe::PathExpression> computePathExpressions(const Func& func);

}}
#endif

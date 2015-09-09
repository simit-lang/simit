#ifndef SIMIT_PATH_EXPRESSION_ANALYSIS_H
#define SIMIT_PATH_EXPRESSION_ANALYSIS_H

#include <map>

namespace simit {
namespace pe {
class PathExpression;
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

private:
  std::map<Var, pe::PathExpression> pathExpressions;

  /// Add a path expression to the builder. This path expression that will be
  /// used to compute derived path expressions.
  void addPathExpression(Var target, const pe::PathExpression& pe);
};

/// Associates tensor variables with path expressions. Variables that are not
/// assigned a path expression must be managed in another way, for example,
/// using dense storage or with dynamically updated indices.
std::map<Var, pe::PathExpression> computePathExpressions(const Func& func);

}}
#endif

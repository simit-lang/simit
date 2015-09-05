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

/// Associates tensor variables with path expressions. Variables that are not
/// assigned a path expression must be managed in another way, for example,
/// using dense storage or with dynamically updated indices.
std::map<Var, pe::PathExpression> assignPathExpressions(const Func& func);

}}
#endif

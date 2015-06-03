#ifndef SIMIT_PATH_EXPRESSION_ANALYSIS_H
#define SIMIT_PATH_EXPRESSION_ANALYSIS_H

#include <map>

namespace simit {
namespace ir {

class Var;
class PathExpression;
class Func;

std::map<Var, PathExpression> getPathExpressions(const Func &func);

std::ostream &operator<<(std::ostream &, const PathExpression &);

}}

#endif

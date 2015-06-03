#include "path_expression_analysis.h"

#include "path_expressions.h"
#include "var.h"
#include "ir.h"
#include "ir_visitor.h"

namespace simit {
namespace ir {

class PathExpressionVisitor : public IRVisitor {
public:
  std::map<Var, PathExpression> pathExpressions;

};

std::map<Var, PathExpression> getPathExpressions(const Func &func) {
  PathExpressionVisitor peVisitor;
  func.accept(&peVisitor);
  return peVisitor.pathExpressions;
}

std::ostream &operator<<(std::ostream &os, const PathExpression &pe) {
  os << "pe";
  return os;
}

}}

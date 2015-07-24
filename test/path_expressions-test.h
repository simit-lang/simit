#ifndef SIMIT_PATH_EXPRESSIONS_TEST_H
#define SIMIT_PATH_EXPRESSIONS_TEST_H

#include "path_expressions.h"

using namespace std;
using namespace simit::pe;

static PathExpression makeEV(const string &eName="e", const string &vName="v") {
  Var e = Var(eName);
  Var v = Var(vName);
  PathExpression ev = Link::make(e, v, Link::ev);
  return ev;
}

static PathExpression makeVE(const string &vName="v", const string &eName="e") {
  Var v = Var(vName);
  Var e = Var(eName);
  PathExpression ve = Link::make(v, e, Link::ve);
  return ve;
}

#endif

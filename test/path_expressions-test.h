#ifndef SIMIT_PATH_EXPRESSIONS_TEST_H
#define SIMIT_PATH_EXPRESSIONS_TEST_H

#include "path_expressions.h"

using namespace std;
using namespace simit::pe;

PathExpression makeEV(const string &eName="", const string &vName="") {
  Var e = Var(eName);
  Var v = Var(vName);
  PathExpression ev = Link::make(e, v, Link::ev);
  return ev;
}

PathExpression makeVE(const string &vName="", const string &eName="") {
  Var v = Var(vName);
  Var e = Var(eName);
  PathExpression ve = Link::make(v, e, Link::ve);
  return ve;
}

#endif

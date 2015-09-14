#ifndef SIMIT_PATH_EXPRESSIONS_TEST_H
#define SIMIT_PATH_EXPRESSIONS_TEST_H

#include "path_expressions.h"

using namespace std;
using namespace simit::pe;

static PathExpression makeEV(const string &eName="e", const string &vName="v") {
  Var e(eName, simit::pe::Set("E"));
  Var v(vName, simit::pe::Set("V"));
  return Link::make(e, v, Link::ev);
}

static PathExpression makeVE(const string &vName="v", const string &eName="e") {
  Var v(vName, simit::pe::Set("V"));
  Var e(eName, simit::pe::Set("E"));
  return Link::make(v, e, Link::ve);
}

#endif

#ifndef SIMIT_PATH_EXPRESSIONS_TEST_H
#define SIMIT_PATH_EXPRESSIONS_TEST_H

#include "path_expressions.h"

using namespace std;
using namespace simit::pe;

static PathExpression makeEV(string e="e", string E="E",
                             string v="v", string V="V") {
  Var eVar(e, simit::pe::Set(E));
  Var vVar(v, simit::pe::Set(V));
  return Link::make(eVar, vVar, Link::ev);
}

static PathExpression makeVE(string v="v", string V="V",
                             string e="e", string E="E") {
  Var vVar(v, simit::pe::Set(V));
  Var eVar(e, simit::pe::Set(E));
  return Link::make(vVar, eVar, Link::ve);
}

#endif

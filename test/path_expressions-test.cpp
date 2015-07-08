#include "gtest/gtest.h"

#include "path_expressions.h"
#include "types.h"

using namespace std;
using namespace simit::pe;

TEST(PathExpression, EV) {
  Var e = Var("E");
  Var v = Var("V");
  PathExpression ev = EV::make(e, v);
  ASSERT_EQ(ev.getPathEndpoint(0), e);
  ASSERT_EQ(ev.getPathEndpoint(1), v);
}

//TEST(PathExpression, VE) {}
//TEST(PathExpression, VEV) {}

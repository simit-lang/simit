#include "gtest/gtest.h"

#include "path_expressions.h"
#include "types.h"

using namespace std;
using namespace simit::pe;

TEST(PathExpression, EV) {
  ElementVar e = ElementVar("E");
  ElementVar v = ElementVar("V");
  PathExpression ev = EV::make(e, v, 0);
  ASSERT_EQ(ev.getPathEndpoint(0), e);
  ASSERT_EQ(ev.getPathEndpoint(1), v);
}

//TEST(PathExpression, VE) {}
//TEST(PathExpression, VEV) {}

#include "gtest/gtest.h"

#include "path_expressions.h"
#include "types.h"

using namespace std;
using namespace simit::pe;

TEST(PathExpression, EV) {
  ElementVar v = ElementVar("V");
  ElementVar e = ElementVar("E");

  PathExpression ev = Link::make(e, v, 0);

}

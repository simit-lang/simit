#include "gtest/gtest.h"

#include "path_expressions.h"
#include "types.h"

using namespace std;
using namespace simit::pe;

TEST(PathExpression, EV) {
  ElementVar e = ElementVar("E");
  ElementVar v = ElementVar("V");
  PathExpression ev = EV::make(e, v, 0);

  PathExpression::Path path = ev.getPath();
  ASSERT_EQ(2u, path.size());

  std::vector<ElementVar> vars = {e, v};
  unsigned i = 0;
  for (auto &var : path) {
    ASSERT_EQ(vars[i++], var);
  }
}

//TEST(PathExpression, VE) {}
//TEST(PathExpression, VEV) {}

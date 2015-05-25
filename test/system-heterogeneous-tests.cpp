#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "error.h"
#include "types.h"

using namespace std;
using namespace simit;

TEST(System, DISABLED_heterogeneous_vector) {
  Set V;
  Set U;
  Set E(V, U);

  FieldRef<simit_float> Vx = V.addField<simit_float>("x");
  FieldRef<simit_float> Ux = U.addField<simit_float>("x");
  FieldRef<simit_float> Ex = U.addField<simit_float>("x");

  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  ElementRef v3 = V.add();
  Vx(v1) = 2.0;
  Vx(v2) = 3.0;
  Vx(v3) = 7.0;

  ElementRef u1 = U.add();
  ElementRef u2 = U.add();

  Function func = getFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("U", &U);
  func.bind("E", &E);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(5.0, Ux(u1));
  SIMIT_EXPECT_FLOAT_EQ(7.0, Ux(u2));
}

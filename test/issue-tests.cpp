#include "simit-test.h"

#include "init.h"
#include "graph.h"
#include "tensor.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(DISABLED_issue, 45) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int,2> a = V.addField<int,2>("a");
  FieldRef<int,2> b = V.addField<int,2>("b");
  a(v0) = {1, 2};
  a(v1) = {3, 4};
  a(v2) = {5, 6};

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(1, (int)b(v0)(0));
  ASSERT_EQ(0, (int)b(v0)(1));
  ASSERT_EQ(3, (int)b(v1)(0));
  ASSERT_EQ(0, (int)b(v1)(1));
  ASSERT_EQ(0, (int)b(v2)(0));
  ASSERT_EQ(0, (int)b(v2)(1));
}

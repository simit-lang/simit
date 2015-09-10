#include "simit-test.h"

#include "graph.h"

using namespace std;
using namespace simit;

TEST(DISABLED_System, add) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b.set(v0, 1.0);
  b.set(v1, 2.0);
  b.set(v2, 3.0);

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e.set(e0, 1.0);
  e.set(e1, 2.0);

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f.set(f0, 4.0);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  std::cout << a << std::endl;

  // Check that outputs are correct
  ASSERT_EQ(15.0, a.get(v0));
  ASSERT_EQ(13.0, a.get(v1));
  ASSERT_EQ(14.0, a.get(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(v0));
  ASSERT_EQ(2.0, b.get(v1));
  ASSERT_EQ(3.0, b.get(v2));
}

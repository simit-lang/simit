#include "simit-test.h"

#include "graph.h"

using namespace std;
using namespace simit;

TEST(System, add) {
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

  // Check that outputs are correct
  ASSERT_EQ(19.0, a.get(v0));
  ASSERT_EQ(13.0, a.get(v1));
  ASSERT_EQ(26.0, a.get(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(v0));
  ASSERT_EQ(2.0, b.get(v1));
  ASSERT_EQ(3.0, b.get(v2));
}

TEST(DISABLED_System, gemm_simple) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Taint c
  c.set(p0, 22.0);
  c.set(p1, 35.0);
  c.set(p2, 42.0);

  // Springs
  Set springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check that inputs are preserved
  //ASSERT_EQ(1.0, b.get(p0));
  //ASSERT_EQ(2.0, b.get(p1));
  //ASSERT_EQ(3.0, b.get(p2));

  // Check that outputs are correct
  //ASSERT_EQ(3.0, c.get(p0));
  //ASSERT_EQ(13.0, c.get(p1));
  //ASSERT_EQ(10.0, c.get(p2));
}

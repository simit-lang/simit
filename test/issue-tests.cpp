#include "simit-test.h"

#include "init.h"
#include "graph.h"
#include "tensor.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(DISABLED_issue, 44) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  // Springs
  Set springs(points,{3,2}); // rectangular lattice
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  // Build points
  ElementRef p00 = springs.getLatticePoint({0,0});
  ElementRef p01 = springs.getLatticePoint({1,0});
  ElementRef p02 = springs.getLatticePoint({2,0});
  ElementRef p10 = springs.getLatticePoint({0,1});
  ElementRef p11 = springs.getLatticePoint({1,1});
  ElementRef p12 = springs.getLatticePoint({2,1});

  b.set(p00, 1.0);
  b.set(p01, 2.0);
  b.set(p02, 3.0);
  b.set(p10, 4.0);
  b.set(p11, 5.0);
  b.set(p12, 6.0);

  // Taint c
  c.set(p00, 42.0);
  c.set(p12, 42.0);


  // Build springs
  ElementRef s000 = springs.getLatticeLink({0,0},0);
  ElementRef s001 = springs.getLatticeLink({1,0},0);
  ElementRef s002 = springs.getLatticeLink({2,0},0);
  ElementRef s010 = springs.getLatticeLink({0,1},0);
  ElementRef s011 = springs.getLatticeLink({1,1},0);
  ElementRef s012 = springs.getLatticeLink({2,1},0);
  ElementRef s100 = springs.getLatticeLink({0,0},1);
  ElementRef s101 = springs.getLatticeLink({1,0},1);
  ElementRef s102 = springs.getLatticeLink({2,0},1);
  ElementRef s110 = springs.getLatticeLink({0,1},1);
  ElementRef s111 = springs.getLatticeLink({1,1},1);
  ElementRef s112 = springs.getLatticeLink({2,1},1);

  a.set(s000, 1.0);
  a.set(s001, 2.0);
  a.set(s002, 3.0);
  a.set(s010, 4.0);
  a.set(s011, 5.0);
  a.set(s012, 6.0);
  a.set(s100, 7.0);
  a.set(s101, 8.0);
  a.set(s102, 9.0);
  a.set(s110, 10.0);
  a.set(s111, 11.0);
  a.set(s112, 12.0);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(p00));
  ASSERT_EQ(2.0, b.get(p01));
  ASSERT_EQ(3.0, b.get(p02));
  ASSERT_EQ(4.0, b.get(p10));
  ASSERT_EQ(5.0, b.get(p11));
  ASSERT_EQ(6.0, b.get(p12));

  // Check that outputs are correct
  ASSERT_EQ(100.0, (simit_float)c.get(p00));
  ASSERT_EQ(146.0, (simit_float)c.get(p01));
  ASSERT_EQ(211.0, (simit_float)c.get(p02));
  ASSERT_EQ(181.0, (simit_float)c.get(p10));
  ASSERT_EQ(224.0, (simit_float)c.get(p11));
  ASSERT_EQ(304.0, (simit_float)c.get(p12));
}

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

TEST(DISABLED_issue, 53) {
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  int a[2] = {1,2};
  int b[2];
  func.bind("a", a);
  func.bind("b", b);
  func.runSafe();
  ASSERT_EQ(2, b[0]);
  ASSERT_EQ(4, b[1]);
}

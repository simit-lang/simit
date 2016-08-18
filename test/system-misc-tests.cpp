#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(system, swap) {
  Set V;
  FieldRef<simit_float> val = V.addField<simit_float>("val");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  ElementRef v3 = V.add();
  val.set(v0, 1.0);
  val.set(v1, 2.0);
  val.set(v2, 3.0);
  val.set(v3, 4.0);

  Set E(V,V);
  E.add(v0,v1);
  E.add(v2,v3);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);

  func.runSafe();

  // Check that inputs are swapped appropriately
  ASSERT_EQ(2.0, val.get(v0));
  ASSERT_EQ(1.0, val.get(v1));
  ASSERT_EQ(4.0, val.get(v2));
  ASSERT_EQ(3.0, val.get(v3));
}

TEST(system, element_field_access_in_proc) {
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
  e.set(e0, 4.0);
  e.set(e1, 5.0);
  
  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);

  func.runSafe();
  
  ASSERT_EQ(33.0, a.get(v0));
}

TEST(system, slice) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");
  FieldRef<simit_float> d = points.addField<simit_float>("d");
  
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Taint c
  c.set(p0, 42.0);
  c.set(p2, 42.0);
  
  // Taint d
  d.set(p0, 55.0);
  d.set(p2, 55.0);

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

  ASSERT_EQ(1.0, b.get(p0));
  ASSERT_EQ(2.0, b.get(p1));
  ASSERT_EQ(3.0, b.get(p2));
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(3.0, c.get(p1));
  ASSERT_EQ(2.0, c.get(p2));
  ASSERT_EQ(0.0, d.get(p0));
  ASSERT_EQ(2.0, d.get(p1));
  ASSERT_EQ(2.0, d.get(p2));
}

TEST(DISABLED_system, transpose) {
  Set V;
  FieldRef<int> b = V.addField<int>("b");
  FieldRef<int> c = V.addField<int>("c");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1;
  b(v1) = 2;
  b(v2) = 3;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(4,  c(v0));
  ASSERT_EQ(10, c(v1));
  ASSERT_EQ(4,  c(v2));
}

TEST(DISABLED_system, transpose_rectangular) {
  Set V;
  FieldRef<int> b = V.addField<int>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();

  Set E(V);
  ElementRef e0 = E.add(v0);
  ElementRef e1 = E.add(v2);
  FieldRef<int> a = E.addField<int>("a");
  a(e0) = 1;
  a(e1) = 2;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(1.0, b(v0));
  ASSERT_EQ(0.0, b(v1));
  ASSERT_EQ(2.0, b(v2));
}

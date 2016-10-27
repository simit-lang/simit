#include "simit-test.h"

#include "init.h"
#include "graph.h"

using namespace std;
using namespace simit;

TEST(system, add) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(19.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(26.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, add_swapped_vectors) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(19.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(26.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, add_func) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(19.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(26.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, add_func_results) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(19.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(26.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, add_named_tuples) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(19.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(26.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, add_blocked) {
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

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(6.0, a.get(v0));
  ASSERT_EQ(22.0, a.get(v1));
  ASSERT_EQ(26.0, a.get(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(v0));
  ASSERT_EQ(2.0, b.get(v1));
  ASSERT_EQ(3.0, b.get(v2));
}

TEST(system, add_blocked_inout) {
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

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(6.0, a.get(v0));
  ASSERT_EQ(22.0, a.get(v1));
  ASSERT_EQ(26.0, a.get(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(v0));
  ASSERT_EQ(2.0, b.get(v1));
  ASSERT_EQ(3.0, b.get(v2));
}

TEST(system, add_double_blocked) {
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

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(12.0, a.get(v0));
  ASSERT_EQ(44.0, a.get(v1));
  ASSERT_EQ(52.0, a.get(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(v0));
  ASSERT_EQ(2.0, b.get(v1));
  ASSERT_EQ(3.0, b.get(v2));
}

TEST(system, add_stencil) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  // Springs
  Set springs(points,{3,2}); // rectangular grid
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  // Build points
  ElementRef p00 = springs.getGridPoint({0,0});
  ElementRef p01 = springs.getGridPoint({1,0});
  ElementRef p02 = springs.getGridPoint({2,0});
  ElementRef p10 = springs.getGridPoint({0,1});
  ElementRef p11 = springs.getGridPoint({1,1});
  ElementRef p12 = springs.getGridPoint({2,1});

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
  ElementRef s000 = springs.getGridEdge({0,0},0);
  ElementRef s001 = springs.getGridEdge({1,0},0);
  ElementRef s002 = springs.getGridEdge({2,0},0);
  ElementRef s010 = springs.getGridEdge({0,1},0);
  ElementRef s011 = springs.getGridEdge({1,1},0);
  ElementRef s012 = springs.getGridEdge({2,1},0);
  ElementRef s100 = springs.getGridEdge({0,0},1);
  ElementRef s101 = springs.getGridEdge({1,0},1);
  ElementRef s102 = springs.getGridEdge({2,0},1);
  ElementRef s110 = springs.getGridEdge({0,1},1);
  ElementRef s111 = springs.getGridEdge({1,1},1);
  ElementRef s112 = springs.getGridEdge({2,1},1);

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

  // Build springs 2
  Set springs2(points,points);
  FieldRef<simit_float> a2 = springs2.addField<simit_float>("a");
  ElementRef t0 = springs2.add(p00,p12);
  ElementRef t1 = springs2.add(p10,p02);

  a2.set(t0, 0.5);
  a2.set(t1, 1.5);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);
  func.bind("springs", &springs);
  func.bind("springs2", &springs2);

  func.runSafe();

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(p00));
  ASSERT_EQ(2.0, b.get(p01));
  ASSERT_EQ(3.0, b.get(p02));
  ASSERT_EQ(4.0, b.get(p10));
  ASSERT_EQ(5.0, b.get(p11));
  ASSERT_EQ(6.0, b.get(p12));

  // Check that outputs are correct
  ASSERT_EQ(103.5, (simit_float)c.get(p00));
  ASSERT_EQ(146.0, (simit_float)c.get(p01));
  ASSERT_EQ(221.5, (simit_float)c.get(p02));
  ASSERT_EQ(191.5, (simit_float)c.get(p10));
  ASSERT_EQ(224.0, (simit_float)c.get(p11));
  ASSERT_EQ(307.5, (simit_float)c.get(p12));
}

TEST(system, DISABLED_add_stencil_indexless) {
  // HACK: Set kIndexlessStencils to true for this type of test
  kIndexlessStencils = true;

  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  // Springs
  Set springs(points,{3,2}); // rectangular grid
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  // Build points
  ElementRef p00 = springs.getGridPoint({0,0});
  ElementRef p01 = springs.getGridPoint({1,0});
  ElementRef p02 = springs.getGridPoint({2,0});
  ElementRef p10 = springs.getGridPoint({0,1});
  ElementRef p11 = springs.getGridPoint({1,1});
  ElementRef p12 = springs.getGridPoint({2,1});

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
  ElementRef s000 = springs.getGridEdge({0,0},0);
  ElementRef s001 = springs.getGridEdge({1,0},0);
  ElementRef s002 = springs.getGridEdge({2,0},0);
  ElementRef s010 = springs.getGridEdge({0,1},0);
  ElementRef s011 = springs.getGridEdge({1,1},0);
  ElementRef s012 = springs.getGridEdge({2,1},0);
  ElementRef s100 = springs.getGridEdge({0,0},1);
  ElementRef s101 = springs.getGridEdge({1,0},1);
  ElementRef s102 = springs.getGridEdge({2,0},1);
  ElementRef s110 = springs.getGridEdge({0,1},1);
  ElementRef s111 = springs.getGridEdge({1,1},1);
  ElementRef s112 = springs.getGridEdge({2,1},1);

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

  // Build springs 2
  Set springs2(points,points);
  FieldRef<simit_float> a2 = springs2.addField<simit_float>("a");
  ElementRef t0 = springs2.add(p00,p12);
  ElementRef t1 = springs2.add(p10,p02);

  a2.set(t0, 0.5);
  a2.set(t1, 1.5);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);
  func.bind("springs2", &springs2);

  func.runSafe();

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(p00));
  ASSERT_EQ(2.0, b.get(p01));
  ASSERT_EQ(3.0, b.get(p02));
  ASSERT_EQ(4.0, b.get(p10));
  ASSERT_EQ(5.0, b.get(p11));
  ASSERT_EQ(6.0, b.get(p12));

  // Check that outputs are correct
  ASSERT_EQ(103.5, (simit_float)c.get(p00));
  ASSERT_EQ(146.0, (simit_float)c.get(p01));
  ASSERT_EQ(221.5, (simit_float)c.get(p02));
  ASSERT_EQ(191.5, (simit_float)c.get(p10));
  ASSERT_EQ(224.0, (simit_float)c.get(p11));
  ASSERT_EQ(307.5, (simit_float)c.get(p12));

  kIndexlessStencils = false;
}

TEST(system, add_generics) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(19.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(26.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, add_twice) {
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

TEST(system, add_transpose) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  e(e0) = 1.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(24.0, (double)a(v0));
  ASSERT_EQ(5.0, (double)a(v1));
  ASSERT_EQ(28.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, gemm) {
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
  ASSERT_EQ(1.0, (double)b.get(p0));
  ASSERT_EQ(2.0, (double)b.get(p1));
  ASSERT_EQ(3.0, (double)b.get(p2));

  // Check that outputs are correct
  ASSERT_EQ(18158.0, (double)c.get(p0));
  ASSERT_EQ(22674.0, (double)c.get(p1));
  ASSERT_EQ(25276.0, (double)c.get(p2));
}

TEST(DISABLED_system, gemm_blocked) {
  // Points
  Set points;
  FieldRef<simit_float,2> b = points.addField<simit_float,2>("b");
  FieldRef<simit_float,2> c = points.addField<simit_float,2>("c");
  FieldRef<simit_float,2,2> d = points.addField<simit_float,2,2>("d");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  d.set(p0, {1.0, 2.0, 3.0, 4.0});
  d.set(p1, {2.0, 3.0, 4.0, 5.0});
  d.set(p2, {3.0, 4.0, 5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  Set springs(points,points);
  FieldRef<simit_float,2,2> a = springs.addField<simit_float,2,2>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check that outputs are correct
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  TensorRef<simit_float,2> c0 = c.get(p0);
  ASSERT_EQ(4048.0, c0(0));
  ASSERT_EQ(8288.0, c0(1));

  TensorRef<simit_float,2> c1 = c.get(p1);
  ASSERT_EQ(13502.0, c1(0));
  ASSERT_EQ(21958.0, c1(1));

  TensorRef<simit_float,2> c2 = c.get(p2);
  ASSERT_EQ(16544.0, c2(0));
  ASSERT_EQ(11120.0, c2(1));
}

TEST(system, sub) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  Set F(V,V);
  FieldRef<simit_float> f = F.addField<simit_float>("e");
  ElementRef f0 = F.add(v0,v2);
  f(f0) = 4.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("F", &F);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(-13.0, (double)a(v0));
  ASSERT_EQ(13.0, (double)a(v1));
  ASSERT_EQ(-6.0, (double)a(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, (double)b(v0));
  ASSERT_EQ(2.0, (double)b(v1));
  ASSERT_EQ(3.0, (double)b(v2));
}

TEST(system, sub_blocked) {
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

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("V", &V);
  func.bind("E", &E);

  func.runSafe();

  // Check that outputs are correct
  ASSERT_EQ(-2.0, a.get(v0));
  ASSERT_EQ(-6.0, a.get(v1));
  ASSERT_EQ(10.0, a.get(v2));

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(v0));
  ASSERT_EQ(2.0, b.get(v1));
  ASSERT_EQ(3.0, b.get(v2));
}


#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(System, map_triangle) {
  simit::Set<> verts;
  simit::FieldRef<simit_float> b = verts.addField<simit_float>("b");

  ElementRef v0 = verts.add();
  ElementRef v1 = verts.add();
  ElementRef v2 = verts.add();
  ElementRef v3 = verts.add();

  simit::Set<3> trigs(verts,verts,verts);
  simit::FieldRef<simit_float> a = trigs.addField<simit_float>("a");

  ElementRef t0 = trigs.add(v0,v1,v2);
  ElementRef t1 = trigs.add(v1,v2,v3);

  a.set(t0, 1.0);
  a.set(t1, 0.1);

 // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("verts", &verts);
  f->bind("trigs", &trigs);

  f->runSafe();

  // Check outputs
  ASSERT_SIMIT_FLOAT_EQ(1.0, b.get(v0));
  ASSERT_SIMIT_FLOAT_EQ(1.1, b.get(v1));
  ASSERT_SIMIT_FLOAT_EQ(1.1, b.get(v2));
  ASSERT_SIMIT_FLOAT_EQ(0.1, b.get(v3));
}

TEST(System, map_assemble_from_literal_vector) {
  // Points
  Set<> points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x.set(p0, {0.0});
  x.set(p1, {0.0});

  // Springs
  Set<2> springs(points,points);

  springs.add(p0,p1);
  springs.add(p1,p2);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(1.0, x.get(p0));
  ASSERT_EQ(1.0, x.get(p1));
}

TEST(System, map_assemble_vector_components) {
  // Points
  Set<> points;
  FieldRef<simit_float,2> x = points.addField<simit_float,2>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  // Springs
  Set<2> springs(points,points);

  springs.add(p0,p1);
  springs.add(p1,p2);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(42.0, x.get(p0)(0));
  ASSERT_EQ(0.0,  x.get(p0)(1));
  ASSERT_EQ(42.0,  x.get(p1)(0));
  ASSERT_EQ(0.0, x.get(p1)(1));
  ASSERT_EQ(0.0,  x.get(p2)(0));
  ASSERT_EQ(0.0, x.get(p2)(1));
}

TEST(System, DISABLED_map_assemble_fem) {
  // Points
  Set<> points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x.set(p0, {1.0});
  x.set(p1, {1.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<simit_float> u = springs.addField<simit_float>("u");

  springs.add(p0,p1);
  springs.add(p1,p2);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(1.0, x.get(p0));
  ASSERT_EQ(1.0, x.get(p1));
}

TEST(System, map_one_set) {
  // Points
  Set<> points;
  FieldRef<simit_float> a = points.addField<simit_float>("a");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->runSafe();

  // Check outputs
  ASSERT_SIMIT_FLOAT_EQ(2, a.get(p0));
  ASSERT_SIMIT_FLOAT_EQ(4, a.get(p1));
  ASSERT_SIMIT_FLOAT_EQ(6, a.get(p2));
}

TEST(System, DISABLED_map_one_set_const_ref) {
  // Points
  Set<> points;
  FieldRef<simit_float> a = points.addField<simit_float>("a");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->runSafe();

  // Check outputs
  ASSERT_SIMIT_FLOAT_EQ(0.5, a.get(p0));
  ASSERT_SIMIT_FLOAT_EQ(1.0, a.get(p1));
  ASSERT_SIMIT_FLOAT_EQ(1.5, a.get(p2));
}

TEST(System, map_no_results_one_set) {
  // Points
  Set<> points;
  FieldRef<simit_float> a = points.addField<simit_float>("a");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(2.0, (simit_float)a.get(p0));
  ASSERT_EQ(4.0, (simit_float)a.get(p1));
  ASSERT_EQ(6.0, (simit_float)a.get(p2));
}

TEST(System, map_no_results_two_sets) {
  // Points
  Set<> points;
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  // Springs
  Set<2> springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(2.0, (simit_float)a.get(s0));
  ASSERT_EQ(4.0, (simit_float)a.get(s1));
}

TEST(System, map_two_results_one_set) {
  // Points
  Set<> points;
  FieldRef<simit_float> a = points.addField<simit_float>("a");
  FieldRef<simit_float> b = points.addField<simit_float>("b");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ( 6.0, (simit_float)b.get(p0));
  ASSERT_EQ(24.0, (simit_float)b.get(p1));
  ASSERT_EQ(54.0, (simit_float)b.get(p2));
}

TEST(System, map_two_results_two_sets) {
  // Points
  Set<> points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  // Springs
  Set<2> springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(43.0, (simit_float)b.get(p0));
  ASSERT_EQ(177.0, (simit_float)b.get(p1));
  ASSERT_EQ(168.0, (simit_float)b.get(p2));
}

TEST(System, map_edgeset_no_endpoints) {
  // Points
  Set<> points;
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  // Springs
  Set<2> springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(2.0, (simit_float)a.get(s0));
  ASSERT_EQ(4.0, (simit_float)a.get(s1));
}

TEST(System, map_edgeset_no_endpoints_results) {
  // Points
  Set<> points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  // Springs
  Set<2> springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(2.0, (simit_float)a.get(s0));
  ASSERT_EQ(4.0, (simit_float)a.get(s1));
}

TEST(System, slice) {
  // Points
  Set<> points;
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
  Set<2> springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

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

TEST(System, map_norm) {
  Set<> points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  FieldRef<simit_float> y = points.addField<simit_float>("y");
  
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x.set(p0, {1.0, 2.0, 3.0});
  x.set(p1, {4.0, 5.0, 6.0});
  x.set(p2, {7.0, 8.0, 9.0});

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();
  f->bind("points", &points);
  f->runSafe();

  ASSERT_SIMIT_FLOAT_EQ(3.74165738677394132949,  (simit_float)y(p0));
  ASSERT_SIMIT_FLOAT_EQ(8.77496438739212258895,  (simit_float)y(p1));
  ASSERT_SIMIT_FLOAT_EQ(13.92838827718411920387, (simit_float)y(p2));
}

TEST(System, map_pass_field) {
  Set<> points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  FieldRef<simit_float> y = points.addField<simit_float>("y");
  
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x.set(p0, {1.0, 2.0, 3.0});
  x.set(p1, {4.0, 5.0, 6.0});
  x.set(p2, {7.0, 8.0, 9.0});

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();
  f->bind("points", &points);
  f->runSafe();

  ASSERT_SIMIT_FLOAT_EQ(3.74165738677394132949,  (simit_float)y(p0));
  ASSERT_SIMIT_FLOAT_EQ(8.77496438739212258895,  (simit_float)y(p1));
  ASSERT_SIMIT_FLOAT_EQ(13.92838827718411920387, (simit_float)y(p2));
}


TEST(System, DISABLED_map_vec_assign) {
  Set<> points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x.set(p0, {1.0, 2.0, 3.0});
  x.set(p1, {4.0, 5.0, 6.0});
  x.set(p2, {7.0, 8.0, 9.0});

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->runSafe();

  ASSERT_SIMIT_FLOAT_EQ(1.1, x(p0)(0));
  ASSERT_SIMIT_FLOAT_EQ(2.1, x(p0)(1));
  ASSERT_SIMIT_FLOAT_EQ(3.1, x(p0)(2));
  ASSERT_SIMIT_FLOAT_EQ(4.1, x(p1)(0));
  ASSERT_SIMIT_FLOAT_EQ(5.1, x(p1)(1));
  ASSERT_SIMIT_FLOAT_EQ(6.1, x(p1)(2));
  ASSERT_SIMIT_FLOAT_EQ(7.1, x(p2)(0));
  ASSERT_SIMIT_FLOAT_EQ(8.1, x(p2)(1));
  ASSERT_SIMIT_FLOAT_EQ(9.1, x(p2)(2));
}

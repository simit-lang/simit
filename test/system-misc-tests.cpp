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

TEST(System, cg) {
  Set<> points;
  FieldRef<simit_float>  b = points.addField<simit_float>("b");
  FieldRef<simit_float>  c = points.addField<simit_float>("c");
  FieldRef<int>    id = points.addField<int>("id");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  Set<2> springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 4.0);
  a.set(s1, 5.0);

  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  ASSERT_SIMIT_FLOAT_EQ(0.95883777239709455653, (simit_float)c.get(p0));
  ASSERT_SIMIT_FLOAT_EQ(1.98789346246973352983, (simit_float)c.get(p1));
  ASSERT_SIMIT_FLOAT_EQ(3.05326876513317202466, (simit_float)c.get(p2));
}

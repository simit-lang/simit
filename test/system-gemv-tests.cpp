#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(System, gemv) {
  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Taint c
  c.set(p0, 42.0);
  c.set(p2, 42.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

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

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(p0));
  ASSERT_EQ(2.0, b.get(p1));
  ASSERT_EQ(3.0, b.get(p2));

  // Check that outputs are correct
  ASSERT_EQ(3.0, c.get(p0));
  ASSERT_EQ(13.0, c.get(p1));
  ASSERT_EQ(10.0, c.get(p2));
}

TEST(System, gemv_diagonal) {
  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

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
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(6.0, c.get(p1));
  ASSERT_EQ(6.0, c.get(p2));
}

TEST(System, gemv_nw) {
  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

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
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(4.0, c.get(p1));
  ASSERT_EQ(0.0, c.get(p2));
}

TEST(System, gemv_sw) {
  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

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
  ASSERT_EQ(0.0, c.get(p0));
  ASSERT_EQ(1.0, c.get(p1));
  ASSERT_EQ(4.0, c.get(p2));
}

TEST(System, gemv_assemble_from_points) {
  // Points
  Set<> points;
  FieldRef<double> a = points.addField<double>("a");
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  c.set(p0, 42.0);

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
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(4.0, c.get(p1));
  ASSERT_EQ(0.0, c.get(p2));
}

TEST(System, gemv_inplace) {
  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

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
  ASSERT_EQ(3.0, b.get(p0));
  ASSERT_EQ(13.0, b.get(p1));
  ASSERT_EQ(10.0, b.get(p2));
}

TEST(System, gemv_blocked) {
  // Points
  Set<> points;
  FieldRef<double,2> b = points.addField<double,2>("b");
  FieldRef<double,2> c = points.addField<double,2>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double,2,2> a = springs.addField<double,2,2>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  TensorRef<double,2> c0 = c.get(p0);
  ASSERT_EQ(16.0, c0(0));
  ASSERT_EQ(36.0, c0(1));

  TensorRef<double,2> c1 = c.get(p1);
  ASSERT_EQ(116.0, c1(0));
  ASSERT_EQ(172.0, c1(1));

  TensorRef<double,2> c2 = c.get(p2);
  ASSERT_EQ(100.0, c2(0));
  ASSERT_EQ(136.0, c2(1));
}

TEST(System, gemv_blocked_nw) {
  // Points
  Set<> points;
  FieldRef<double,2> b = points.addField<double,2>("b");
  FieldRef<double,2> c = points.addField<double,2>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double,2,2> a = springs.addField<double,2,2>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  f->runSafe();

  // Check that outputs are correct
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  TensorRef<double,2> c0 = c.get(p0);
  ASSERT_EQ(5.0, c0(0));
  ASSERT_EQ(11.0, c0(1));

  TensorRef<double,2> c1 = c.get(p1);
  ASSERT_EQ(39.0, c1(0));
  ASSERT_EQ(53.0, c1(1));

  TensorRef<double,2> c2 = c.get(p2);
  ASSERT_EQ(0.0, c2(0));
  ASSERT_EQ(0.0, c2(1));
}

TEST(System, gemv_blocked_computed) {
  // Points
  Set<> points;
  FieldRef<double,2> b = points.addField<double,2>("b");
  FieldRef<double,2> c = points.addField<double,2>("c");
  FieldRef<double,2> x = points.addField<double,2>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  x.set(p0, {1.0, 2.0});
  x.set(p1, {3.0, 4.0});
  x.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

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
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  TensorRef<double,2> c0 = c.get(p0);
  ASSERT_EQ(36.0, c0(0));
  ASSERT_EQ(72.0, c0(1));

  TensorRef<double,2> c1 = c.get(p1);
  ASSERT_EQ(336.0, c1(0));
  ASSERT_EQ(472.0, c1(1));

  TensorRef<double,2> c2 = c.get(p2);
  ASSERT_EQ(300.0, c2(0));
  ASSERT_EQ(400.0, c2(1));
}

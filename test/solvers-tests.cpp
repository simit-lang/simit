/// Built-in solver functions. Require that Simit is built with Eigen.
#ifdef EIGEN

#include "simit-test.h"

#include "tensor.h"
#include "ir.h"
#include "intrinsics.h"
#include "ir_printer.h"
#include "graph.h"
#include "program.h"
#include "error.h"
#include "types.h"

#include "runtime.h"

using namespace std;
using namespace simit;
using namespace simit::ir;

TEST(Solvers, solve) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 2.0);
  b.set(p1, 1.0);
  b.set(p2, 4.0);

  // Taint c
  c.set(p0, 42.0);
  c.set(p2, 42.0);

  // Springs
  Set springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 2.0);
  a.set(s1, 1.0);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check that inputs are preserved
  SIMIT_ASSERT_FLOAT_EQ(2.0, b.get(p0));
  SIMIT_ASSERT_FLOAT_EQ(1.0, b.get(p1));
  SIMIT_ASSERT_FLOAT_EQ(4.0, b.get(p2));

  // Check that outputs are correct
  ASSERT_NEAR(2.0, (double)c.get(p0), 0.00001);
  ASSERT_NEAR(1.0, (double)c.get(p1), 0.00001);
  ASSERT_NEAR(4.0, (double)c.get(p2), 0.00001);
}

TEST(Solvers, solve_blocked) {
  // Points
  Set points;
  FieldRef<simit_float,2> b = points.addField<simit_float,2>("b");
  FieldRef<simit_float,2> c = points.addField<simit_float,2>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, {1.0, 3.0});
  b.set(p1, {5.0, 7.0});
  b.set(p2, {2.0, 4.0});

  // Taint c
  c.set(p0, {1.0, 1.0});
  c.set(p2, {1.0, 1.0});

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
  // They're not going to be very close because the matrix is sucky for solves
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  TensorRef<simit_float,2> c0 = c.get(p0);
  ASSERT_NEAR(1.0, c0(0), 1.0);
  ASSERT_NEAR(3.0, c0(1), 1.0);

  TensorRef<simit_float,2> c1 = c.get(p1);
  ASSERT_NEAR(5.0, c1(0), 1.0);
  ASSERT_NEAR(7.0, c1(1), 1.0);

  TensorRef<simit_float,2> c2 = c.get(p2);
  ASSERT_NEAR(2.0, c2(0), 1.0);
  ASSERT_NEAR(4.0, c2(1), 1.0);
}

#endif

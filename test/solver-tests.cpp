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

TEST(solver, solve) {
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

TEST(solver, solve_blocked) {
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

TEST(solver, chol) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> x = points.addField<simit_float>("x");
  FieldRef<bool> fixed = points.addField<bool>("fixed");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b(p0) = 10.0;
  b(p1) = 20.0;
  b(p2) = 30.0;

  fixed(p0) = true;

  // Springs
  Set springs(points,points);

  springs.add(p0,p1);
  springs.add(p1,p2);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points",  &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check results
  SIMIT_ASSERT_FLOAT_EQ( 20.0, x(p0));
  SIMIT_ASSERT_FLOAT_EQ(-30.0, x(p1));
  SIMIT_ASSERT_FLOAT_EQ( 60.0, x(p2));
}

template<typename Float>
void getB(int Bn,  int Bm,  int** Browptr, int** Bcolidx,
          int Bnn, int Bmm, Float** Bvals) {
  Eigen::SparseMatrix<Float,Eigen::RowMajor> B(Bn, Bm);
  B.insert(0,0) = 10;
  B.insert(1,0) = 20;
  B.insert(2,0) = 30;
  B.insert(1,1) = 1;
  B.insert(0,2) = 1;
  B.insert(2,2) = 2;
  eigen2csr(B, Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals);
}
extern "C"
void sgetB(int Bn,  int Bm,  int** Browptr, int** Bcolidx,
           int Bnn, int Bmm, float** Bvals) {
  getB(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals);
}
extern "C"
void dgetB(int Bn,  int Bm,  int** Browptr, int** Bcolidx,
           int Bnn, int Bmm, double** Bvals) {
  getB(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals);
}

TEST(solver, cholmat) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> x = points.addField<simit_float>("x");
  FieldRef<bool> fixed = points.addField<bool>("fixed");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b(p0) = 1.0;
  b(p1) = 1.0;
  b(p2) = 1.0;

  fixed(p0) = true;

  // Springs
  Set springs(points,points);

  springs.add(p0,p1);
  springs.add(p1,p2);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points",  &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check results
  SIMIT_ASSERT_FLOAT_EQ( 22.0, x(p0));
  SIMIT_ASSERT_FLOAT_EQ(-33.0, x(p1));
  SIMIT_ASSERT_FLOAT_EQ( 65.0, x(p2));
}

#endif

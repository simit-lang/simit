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
  Set V;
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  FieldRef<simit_float> c = V.addField<simit_float>("c");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b.set(v0, 2.0);
  b.set(v1, 1.0);
  b.set(v2, 4.0);

  Set E(V,V);
  FieldRef<simit_float> a = E.addField<simit_float>("a");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  a.set(e0, 2.0);
  a.set(e1, 1.0);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_NEAR(2.0, (double)c.get(v0), 0.00001);
  ASSERT_NEAR(1.0, (double)c.get(v1), 0.00001);
  ASSERT_NEAR(4.0, (double)c.get(v2), 0.00001);
}

TEST(solver, solve_blocked) {
  Set V;
  FieldRef<simit_float,2> b = V.addField<simit_float,2>("b");
  FieldRef<simit_float,2> c = V.addField<simit_float,2>("c");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b.set(v0, {1.0, 3.0});
  b.set(v1, {5.0, 7.0});
  b.set(v2, {2.0, 4.0});

  Set E(V,V);
  FieldRef<simit_float,2,2> a = E.addField<simit_float,2,2>("a");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  a.set(e0, {1.0, 2.0, 3.0, 4.0});
  a.set(e1, {5.0, 6.0, 7.0, 8.0});

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  TensorRef<simit_float,2> c0 = c.get(v0);
  ASSERT_NEAR(1.0, c0(0), 1.0);
  ASSERT_NEAR(3.0, c0(1), 1.0);

  TensorRef<simit_float,2> c1 = c.get(v1);
  ASSERT_NEAR(5.0, c1(0), 1.0);
  ASSERT_NEAR(7.0, c1(1), 1.0);

  TensorRef<simit_float,2> c2 = c.get(v2);
  ASSERT_NEAR(2.0, c2(0), 1.0);
  ASSERT_NEAR(4.0, c2(1), 1.0);
}

TEST(solver, chol) {
  Set V;
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  FieldRef<bool> fixed = V.addField<bool>("fixed");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 10.0;
  b(v1) = 20.0;
  b(v2) = 30.0;
  fixed(v0) = true;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 20.0, x(v0));
  SIMIT_ASSERT_FLOAT_EQ(-30.0, x(v1));
  SIMIT_ASSERT_FLOAT_EQ( 60.0, x(v2));
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
  Set V;
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  FieldRef<bool> fixed = V.addField<bool>("fixed");
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 1.0;
  b(v2) = 1.0;
  fixed(v0) = true;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 22.0, x(v0));
  SIMIT_ASSERT_FLOAT_EQ(-33.0, x(v1));
  SIMIT_ASSERT_FLOAT_EQ( 65.0, x(v2));
}

TEST(DISABLED_solver, schur) {
  Set V;
  FieldRef<bool> fixed = V.addField<bool>("fixed");
  FieldRef<simit_float> f = V.addField<simit_float>("f");
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  f(v0) = 10.0;
  f(v1) = 20.0;
  f(v2) = 30.0;
  fixed(v0) = true;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Set D(V);
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  FieldRef<simit_float> l = V.addField<simit_float>("l");
  ElementRef d0 = D.add(v0);
  ElementRef d1 = D.add(v2);
  b(d0) =  100.0;
  b(d1) = -100.0;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("D", &D);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 100.0, x(v0));
  SIMIT_ASSERT_FLOAT_EQ(  10.0,   x(v1));
  SIMIT_ASSERT_FLOAT_EQ(-100.0, x(v2));
}

#endif

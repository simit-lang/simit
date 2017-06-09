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
  FieldRef<simit_float> x = V.addField<simit_float>("x");
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

  SIMIT_ASSERT_FLOAT_EQ( 0.202777777777778, (simit_float)x.get(v0));
  SIMIT_ASSERT_FLOAT_EQ(-0.013888888888889, (simit_float)x.get(v1));
  SIMIT_ASSERT_FLOAT_EQ( 0.802777777777778, (simit_float)x.get(v2));
}

TEST(solver, solve_blocked) {
  Set V;
  FieldRef<simit_float,2> b = V.addField<simit_float,2>("b");
  FieldRef<simit_float,2> x = V.addField<simit_float,2>("x");
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

  TensorRef<simit_float,2> c0 = x.get(v0);
  SIMIT_ASSERT_FLOAT_EQ( 0.069642857142857, (simit_float)c0(0));
  SIMIT_ASSERT_FLOAT_EQ(-0.004910714285714, (simit_float)c0(1));

  TensorRef<simit_float,2> c1 = x.get(v1);
  SIMIT_ASSERT_FLOAT_EQ(-0.044642857142857, (simit_float)c1(0));
  SIMIT_ASSERT_FLOAT_EQ( 0.073660714285714, (simit_float)c1(1));

  TensorRef<simit_float,2> c2 = x.get(v2);
  SIMIT_ASSERT_FLOAT_EQ( 0.269642857142857, (simit_float)c2(0));
  SIMIT_ASSERT_FLOAT_EQ(-0.204910714285714, (simit_float)c2(1));
}

TEST(solver, lu) {
  Set V;
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 10.0;
  b(v1) = 20.0;
  b(v2) = 30.0;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(12.50, x(v0));
  SIMIT_ASSERT_FLOAT_EQ(-7.50, x(v1));
  SIMIT_ASSERT_FLOAT_EQ(18.75, x(v2));
}

TEST(solver, triangular) {
  Set V;
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 10.0;
  b(v1) = 20.0;
  b(v2) = 30.0;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(5.0, x(v0));
  SIMIT_ASSERT_FLOAT_EQ(3.75, x(v1));
  SIMIT_ASSERT_FLOAT_EQ(13.125, x(v2));
}

TEST(DISABLED_solver, lu_blocked) {
  Set V;
  auto b = V.addField<simit_float,2>("b");
  auto x = V.addField<simit_float,2>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = {10.0, 20.0};
  b(v1) = {30.0, 40.0};
  b(v2) = {50.0, 60.0};

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 1.0000, x(v0)(0));
  SIMIT_ASSERT_FLOAT_EQ( 8.5000, x(v0)(1));
  SIMIT_ASSERT_FLOAT_EQ(-1.0000, x(v1)(0));
  SIMIT_ASSERT_FLOAT_EQ(-3.5000, x(v1)(1));
  SIMIT_ASSERT_FLOAT_EQ( 6.4167, x(v2)(0));
  SIMIT_ASSERT_FLOAT_EQ(12.6667, x(v2)(1));
}

TEST(solver, lumat) {
  Set V;
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Set T(V);
  FieldRef<simit_float> b = T.addField<simit_float>("b");
  ElementRef d0 = T.add(v0);
  ElementRef d1 = T.add(v2);
  b(d0) = 1.0;
  b(d1) = 2.0;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("T", &T);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 1.750, (simit_float)x(v0));
  SIMIT_ASSERT_FLOAT_EQ(-1.250, (simit_float)x(v1));
  SIMIT_ASSERT_FLOAT_EQ( 1.625, (simit_float)x(v2));
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

TEST(solver, cholmat) {
  Set V;
  FieldRef<simit_float> x = V.addField<simit_float>("x");
  FieldRef<bool> fixed = V.addField<bool>("fixed");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  fixed(v0) = true;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Set T(V);
  FieldRef<simit_float> b = T.addField<simit_float>("b");
  ElementRef d0 = T.add(v0);
  ElementRef d1 = T.add(v2);
  b(d0) = 1.0;
  b(d1) = 2.0;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("T", &T);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 3.0, (simit_float)x(v0));
  SIMIT_ASSERT_FLOAT_EQ(-5.0, (simit_float)x(v1));
  SIMIT_ASSERT_FLOAT_EQ( 7.0, (simit_float)x(v2));
}

TEST(solver, schur) {
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
  FieldRef<simit_float> b = D.addField<simit_float>("b");
  FieldRef<simit_float> l = D.addField<simit_float>("l");
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
  SIMIT_ASSERT_FLOAT_EQ(  10.0, x(v1));
  SIMIT_ASSERT_FLOAT_EQ(-100.0, x(v2));
}

TEST(DISABLED_solver, schur_blocked) {
  Set V;
  FieldRef<bool> fixed = V.addField<bool>("fixed");
  FieldRef<simit_float,2> f = V.addField<simit_float,2>("f");
  FieldRef<simit_float,2> x = V.addField<simit_float,2>("x");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  f(v0) = {10.0, 10.0};
  f(v1) = {20.0, 20.0};
  f(v2) = {30.0, 30.0};
  fixed(v0) = true;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Set D(V);
  FieldRef<simit_float,2> b = D.addField<simit_float,2>("b");
  FieldRef<simit_float,2> l = D.addField<simit_float,2>("l");
  ElementRef d0 = D.add(v0);
  ElementRef d1 = D.add(v2);
  b(d0) = { 100.0,  100};
  b(d1) = {-100.0, -100};

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.bind("D", &D);
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ( 100.0,       x(v0)(0));
  SIMIT_ASSERT_FLOAT_EQ( 100.0,       x(v0)(1));
  SIMIT_ASSERT_FLOAT_EQ(  3.33333333, x(v1)(0));
  SIMIT_ASSERT_FLOAT_EQ(  3.33333333, x(v1)(1));
  SIMIT_ASSERT_FLOAT_EQ(-100.0,       x(v2)(0));
  SIMIT_ASSERT_FLOAT_EQ(-100.0,       x(v2)(1));
}

#endif

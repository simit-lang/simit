#include "simit-test.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, isprings_simple) {
  // Points
  Set points;
  simit::FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  simit::FieldRef<simit_float,3> v = points.addField<simit_float,3>("v");
  simit::FieldRef<simit_float,3> x2 = points.addField<simit_float,3>("x2");
  simit::FieldRef<simit_float,3> v2 = points.addField<simit_float,3>("v2");

  simit::FieldRef<simit_float,3> ones = points.addField<simit_float,3>("ones");
  simit::FieldRef<simit_float,3> zeros = points.addField<simit_float,3>("zeros");

  // Springs
  Set springs(points,points);
  simit::FieldRef<simit_float> m = springs.addField<simit_float>("m");
  simit::FieldRef<simit_float> l0 = springs.addField<simit_float>("l0");
  simit::FieldRef<simit_float> k = springs.addField<simit_float>("k");

  // Build a 3-chain
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x(p0) = {0.0, 0.0, 0.0};
  x(p1) = {1.0, 0.0, 0.0};
  x(p2) = {2.0, 0.0, 0.0};

  v(p0) = {0.1, 0.0, 0.0};
  v(p1) = {0.1, 0.0, 0.0};
  v(p2) = {0.1, 0.0, 0.0};

  zeros(p0) = {0.0, 0.0, 0.0};
  zeros(p1) = {0.0, 0.0, 0.0};
  zeros(p2) = {0.0, 0.0, 0.0};

  ones(p0) = {1.0, 1.0, 1.0};
  ones(p1) = {1.0, 1.0, 1.0};
  ones(p2) = {1.0, 1.0, 1.0};

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  // Initialize springs
  simit_float rho = 1.0;
  simit_float _l0 = 0.9;
  simit_float stiffness = 1e1;
  simit_float radius = 0.1;
  simit_float volume = 3.1415*radius*radius*_l0;
  simit_float springMass = rho*volume;

  m.set(s0, springMass);
  m.set(s1, springMass);

  l0.set(s0, _l0);
  l0.set(s1, _l0);

  k.set(s0, stiffness);
  k.set(s1, stiffness);

  // Compile program and bind arguments
  Function func = getFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);
  func.bind("springs", &springs);
  for (size_t i=0; i < 10; ++i) {
    func.runSafe();
  }

  // Check outputs
  ASSERT_SIMIT_FLOAT_EQ(0.10241860338789253, x2(p0)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                 x2(p0)(1));
  ASSERT_SIMIT_FLOAT_EQ(-0.0103815692520815, x2(p0)(2));
  ASSERT_SIMIT_FLOAT_EQ(1.01,                x2(p1)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                 x2(p1)(1));
  ASSERT_SIMIT_FLOAT_EQ(-0.020763138504163,  x2(p1)(2));
  ASSERT_SIMIT_FLOAT_EQ(1.9175813966121074,  x2(p2)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                 x2(p2)(1));
  ASSERT_SIMIT_FLOAT_EQ(-0.0103815692520815, x2(p2)(2));
}

TEST(Program, isprings) {
  double l0_       = 1.0;
  double stiffness = 100.00;
  double density   = 1000.0;
  double radius = 0.01;
  double PI_ = 3.14159265358979;
  double zfloor = 0.4;

  // Points
  Set points;
  simit::FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  simit::FieldRef<simit_float,3> v = points.addField<simit_float,3>("v");
  simit::FieldRef<simit_float> m = points.addField<simit_float>("m");
  simit::FieldRef<bool> fixed = points.addField<bool>("fixed");

  // Springs
  Set springs(points,points);
  simit::FieldRef<simit_float> k = springs.addField<simit_float>("k");
  simit::FieldRef<simit_float> l0 = springs.addField<simit_float>("l0");

  // Build a 3-chain
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x(p0) = {0.0, 0.0, 0.0};
  x(p1) = {0.0, 0.0, 1.0};
  x(p2) = {0.0, 0.0, 2.0};

  v(p0) = {0.0, 0.0, 0.0};
  v(p1) = {0.0, 0.0, 0.0};
  v(p2) = {0.0, 0.0, 0.0};

  m(p0) = PI_*radius*radius*l0_*density;
  m(p1) = PI_*radius*radius*l0_*density;
  m(p2) = PI_*radius*radius*l0_*density;

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  l0(s0) = l0_;
  l0(s1) = l0_;

  k(s0) = stiffness;
  k(s1) = stiffness;


  for (auto &p : points) {
    if (x(p)(2) < zfloor) {
      fixed(p) = true;
    }
    else {
      fixed(p) = false;
    }
  }

  // Compile program and bind arguments
  Function func = getFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);
  func.bind("springs", &springs);
  for (size_t i=0; i < 10; ++i) {
    func.runSafe();
  }

  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p0)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p0)(1));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p0)(2));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p1)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p1)(1));
  ASSERT_SIMIT_FLOAT_EQ(-0.54482994491692449, v(p1)(2));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p2)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  v(p2)(1));
  ASSERT_SIMIT_FLOAT_EQ(-0.8480293605755741,  v(p2)(2));

  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p0)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p0)(1));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p0)(2));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p1)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p1)(1));
  ASSERT_SIMIT_FLOAT_EQ(0.96180188527075183,  x(p1)(2));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p2)(0));
  ASSERT_SIMIT_FLOAT_EQ(0.0,                  x(p2)(1));
  ASSERT_SIMIT_FLOAT_EQ(1.9500008677096612,   x(p2)(2));
}

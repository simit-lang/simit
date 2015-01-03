#include "simit-test.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, DISABLED_isprings) {
  // Points
  Set<> points;
  simit::FieldRef<double,3> x = points.addField<double,3>("x");
  simit::FieldRef<double,3> v = points.addField<double,3>("v");
  simit::FieldRef<double,3> x2 = points.addField<double,3>("x2");
  simit::FieldRef<double,3> v2 = points.addField<double,3>("v2");

  simit::FieldRef<double,3> ones = points.addField<double,3>("ones");
  simit::FieldRef<double,3> zeros = points.addField<double,3>("zeros");
  simit::FieldRef<double,3> print = points.addField<double,3>("print");

  // Springs
  Set<2> springs(points,points);
  simit::FieldRef<double> m = springs.addField<double>("m");
  simit::FieldRef<double> l0 = springs.addField<double>("l0");
  simit::FieldRef<double> k = springs.addField<double>("k");

  // Build a 3-chain
  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  x.set(p0, {0.0, 0.0, 0.0});
  x.set(p1, {1.0, 0.0, 0.0});
  x.set(p2, {2.0, 0.0, 0.0});

  v.set(p0, {0.1, 0.0, 0.0});
  v.set(p1, {0.1, 0.0, 0.0});
  v.set(p2, {0.1, 0.0, 0.0});

  print.set(p0, {0.0, 0.0, 0.0});
  print.set(p1, {0.0, 0.0, 0.0});
  print.set(p2, {0.0, 0.0, 0.0});

  zeros.set(p0, {0.0, 0.0, 0.0});
  zeros.set(p1, {0.0, 0.0, 0.0});
  zeros.set(p2, {0.0, 0.0, 0.0});

  ones.set(p0, {1.0, 1.0, 1.0});
  ones.set(p1, {1.0, 1.0, 1.0});
  ones.set(p2, {1.0, 1.0, 1.0});

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  // Initialize springs
  double rho = 1.0;
  double _l0 = 0.9;
  double stiffness = 1e1;
  double radius = 0.1;
  double volume = 3.1415*radius*radius*_l0;
  double springMass = rho*volume;

  m.set(s0, springMass);
  m.set(s1, springMass);

  l0.set(s0, _l0);
  l0.set(s1, _l0);

  k.set(s0, stiffness);
  k.set(s1, stiffness);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->bind("springs", &springs);

  for (size_t i=0; i < 10; ++i) {
    f->runSafe();
  }

  // Check outputs
  TensorRef<double,3> x21 = x2.get(p0);

  ASSERT_DOUBLE_EQ(0.10241860338789253, x21(0));
  ASSERT_DOUBLE_EQ(0.0, x21(1));
  ASSERT_DOUBLE_EQ(-0.0103815692520815, x21(2));

  TensorRef<double,3> x22 = x2.get(p1);
  ASSERT_DOUBLE_EQ(1.01, x22(0));
  ASSERT_DOUBLE_EQ(0.0, x22(1));
  ASSERT_DOUBLE_EQ(-0.020763138504163, x22(2));

  TensorRef<double,3> x23 = x2.get(p2);
  ASSERT_DOUBLE_EQ(1.9175813966121074, x23(0));
  ASSERT_DOUBLE_EQ(0.0, x23(1));
  ASSERT_DOUBLE_EQ(-0.0103815692520815, x23(2));
}

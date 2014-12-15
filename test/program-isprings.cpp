#include "gtest/gtest.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(isprings, DISABLED_isprings) {
  Program program;
  std::string programText = R"(
element Point
  x : tensor[3](float);
  v : tensor[3](float);

  x2 : tensor[3](float);
  v2 : tensor[3](float);

  zeros : tensor[3](float);
  ones : tensor[3](float);
  print : tensor[3](float);
end

element Spring
  k  : float;
  l0 : float;
  m  : float;
end

extern points  : set{Point};
extern springs : set{Spring}(points,points);

func compute_MK(s: Spring, p : (Point*2)) ->
    (MK : tensor[points,points](tensor[3,3](float)))
  I = [1.0, 0.0, 0.0; 0.0, 1.0, 0.0; 0.0, 0.0, 1.0];
  grav = [0.0, 0.0, -9.81];
  h = 1e-1;
  k = s.k;
  l0 = s.l0;

  % Mass matrix
  halfm = 0.5*s.m;
  m = halfm*I;
  M00 = m;
  M11 = m;

  % Spring force
  dx = p(1).x - p(0).x;
  l = norm(dx);

  % Stiffness matrix
  dxtdx = dx*dx';
  dxdxt = dx'*dx;
  K = h*h*k/(l0*l0*l*l)*(dxdxt + (l-l0)/l*(dxtdx*I - dxdxt));

  MK(p(0),p(0)) = M00 + K;
  MK(p(0),p(1)) =     - K;
  MK(p(1),p(0)) =     - K;
  MK(p(1),p(1)) = M11 + K;
end

func compute_f(s: Spring, p : (Point*2)) ->
    (f : tensor[points](tensor[3](float)))
  I = [1.0, 0.0, 0.0; 0.0, 1.0, 0.0; 0.0, 0.0, 1.0];
  grav = [0.0, 0.0, -9.81];
  h = 1e-1;
  k = s.k;
  l0 = s.l0;

  % Mass matrix
  halfm = 0.5*s.m;
  m = halfm*I;
  M00 = m;
  M11 = m;

  % Momentum
  fm0 = halfm*p(0).v;
  fm1 = halfm*p(1).v;

  % Gravity
  fg = h*halfm*grav;

  % Spring force
  dx = p(1).x - p(0).x;
  l = norm(dx);
  fs = h*k/(l0*l0)*(l-l0)*dx/l;

  % Insert into globals
  f(p(0)) = fm0 + fg + fs;
  f(p(1)) = fm1 + fg - fs;
end

proc main
  h = 1e-1;

  fm = map compute_f to springs with points reduce +;
  f = fm;

  MK = map compute_MK to springs with points reduce +;

  %r = f - MK*v;
  MKv = MK * points.v;
  r = f - MKv;

  r2old = r*r';

  Ap = MK*r;
  rap = r*Ap;
  a = r2old/rap;
  ap = a*r;

  v = points.v + ap;
  points.v2 = v;

  hvap = h * v;
  points.x2  = points.x + hvap;
end
  )";

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
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> func = program.compile("main");
  if (!func) FAIL() << program.getDiagnostics().getMessage();

  func->bind("points", &points);
  func->bind("springs", &springs);

  for (size_t i=0; i < 10; ++i) {
    func->runSafe();
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

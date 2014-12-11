#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, esprings) {
  Program program;
  std::string programText = R"(
element Point
  x : tensor[3](float);
  v : tensor[3](float);

  fs : tensor[3](float);
  fg : tensor[3](float);
  M : tensor[3](float);
  p : tensor[3](float);
end

element Spring
  m  : float;
  l0 : float;
end

extern points  : set{Point};
extern springs : set{Spring}(points,points);

func distribute_masses(s : Spring, p : (Point*2)) ->
    (M : tensor[points](tensor[3](float)))
  eye = [1.0, 1.0, 1.0];
  M(p(0)) = 0.5*s.m*eye;
  M(p(1)) = 0.5*s.m*eye;
end

func distribute_gravity(s : Spring, p : (Point*2)) ->
    (f : tensor[points](tensor[3](float)))
  grav = [0.0, 0.0, -9.81];
  halfm = 0.5*s.m*grav;
  f(p(0)) = halfm;
  f(p(1)) = halfm;
end

func compute_stiffness(s : Spring, p : (Point*2)) ->
    (f : tensor[points](tensor[3](float)))
  stiffness = 3.0;
  dx = p(1).x - p(0).x;
  l = norm(dx);
  f0 = stiffness/(s.l0*s.l0)*(l-s.l0)*dx/l;
  f(p(0)) = f0;
  f(p(1)) = -f0;
end

proc main
  h = 0.01;

  fg = map distribute_gravity to springs with points reduce +;
  M = map distribute_masses to springs with points reduce +;
  fs = map compute_stiffness to springs with points reduce +;

  points.fs = --fs;
  points.fg = --fg;
  points.M = --M;

  % p = M*v + h*(fs + fg);
  points.p = (points.M .* points.v) + (h * (points.fs + points.fg));

  % v = p / diag(M)
  points.v = points.p ./ points.M;

  % x = x + hv
  points.x  = points.x + (h * points.v);
end
  )";

  // Points
  Set<> points;
  FieldRef<double,3> x = points.addField<double,3>("x");
  FieldRef<double,3> v = points.addField<double,3>("v");

  FieldRef<double,3> fs = points.addField<double,3>("fs");
  FieldRef<double,3> fg = points.addField<double,3>("fg");
  FieldRef<double,3> M = points.addField<double,3>("M");
  FieldRef<double,3> p = points.addField<double,3>("p");

  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  ElementRef p3 = points.add();
  ElementRef p4 = points.add();
  ElementRef p5 = points.add();
  ElementRef p6 = points.add();
  ElementRef p7 = points.add();
  ElementRef p8 = points.add();

  x.set(p1, {0.0, 0.0, 0.0});
  x.set(p2, {1.0, 0.0, 0.0});
  x.set(p3, {0.0, 1.0, 0.0});
  x.set(p4, {1.0, 1.0, 0.0});
  x.set(p5, {0.0, 0.0, 1.0});
  x.set(p6, {1.0, 0.0, 1.0});
  x.set(p7, {0.0, 1.0, 1.0});
  x.set(p8, {1.0, 1.0, 1.0});

  v.set(p1, {0.0, 0.0, 0.0});
  v.set(p2, {0.0, 0.0, 0.0});
  v.set(p3, {0.0, 0.0, 0.0});
  v.set(p4, {0.0, 0.0, 0.0});
  v.set(p5, {0.0, 0.0, 0.0});
  v.set(p6, {0.0, 0.0, 0.0});
  v.set(p7, {0.0, 0.0, 0.0});
  v.set(p8, {0.0, 0.0, 0.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> l0 = springs.addField<double>("l0");
  FieldRef<double> m = springs.addField<double>("m");

  // x springs
  ElementRef s1 = springs.add(p1,p2);
  ElementRef s2 = springs.add(p3,p4);
  ElementRef s3 = springs.add(p5,p6);
  ElementRef s4 = springs.add(p7,p8);

  ElementRef s5 = springs.add(p1,p3);
  ElementRef s6 = springs.add(p2,p4);
  ElementRef s7 = springs.add(p5,p7);
  ElementRef s8 = springs.add(p6,p8);

  ElementRef s9 = springs.add(p1,p5);
  ElementRef s10 = springs.add(p2,p6);
  ElementRef s11 = springs.add(p3,p7);
  ElementRef s12 = springs.add(p4,p8);

  m.set(s1, 0.0282735);
  m.set(s2, 0.0282735);
  m.set(s3, 0.0282735);
  m.set(s4, 0.0282735);
  m.set(s5, 0.0282735);
  m.set(s6, 0.0282735);
  m.set(s7, 0.0282735);
  m.set(s8, 0.0282735);
  m.set(s9, 0.0282735);
  m.set(s10, 0.0282735);
  m.set(s11, 0.0282735);
  m.set(s12, 0.0282735);

  l0.set(s1, 0.9);
  l0.set(s2, 0.9);
  l0.set(s3, 0.9);
  l0.set(s4, 0.9);
  l0.set(s5, 0.9);
  l0.set(s6, 0.9);
  l0.set(s7, 0.9);
  l0.set(s8, 0.9);
  l0.set(s9, 0.9);
  l0.set(s10, 0.9);
  l0.set(s11, 0.9);
  l0.set(s12, 0.9);

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
  TensorRef<double,3> x1 = x.get(p1);
  ASSERT_DOUBLE_EQ(0.0409248172084922, x1(0));
  ASSERT_DOUBLE_EQ(0.0409248172084922, x1(1));
  ASSERT_DOUBLE_EQ(-0.0130301827915078, x1(2));

  TensorRef<double,3> x2 = x.get(p2);
  ASSERT_DOUBLE_EQ(0.959075182791508, x2(0));
  ASSERT_DOUBLE_EQ(0.0409248172084922, x2(1));
  ASSERT_DOUBLE_EQ(-0.0130301827915078, x2(2));

  TensorRef<double,3> x3 = x.get(p3);
  ASSERT_DOUBLE_EQ(0.0409248172084922, x3(0));
  ASSERT_DOUBLE_EQ(0.959075182791508, x3(1));
  ASSERT_DOUBLE_EQ(-0.0130301827915078, x3(2));

  TensorRef<double,3> x4 = x.get(p4);
  ASSERT_DOUBLE_EQ(0.959075182791508, x4(0));
  ASSERT_DOUBLE_EQ(0.959075182791508, x4(1));
  ASSERT_DOUBLE_EQ(-0.0130301827915078, x4(2));

  TensorRef<double,3> x5 = x.get(p5);
  ASSERT_DOUBLE_EQ(0.0409248172084922, x5(0));
  ASSERT_DOUBLE_EQ(0.0409248172084922, x5(1));
  ASSERT_DOUBLE_EQ(0.905120182791508, x5(2));

  TensorRef<double,3> x6 = x.get(p6);
  ASSERT_DOUBLE_EQ(0.959075182791508, x6(0));
  ASSERT_DOUBLE_EQ(0.0409248172084922, x6(1));
  ASSERT_DOUBLE_EQ(0.905120182791508, x6(2));

  TensorRef<double,3> x7 = x.get(p7);
  ASSERT_DOUBLE_EQ(0.0409248172084922, x7(0));
  ASSERT_DOUBLE_EQ(0.959075182791508, x7(1));
  ASSERT_DOUBLE_EQ(0.905120182791508, x7(2));

  TensorRef<double,3> x8 = x.get(p8);
  ASSERT_DOUBLE_EQ(0.959075182791508, x8(0));
  ASSERT_DOUBLE_EQ(0.959075182791508, x8(1));
  ASSERT_DOUBLE_EQ(0.905120182791508, x8(2));
}
#include "gtest/gtest.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "errors.h"

using namespace std;
using namespace simit;

// TODO: Create a more effective test suite the same way as how dense LA is
//       tested, using parameterized tests, sparse literals and code assertions
//       This test suite should just test Program class performance, and not
//       different code generation kernels.

TEST(Program, add) {
  std::string programText = R"(
    element Point
      x : float;
    end

    extern points : set{Point};

    proc addSets
      points.x = points.x + points.x;
    end
  )";

  Program program;
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("addSets");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  Set<> points;
  FieldRef<double> x = points.addField<double>("x");
  f->bind("points", &points);

  ElementRef p0 = points.addElement();
  x.set(p0, 42.0);

  ASSERT_EQ(42.0, x.get(p0));
  f->runSafe();
  ASSERT_EQ(84.0, x.get(p0));
}

TEST(Program, add_blocked) {
  std::string programText = R"(
    element Point
      x : tensor[3](float);
    end

    extern points : set{Point};

    func addSets(points : set{Point}) -> (points : set{Point})
      points.x = points.x + points.x;
    end
  )";

  Program program;
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  return;  // TODO newir: Remove

  std::unique_ptr<Function> f = program.compile("addSets");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  Set<> points;
  FieldRef<double,3> x = points.addField<double,3>("x");

  ElementRef p0 = points.addElement();
  x.set(p0, {1.0, 2.0, 3.0});

  TensorRef<double,3> vec1 = x.get(p0);
  ASSERT_EQ(1.0, vec1(0));
  ASSERT_EQ(2.0, vec1(1));
  ASSERT_EQ(3.0, vec1(2));

  f->bind("points", &points);
  f->runSafe();

  TensorRef<double,3> vec2 = x.get(p0);
  ASSERT_EQ(2.0, vec2(0));
  ASSERT_EQ(4.0, vec2(1));
  ASSERT_EQ(6.0, vec2(2));
}

TEST(Program, dot) {
  std::string programText = R"(
    element Point
      x : float;
      z : float;
    end

    extern points : set{Point};

    proc dot
      s = points.x * points.x';
      points.z = s + points.z;
    end
  )";

  Program program;
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("dot");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  Set<> points;
  FieldRef<double> x = points.addField<double>("x");
  FieldRef<double> z = points.addField<double>("z");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();
  x.set(p0, 1.0);
  x.set(p1, 2.0);
  x.set(p2, 3.0);

  f->bind("points", &points);

  f->runSafe();
  ASSERT_EQ(14.0, z.get(p0));
}

TEST(Program, dot_blocked) {
  std::string programText = R"(
    element Point
      x : tensor[3](float);
      z : float;
    end

    extern points : set{Point};

    proc dot
      s = points.x * points.x';
      points.z = s + points.z;
    end
  )";

  Program program;
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("dot");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  Set<> points;
  FieldRef<double,3> x = points.addField<double,3>("x");
  FieldRef<double> z = points.addField<double>("z");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();
  x.set(p0, {1.0,2.0,3.0});
  x.set(p1, {4.0,5.0,6.0});
  x.set(p2, {7.0,8.0,9.0});

  f->bind("points", &points);

  f->runSafe();
  ASSERT_EQ(285.0, (double)z.get(p0));
}


TEST(Program, gemv) {
  Program program;
  std::string programText = R"(
    element Point
      b : float;
      c : float;
    end

    element Spring
      a : float;
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) ->
        (A : tensor[points,points](float))
      A(p(0),p(0)) = s.a;
      A(p(0),p(1)) = s.a;
      A(p(1),p(0)) = s.a;
      A(p(1),p(1)) = s.a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end;
  )";

  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Taint c
  c.set(p0, 42.0);
  c.set(p2, 42.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

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

TEST(Program, gemv_diagonal) {
  Program program;
  std::string programText = R"(
    element Point
      b : float;
      c : float;
    end

    element Spring
      a : float;
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) ->
        (A : tensor[points,points](float))
      A(p(0),p(0)) = s.a;
      A(p(1),p(1)) = s.a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end;
  )";

  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(6.0, c.get(p1));
  ASSERT_EQ(6.0, c.get(p2));
}

TEST(Program, gemv_nw) {
  Program program;
  std::string programText = R"(
    element Point
      b : float;
      c : float;
    end

    element Spring
      a : float;
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) -> (A : tensor[points,points](float))
      A(p(0),p(0)) = s.a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end;
  )";

  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(4.0, c.get(p1));
  ASSERT_EQ(0.0, c.get(p2));
}

TEST(Program, gemv_assemble_from_points) {
  Program program;
  std::string programText = R"(
    element Point
      b : float;
      c : float;
      a : float;
    end

    element Spring
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) -> (A : tensor[points,points](float))
      A(p(0),p(0)) = p(0).a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end
  )";

  // Points
  Set<> points;
  FieldRef<double> a = points.addField<double>("a");
  FieldRef<double> b = points.addField<double>("b");
  FieldRef<double> c = points.addField<double>("c");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  c.set(p0, 42.0);

  // Springs
  Set<2> springs(points,points);

  springs.addElement(p0,p1);
  springs.addElement(p1,p2);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(1.0, c.get(p0));
  ASSERT_EQ(4.0, c.get(p1));
  ASSERT_EQ(0.0, c.get(p2));
}

TEST(Program, gemv_blocked) {
  Program program;
  std::string programText = R"(
    element Point
      b : tensor[2](float);
      c : tensor[2](float);
    end

    element Spring
      a : tensor[2,2](float);
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) ->
        (M : tensor[points,points](tensor[2,2](float)))
      M(p(0),p(0)) = s.a;
      M(p(0),p(1)) = s.a;
      M(p(1),p(0)) = s.a;
      M(p(1),p(1)) = s.a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end
  )";

  // Points
  Set<> points;
  FieldRef<double,2> b = points.addField<double,2>("b");
  FieldRef<double,2> c = points.addField<double,2>("c");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double,2,2> a = springs.addField<double,2,2>("a");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

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

TEST(Program, gemv_blocked_nw) {
  Program program;
  std::string programText = R"(
    element Point
      b : tensor[2](float);
      c : tensor[2](float);
    end

    element Spring
      a : tensor[2,2](float);
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) ->
        (M : tensor[points,points](tensor[2,2](float)))
      M(p(0),p(0)) = s.a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end
  )";

  // Points
  Set<> points;
  FieldRef<double,2> b = points.addField<double,2>("b");
  FieldRef<double,2> c = points.addField<double,2>("c");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double,2,2> a = springs.addField<double,2,2>("a");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

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

TEST(Program, gemv_blocked_computed) {
  Program program;
  std::string programText = R"(
    element Point
      b : tensor[2](float);
      c : tensor[2](float);
      x : tensor[2](float);
    end

    element Spring
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) ->
        (M : tensor[points,points](tensor[2,2](float)))
      M(p(0),p(0)) = p(0).x' * p(1).x;
      M(p(0),p(1)) = p(0).x' * p(1).x;
      M(p(1),p(0)) = p(0).x' * p(1).x;
      M(p(1),p(1)) = p(0).x' * p(1).x;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.c = A * points.b;
    end
  )";

  // Points
  Set<> points;
  FieldRef<double,2> b = points.addField<double,2>("b");
  FieldRef<double,2> c = points.addField<double,2>("c");
  FieldRef<double,2> x = points.addField<double,2>("x");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

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
  springs.addElement(p0,p1);
  springs.addElement(p1,p2);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

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

TEST(Program, gemv_inplace) {
  Program program;
  std::string programText = R"(
    element Point
      b : float;
    end

    element Spring
      a : float;
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func dist_a(s : Spring, p : (Point*2)) -> (A : tensor[points,points](float))
      A(p(0),p(0)) = s.a;
      A(p(0),p(1)) = s.a;
      A(p(1),p(0)) = s.a;
      A(p(1),p(1)) = s.a;
    end

    proc gemv
      A = map dist_a to springs with points reduce +;
      points.b = A * points.b;
    end;
  )";

  // Points
  Set<> points;
  FieldRef<double> b = points.addField<double>("b");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> a = springs.addField<double>("a");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("gemv");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->runSafe();

  // Check that outputs are correct
  ASSERT_EQ(3.0, b.get(p0));
  ASSERT_EQ(13.0, b.get(p1));
  ASSERT_EQ(10.0, b.get(p2));
}

// TODO: fs inplace on x
TEST(Program, fs) {
  Program program;
  std::string programText = R"(
    element Point
      a : tensor[3](float);
      b : tensor[3](float);
      x : tensor[3](float);
    end

    element Spring
      l0 : float;
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func compute_fs(s : Spring, p : (Point*2)) ->
        (f : tensor[points](tensor[3](float)))
      stiffness = 1.0e3;
      dx = p(1).x - p(0).x;
      l = norm(dx);
      f(p(0)) = stiffness/(s.l0*s.l0)*(l-s.l0)*dx/l;
      f(p(1)) = -(stiffness/(s.l0*s.l0)*(l-s.l0)*dx/l);
    end

    proc fs
      f = map compute_fs to springs with points reduce +;
      points.b = f + points.a;
    end
  )";

  // Points
  Set<> points;
  FieldRef<double,3> a = points.addField<double,3>("a");
  FieldRef<double,3> b = points.addField<double,3>("b");
  FieldRef<double,3> x = points.addField<double,3>("x");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  a.set(p0, {0, 0, 0});
  a.set(p1, {0, 0, 0});
  a.set(p2, {0, 0, 0});

  x.set(p0, {5.0, 2.0, 6.0});
  x.set(p1, {6.0, 5.0, 2.0});
  x.set(p2, {7.0, 8.0, 4.0});

  // Taint c
  b.set(p0, {42.0, 42.0, 42.0});
  b.set(p2, {42.0, 42.0, 42.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> l0 = springs.addField<double>("l0");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  l0.set(s0, 1.0);
  l0.set(s1, 2.0);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("fs");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->runSafe();

  // Check outputs
  TensorRef<double,3> c0 = b.get(p0);
  ASSERT_DOUBLE_EQ(803.883864861816,  c0(0));
  ASSERT_DOUBLE_EQ(2411.6515945854476,  c0(1));
  ASSERT_DOUBLE_EQ(-3215.5354594472637, c0(2));

  TensorRef<double,3> c1 = b.get(p1);
  ASSERT_DOUBLE_EQ(-687.514485818028, c1(0));
  ASSERT_DOUBLE_EQ(-2062.5434574540841, c1(1));
  ASSERT_DOUBLE_EQ(3448.27421753484,  c1(2));

  TensorRef<double,3> c2 = b.get(p2);
  ASSERT_DOUBLE_EQ(-116.36937904378782, c2(0));
  ASSERT_DOUBLE_EQ(-349.10813713136343, c2(1));
  ASSERT_DOUBLE_EQ(-232.73875808757563, c2(2));
}


TEST(Program, compute_spring_force) {
  Program program;
  std::string programText = R"(
    element Point
      x : tensor[3](float);
      v : tensor[3](float);
      f : tensor[3](float);

      tmp0 : tensor[3](float);
    end

    element Spring
      m  : float;
      l0 : float;
    end

    extern points  : set{Point};
    extern springs : set{Spring}(points,points);

    func distribute_masses(s : Spring, p : (Point*2)) ->
        (f : tensor[points](tensor[3](float)))
      grav = [0.0, 0.0, -9.81];
      f(p(0)) = 0.5*s.m*grav;
      f(p(1)) = 0.5*s.m*grav;
    end

    func compute_stiffness(s : Spring, p : (Point*2)) ->
        (f : tensor[points](tensor[3](float)))
      stiffness = 1.0e3;
      dx = p(1).x - p(0).x;
      l = norm(dx);
      f(p(0)) = stiffness/(s.l0*s.l0)*(l-s.l0)*dx/l;
      f(p(1)) = -(stiffness/(s.l0*s.l0)*(l-s.l0)*dx/l);
    end

    proc main
      fg = map distribute_masses to springs with points reduce +;
      fs = map compute_stiffness to springs with points reduce +;
      points.tmp0 = -(-fg);
      points.f = --fs + points.tmp0;
    end
  )";

  // Points
  Set<> points;
  FieldRef<double,3> x = points.addField<double,3>("x");
  FieldRef<double,3> v = points.addField<double,3>("v");
  FieldRef<double,3> f = points.addField<double,3>("f");
  FieldRef<double,3> tmp0 = points.addField<double,3>("tmp0");

  ElementRef p0 = points.addElement();
  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();

  x.set(p0, {1.0, 2.0, 3.0});
  x.set(p1, {4.0, 5.0, 6.0});
  x.set(p2, {7.0, 8.0, 9.0});

  // Springs
  Set<2> springs(points,points);
  FieldRef<double> l0 = springs.addField<double>("l0");
  FieldRef<double> m = springs.addField<double>("m");

  ElementRef s0 = springs.addElement(p0,p1);
  ElementRef s1 = springs.addElement(p1,p2);

  l0.set(s0, 1.0);
  l0.set(s1, 2.0);

  m.set(s0, 3.0);
  m.set(s1, 4.0);

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> func = program.compile("main");
  if (!func) FAIL() << program.getDiagnostics().getMessage();

  func->bind("points", &points);
  func->bind("springs", &springs);
  func->runSafe();

  // Check outputs
  TensorRef<double,3> f0 = f.get(p0);
  ASSERT_DOUBLE_EQ(2422.649730810374,  f0(0));
  ASSERT_DOUBLE_EQ(2422.649730810374,  f0(1));
  ASSERT_DOUBLE_EQ(2407.9347308103738, f0(2));

  TensorRef<double,3> f1 = f.get(p1);
  ASSERT_DOUBLE_EQ(-1961.3248654051868, f1(0));
  ASSERT_DOUBLE_EQ(-1961.3248654051868, f1(1));
  ASSERT_DOUBLE_EQ(-2029.9948654051868, f1(2));

  TensorRef<double,3> f2 = f.get(p2);
  ASSERT_DOUBLE_EQ(-461.3248654051871, f2(0));
  ASSERT_DOUBLE_EQ(-461.3248654051871, f2(1));
  ASSERT_DOUBLE_EQ(-480.9448654051871, f2(2));
}

TEST(Program, esprings_simplified) {
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

  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();
  ElementRef p3 = points.addElement();
  ElementRef p4 = points.addElement();
  ElementRef p5 = points.addElement();
  ElementRef p6 = points.addElement();
  ElementRef p7 = points.addElement();
  ElementRef p8 = points.addElement();

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
  ElementRef s1 = springs.addElement(p1,p2);
  ElementRef s2 = springs.addElement(p3,p4);
  ElementRef s3 = springs.addElement(p5,p6);
  ElementRef s4 = springs.addElement(p7,p8);

  ElementRef s5 = springs.addElement(p1,p3);
  ElementRef s6 = springs.addElement(p2,p4);
  ElementRef s7 = springs.addElement(p5,p7);
  ElementRef s8 = springs.addElement(p6,p8);

  ElementRef s9 = springs.addElement(p1,p5);
  ElementRef s10 = springs.addElement(p2,p6);
  ElementRef s11 = springs.addElement(p3,p7);
  ElementRef s12 = springs.addElement(p4,p8);

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

TEST(Program, triangle) {
  Program program;
  std::string programText = R"(
element Trig
  a : float;
end

element Vert
  b : float;
end

extern verts : set{Vert};
extern trigs : set{Trig}(verts, verts);

func dist(t : Trig, v : (Vert*3)) -> ( R : tensor[verts](float))
  R(v(0)) = t.a;
  R(v(1)) = t.a;
  R(v(2)) = t.a;
end

proc main
  R = map dist to trigs with verts reduce +;
  verts.b = --R;
end
  )";

  simit::Set<> verts;
  simit::FieldRef<double> b = verts.addField<double>("b");

  ElementRef v0 = verts.addElement();
  ElementRef v1 = verts.addElement();
  ElementRef v2 = verts.addElement();
  ElementRef v3 = verts.addElement();

  simit::Set<3> trigs(verts,verts,verts);
  simit::FieldRef<double> a = trigs.addField<double>("a");

  ElementRef t0 = trigs.addElement(v0,v1,v2);
  ElementRef t1 = trigs.addElement(v0,v2,v3);

  a.set(t0, 1.0);
  a.set(t1, 0.1);

 // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> func = program.compile("main");
  if (!func) FAIL() << program.getDiagnostics().getMessage();

  func->bind("verts", &verts);
  func->bind("trigs", &trigs);
  func->runSafe();

  // Check outputs
  ASSERT_DOUBLE_EQ(1.1, (double)b.get(v0));
  ASSERT_DOUBLE_EQ(1.0, (double)b.get(v1));
  ASSERT_DOUBLE_EQ(1.1, (double)b.get(v2));
  ASSERT_DOUBLE_EQ(0.1, (double)b.get(v3));
}

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

    func dist_a(s : Spring, p : (Point*2)) ->
        (A : tensor[points,points](float))
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

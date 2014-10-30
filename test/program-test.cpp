#include "gtest/gtest.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "errors.h"

#include "visualizer/visualizer.h"

using namespace std;
using namespace simit;

/// \todo Turn these into a parameterized test suite

TEST(Program, addScalarFields) {
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
  f->run();
  ASSERT_EQ(84.0, x.get(p0));
}

TEST(Program, addVectorFields) {
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
  f->run();

  TensorRef<double,3> vec2 = x.get(p0);
  ASSERT_EQ(2.0, vec2(0));
  ASSERT_EQ(4.0, vec2(1));
  ASSERT_EQ(6.0, vec2(2));
}

TEST(Program, gemv_oop) {
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

    proc mul
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

  std::unique_ptr<Function> f = program.compile("mul");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->run();

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b.get(p0));
  ASSERT_EQ(2.0, b.get(p1));
  ASSERT_EQ(3.0, b.get(p2));

  // Check that outputs are correct
  ASSERT_EQ(3.0, c.get(p0));
  ASSERT_EQ(13.0, c.get(p1));
  ASSERT_EQ(10.0, c.get(p2));
}

TEST(Program, gemv_ip) {
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

    proc mul
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

  std::unique_ptr<Function> f = program.compile("mul");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  f->bind("points", &points);
  f->bind("springs", &springs);
  f->run();

  // Check that outputs are correct
  ASSERT_EQ(3.0, b.get(p0));
  ASSERT_EQ(13.0, b.get(p1));
  ASSERT_EQ(10.0, b.get(p2));
}

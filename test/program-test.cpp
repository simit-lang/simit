#include "gtest/gtest.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "errors.h"

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
  f->bind("points", &points);

  ElementRef p0 = points.addElement();
  x.set(p0, {1.0, 2.0, 3.0});

  TensorRef<double,3> vec1 = x.get(p0);
  ASSERT_EQ(1.0, vec1(0));
  ASSERT_EQ(2.0, vec1(1));
  ASSERT_EQ(3.0, vec1(2));

  f->run();

  TensorRef<double,3> vec2 = x.get(p0);
  ASSERT_EQ(2.0, vec2(0));
  ASSERT_EQ(4.0, vec2(1));
  ASSERT_EQ(6.0, vec2(2));
}

//TEST(Program, spmv) {
//  Program program;
//  std::string programText =
//      "element Point                                                    "
//      "  a : float;                                                     "
//      "  b : float;                                                     "
//      "end                                                              "
//
//      "extern points : Point{};                                         "
//
//      "func dist_mass(p : Point) -> (A : Tensor[points,points](float))  "
//      "  A(p,p) = p.a;                                                  "
//      "end                                                              "
//
//      "proc mul                                                         "
//      "  A = map dist_mass to points reduce +;                          "
//      "  points.b = M * points.b;                                       "
//      "end;                                                             ";
//
//  int errorCode = program.loadString(programText);
//  if (errorCode) FAIL() << program.getDiagnostics().getMessage();
//
//  std::unique_ptr<Function> f = program.compile("mul");
//  if (!f) FAIL() << program.getDiagnostics().getMessage();
//
////  Set<> points;
////  f->bind("points", &points);
////  FieldRef<double,3> x = points.addField<double,3>("x");
////
////  ElementRef p0 = points.addElement();
////  x.set(p0, {1.0, 2.0, 3.0});
//}

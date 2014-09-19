#include "gtest/gtest.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "errors.h"

using namespace std;
using namespace simit;


TEST(Program, addScalarFields) {
  Program program;
  std::string programText =
      "struct Point                                         "
      "  x : float;                                         "
      "end                                                  "
      "func addSets(points : Point{}) -> (points : Point{}) "
      "  points.x = points.x + points.x;                    "
      "end                                                  ";

  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> f = program.compile("addSets");
  if (!f) FAIL() << program.getDiagnostics().getMessage();

  Set<> points;
  f->bind("points", &points);
  FieldRef<double> x = points.addField<double>("x");

  ElementRef p0 = points.addElement();
  x.set(p0, 42.0);

  ASSERT_EQ(42.0, x.get(p0));
  f->run();
  ASSERT_EQ(84.0, x.get(p0));
}

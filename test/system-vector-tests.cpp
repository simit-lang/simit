#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(System, vector_add) {
  Set<> points;
  FieldRef<double> x = points.addField<double>("x");

  ElementRef p0 = points.add();
  x.set(p0, 42.0);

  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();
  f->bind("points", &points);

  f->runSafe();

  ASSERT_EQ(84.0, x.get(p0));
}

TEST(System, vector_add_blocked) {
  Set<> points;
  FieldRef<double,3> x = points.addField<double,3>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();

  x.set(p0, {1.0, 2.0, 3.0});
  x.set(p1, {4.0, 5.0, 6.0});

  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();
  f->bind("points", &points);

  f->runSafe();

  ASSERT_EQ(2.0, x.get(p0)(0));
  ASSERT_EQ(4.0, x.get(p0)(1));
  ASSERT_EQ(6.0, x.get(p0)(2));
  ASSERT_EQ(8.0, x.get(p1)(0));
  ASSERT_EQ(10.0, x.get(p1)(1));
  ASSERT_EQ(12.0, x.get(p1)(2));
}

TEST(System, DISABLED_vector_dot) {
  Set<> points;
  FieldRef<double> x = points.addField<double>("x");
  FieldRef<double> z = points.addField<double>("z");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  x.set(p0, 1.0);
  x.set(p1, 2.0);
  x.set(p2, 3.0);

  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();
  f->bind("points", &points);

  f->runSafe();
  ASSERT_EQ(14.0, z.get(p0));
}

TEST(System, DISABLED_vector_dot_blocked) {
  Set<> points;
  FieldRef<double,3> x = points.addField<double,3>("x");
  FieldRef<double> z = points.addField<double>("z");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  x.set(p0, {1.0,2.0,3.0});
  x.set(p1, {4.0,5.0,6.0});
  x.set(p2, {7.0,8.0,9.0});

  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();
  f->bind("points", &points);

  f->runSafe();
  ASSERT_EQ(285.0, (double)z.get(p0));
}

#include "simit-test.h"

#include <iostream>

#include "graph.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

#define APP_FILE_NAME(appdir) \
  std::string(APPS_DIR) + "/" appdir + "/" + test_info_->name() + ".sim"

TEST(apps, esprings) {
  Set points;
  auto     x = points.addField<simit_float,3>("x");
  auto     v = points.addField<simit_float,3>("v");
  auto     m = points.addField<simit_float>("m");
  auto fixed = points.addField<bool>("fixed");

  Set springs(points,points);
  auto  k = springs.addField<float>("k");
  auto l0 = springs.addField<float>("l0");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  x(p0) = {-1.0, 0.0, 0.0};
  x(p1) = { 0.0, 0.0, 0.0};
  x(p2) = { 1.0, 0.0, 0.0};
  v(p0) = {-0.8, 0.0, 0.0};
  v(p1) = { 0.0, 0.0, 0.0};
  v(p2) = { 0.0, 0.0, 0.0};
  m(p0) =  0.5;
  m(p1) =  1.0;
  m(p2) =  2.0;
  fixed(p1) = true;

  k(s0)  = 1;
  k(s1)  = 2;
  l0(s0) = 0.8;
  l0(s1) = 1.2;

  Function func = loadFunction(APP_FILE_NAME("springs"), "timestep");
  if (!func.defined()) FAIL();
  func.bind("points",  &points);
  func.bind("springs", &springs);
  func.runSafe();
  func.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(-2.5760651710122078,      x(p0)(0));
  SIMIT_ASSERT_FLOAT_EQ(-2.9331861307425285e-05, x(p0)(1));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     x(p0)(2));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     x(p1)(0));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     x(p1)(1));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     x(p1)(2));
  SIMIT_ASSERT_FLOAT_EQ( 1.0,                     x(p2)(0));
  SIMIT_ASSERT_FLOAT_EQ(-0.000029331899999999999, x(p2)(1));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     x(p2)(2));

  SIMIT_ASSERT_FLOAT_EQ(-0.78406907101309153,    v(p0)(0));
  SIMIT_ASSERT_FLOAT_EQ(-1.9521861307425286e-05, v(p0)(1));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     v(p0)(2));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     v(p1)(0));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     v(p1)(1));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     v(p1)(2));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     v(p2)(0));
  SIMIT_ASSERT_FLOAT_EQ(-0.0000195219,            v(p2)(1));
  SIMIT_ASSERT_FLOAT_EQ( 0.0,                     v(p2)(2));
}

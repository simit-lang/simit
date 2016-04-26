#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "error.h"
#include "types.h"

using namespace std;
using namespace simit;

TEST(System, vector_add) {
  Set points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");

  ElementRef p0 = points.add();
  x.set(p0, 42.0);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(84.0, (int)x.get(p0));
}

TEST(System, vector_add_blocked) {
  Set points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();

  x.set(p0, {1.0, 2.0, 3.0});
  x.set(p1, {4.0, 5.0, 6.0});

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(2.0, x.get(p0)(0));
  SIMIT_EXPECT_FLOAT_EQ(4.0, x.get(p0)(1));
  SIMIT_EXPECT_FLOAT_EQ(6.0, x.get(p0)(2));
  SIMIT_EXPECT_FLOAT_EQ(8.0, x.get(p1)(0));
  SIMIT_EXPECT_FLOAT_EQ(10.0, x.get(p1)(1));
  SIMIT_EXPECT_FLOAT_EQ(12.0, x.get(p1)(2));
}

TEST(System, vector_dot) {
  Set points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");
  FieldRef<simit_float> z = points.addField<simit_float>("z");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  x.set(p0, 1.0);
  x.set(p1, 2.0);
  x.set(p2, 3.0);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();
  SIMIT_EXPECT_FLOAT_EQ(14.0, (int)z.get(p0));
}

TEST(System, vector_dot_blocked) {
  Set points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  FieldRef<simit_float> z = points.addField<simit_float>("z");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  x.set(p0, {1.0,2.0,3.0});
  x.set(p1, {4.0,5.0,6.0});
  x.set(p2, {7.0,8.0,9.0});

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();
  SIMIT_EXPECT_FLOAT_EQ(285.0, (simit_float)z.get(p0));
}

TEST(System, vector_dot_intrinsic) {
  Set points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");
  FieldRef<simit_float> z = points.addField<simit_float>("z");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();
  ElementRef p3 = points.add();
  ElementRef p4 = points.add();
  ElementRef p5 = points.add();
  ElementRef p6 = points.add();
  ElementRef p7 = points.add();
  ElementRef p8 = points.add();
  ElementRef p9 = points.add();
  ElementRef p10 = points.add();
  x.set(p0, 1.0);
  x.set(p1, 2.0);
  x.set(p2, 3.0);
  x.set(p3, 4.0);
  x.set(p4, 5.0);
  x.set(p5, 6.0);
  x.set(p6, 7.0);
  x.set(p7, 8.0);
  x.set(p8, 9.0);
  x.set(p9, 10.0);
  x.set(p10, 11.0);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();
  SIMIT_EXPECT_FLOAT_EQ(506.0, (int)z.get(p0));
}

TEST(System, vector_assign_blocked) {
  Set points;
  FieldRef<simit_float,2> x = points.addField<simit_float,2>("x");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  x.set(p0, {1.0, 2.0});
  x.set(p1, {3.0, 4.0});

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(2.0, x.get(p0)(0));
  SIMIT_EXPECT_FLOAT_EQ(4.0, x.get(p0)(1));
  SIMIT_EXPECT_FLOAT_EQ(6.0, x.get(p1)(0));
  SIMIT_EXPECT_FLOAT_EQ(8.0, x.get(p1)(1));
}

TEST(System, vector_add_large_system) {
  Set points;
  FieldRef<simit_float> x = points.addField<simit_float>("x");

  std::vector<ElementRef> ps;
  for(size_t i = 0; i < 2557; ++i) {
    ps.push_back(points.add());
    x.set(ps.back(), (simit_float)i);
  }

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  for(size_t i = 0; i < ps.size(); ++i) {
    SIMIT_EXPECT_FLOAT_EQ(i*2, (size_t)x.get(ps[i]));
  }
}

extern "C" void ext_vec3f_add(simit_float *a, simit_float *b, simit_float *c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

TEST(System, vector_add_using_extern) {
  Set points;
  FieldRef<simit_float> a = points.addField<simit_float>("a");
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  ElementRef p0 = points.add();
  a.set(p0, 42.0);
  b.set(p0, 24.0);
  c.set(p0, -1.0);
  
  ElementRef p1 = points.add();
  a.set(p1, 20.0);
  b.set(p1, 14.0);
  c.set(p1, -1.0);
  
  ElementRef p2 = points.add();
  a.set(p2, 12.0);
  b.set(p2, 21.0);
  c.set(p2, -1.0);
  
  
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(66.0, (int)c.get(p0));
  SIMIT_EXPECT_FLOAT_EQ(34.0, (int)c.get(p1));
  SIMIT_EXPECT_FLOAT_EQ(33.0, (int)c.get(p2));

}


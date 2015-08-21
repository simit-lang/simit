#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, esprings) {
  // Points
  Set points;
  FieldRef<simit_float,3> x = points.addField<simit_float,3>("x");
  FieldRef<simit_float,3> v = points.addField<simit_float,3>("v");

  FieldRef<simit_float,3> fs = points.addField<simit_float,3>("fs");
  FieldRef<simit_float,3> fg = points.addField<simit_float,3>("fg");
  FieldRef<simit_float,3> M = points.addField<simit_float,3>("M");
  FieldRef<simit_float,3> p = points.addField<simit_float,3>("p");

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
  Set springs(points,points);
  FieldRef<simit_float> l0 = springs.addField<simit_float>("l0");
  FieldRef<simit_float> m = springs.addField<simit_float>("m");

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
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  for (size_t i=0; i < 10; ++i) {
    func.runSafe();
  }

  // Check outputs
  TensorRef<simit_float,3> x1 = x.get(p1);
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x1(0));
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x1(1));
  SIMIT_ASSERT_FLOAT_EQ(-0.0130301827915078, x1(2));

  TensorRef<simit_float,3> x2 = x.get(p2);
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x2(0));
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x2(1));
  SIMIT_ASSERT_FLOAT_EQ(-0.0130301827915078, x2(2));

  TensorRef<simit_float,3> x3 = x.get(p3);
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x3(0));
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x3(1));
  SIMIT_ASSERT_FLOAT_EQ(-0.0130301827915078, x3(2));

  TensorRef<simit_float,3> x4 = x.get(p4);
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x4(0));
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x4(1));
  SIMIT_ASSERT_FLOAT_EQ(-0.0130301827915078, x4(2));

  TensorRef<simit_float,3> x5 = x.get(p5);
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x5(0));
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x5(1));
  SIMIT_ASSERT_FLOAT_EQ(0.905120182791508, x5(2));

  TensorRef<simit_float,3> x6 = x.get(p6);
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x6(0));
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x6(1));
  SIMIT_ASSERT_FLOAT_EQ(0.905120182791508, x6(2));

  TensorRef<simit_float,3> x7 = x.get(p7);
  SIMIT_ASSERT_FLOAT_EQ(0.0409248172084922, x7(0));
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x7(1));
  SIMIT_ASSERT_FLOAT_EQ(0.905120182791508, x7(2));

  TensorRef<simit_float,3> x8 = x.get(p8);
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x8(0));
  SIMIT_ASSERT_FLOAT_EQ(0.959075182791508, x8(1));
  SIMIT_ASSERT_FLOAT_EQ(0.905120182791508, x8(2));
}

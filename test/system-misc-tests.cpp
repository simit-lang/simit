#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(System, misc_triangle) {
  simit::Set<> verts;
  simit::FieldRef<double> b = verts.addField<double>("b");

  ElementRef v0 = verts.add();
  ElementRef v1 = verts.add();
  ElementRef v2 = verts.add();
  ElementRef v3 = verts.add();

  simit::Set<3> trigs(verts,verts,verts);
  simit::FieldRef<double> a = trigs.addField<double>("a");

  ElementRef t0 = trigs.add(v0,v1,v2);
  ElementRef t1 = trigs.add(v1,v2,v3);

  a.set(t0, 1.0);
  a.set(t1, 0.1);

 // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("verts", &verts);
  f->bind("trigs", &trigs);

  f->runSafe();

  // Check outputs
  ASSERT_DOUBLE_EQ(1.0, b.get(v0));
  ASSERT_DOUBLE_EQ(1.1, b.get(v1));
  ASSERT_DOUBLE_EQ(1.1, b.get(v2));
  ASSERT_DOUBLE_EQ(0.1, b.get(v3));
}

TEST(System, DISABLED_misc_map_one_set) {
  // Points
  Set<> points;
  FieldRef<double> a = points.addField<double>("a");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  a.set(p0, 1.0);
  a.set(p1, 2.0);
  a.set(p2, 3.0);

  // Compile program and bind arguments
  std::unique_ptr<Function> f = getFunction(TEST_FILE_NAME, "main");
  if (!f) FAIL();

  f->bind("points", &points);
  f->runSafe();

  // Check outputs
  ASSERT_DOUBLE_EQ(2, a.get(p0));
  ASSERT_DOUBLE_EQ(4, a.get(p1));
  ASSERT_DOUBLE_EQ(6, a.get(p2));
}

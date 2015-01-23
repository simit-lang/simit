#include "simit-test.h"

#include "graph.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(Program, cg) {
  Set points;
  FieldRef<simit_float>  b = points.addField<simit_float>("b");
  FieldRef<simit_float>  c = points.addField<simit_float>("c");
  FieldRef<int>    id = points.addField<int>("id");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  Set springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 4.0);
  a.set(s1, 5.0);

  Function func = getFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();

  ASSERT_SIMIT_FLOAT_EQ(0.95883777239709455653, (simit_float)c.get(p0));
  ASSERT_SIMIT_FLOAT_EQ(1.98789346246973352983, (simit_float)c.get(p1));
  ASSERT_SIMIT_FLOAT_EQ(3.05326876513317202466, (simit_float)c.get(p2));
}

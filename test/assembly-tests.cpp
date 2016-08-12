#include "simit-test.h"

#include "init.h"
#include "graph.h"
#include "tensor.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(DISABLED_assembly, unary_edges) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> a = V.addField<int>("a");

  Set E(V);
  E.add(v0);
  E.add(v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ((int)a(v0), 1);
  ASSERT_EQ((int)a(v1), 0);
  ASSERT_EQ((int)a(v2), 1);
}

TEST(DISABLED_assembly, rectangular_matrix) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> c = V.addField<int>("c");
  c(v0) = 1;
  c(v1) = 2;
  c(v2) = 3;

  Set E(V,V);
  ElementRef e0 = E.add(v0, v1);
  ElementRef e1 = E.add(v1, v2);
  FieldRef<int> a = E.addField<int>("a");

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ((int)a(e0), 3);
  ASSERT_EQ((int)a(e1), 5);
}

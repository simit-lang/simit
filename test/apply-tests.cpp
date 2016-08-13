#include "simit-test.h"

#include "init.h"
#include "graph.h"
#include "tensor.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(apply, vertices) {
  Set V;
  FieldRef<int> a = V.addField<int>("a");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  a.set(v0, 1.0);
  a.set(v1, 2.0);
  a.set(v2, 3.0);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.runSafe();

  ASSERT_EQ(2.0, (int)a.get(v0));
  ASSERT_EQ(4.0, (int)a.get(v1));
  ASSERT_EQ(6.0, (int)a.get(v2));
}

TEST(apply, edges_no_endpoints) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();

  // Springs
  Set E(V,V);
  FieldRef<int> a = E.addField<int>("a");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);

  a.set(e0, 1.0);
  a.set(e1, 2.0);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(2, (int)a(e0));
  ASSERT_EQ(4, (int)a(e1));
}

TEST(apply, edges_binary_gather) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> a = V.addField<int>("a");
  a(v0) = 1.0;
  a(v1) = 2.0;
  a(v2) = 3.0;

  Set E(V,V);
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  FieldRef<int> b = E.addField<int>("b");

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(3.0, (int)b(e0));
  ASSERT_EQ(5.0, (int)b(e1));
}

#include "simit-test.h"

#include "init.h"
#include "graph.h"
#include "tensor.h"
#include "program.h"
#include "error.h"

using namespace std;
using namespace simit;

TEST(assembly, vertices) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> a = V.addField<int>("a");
  FieldRef<int> b = V.addField<int>("b");
  a(v0) = 1;
  a(v1) = 2;
  a(v2) = 3;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.runSafe();

  ASSERT_EQ(2, b(v0));
  ASSERT_EQ(4, b(v1));
  ASSERT_EQ(6, b(v2));
}

TEST(assembly, vertices_two_results) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> a = V.addField<int>("a");
  FieldRef<int> b = V.addField<int>("b");
  a(v0) = 1;
  a(v1) = 2;
  a(v2) = 3;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.runSafe();

  ASSERT_EQ( 6, (int)b(v0));
  ASSERT_EQ(24, (int)b(v1));
  ASSERT_EQ(54, (int)b(v2));
}

TEST(assembly, edges_no_endpoints) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();

  Set E(V,V);
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  FieldRef<int> a = E.addField<int>("a");
  FieldRef<int> b = E.addField<int>("b");
  a(e0) = 1;
  a(e1) = 2;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(2, (int)b(e0));
  ASSERT_EQ(4, (int)b(e1));
}

TEST(assembly, edges_unary) {
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

TEST(assembly, edges_binary) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> a = V.addField<int>("a");

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ((int)a(v0), 1);
  ASSERT_EQ((int)a(v1), 2);
  ASSERT_EQ((int)a(v2), 1);
}

TEST(assembly, edges_tertiary) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  ElementRef v3 = V.add();
  FieldRef<int> b = V.addField<int>("b");

  Set E(V,V,V);
  ElementRef e0 = E.add(v0,v1,v2);
  ElementRef e1 = E.add(v1,v2,v3);
  simit::FieldRef<int> a = E.addField<int>("a");
  a(e0) = 10;
  a(e1) = 1;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(10, b(v0));
  ASSERT_EQ(11, b(v1));
  ASSERT_EQ(11, b(v2));
  ASSERT_EQ(1,  b(v3));
}

TEST(assembly, edges_two_results) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> b = V.addField<int>("b");

  Set E(V,V);
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  FieldRef<int> a = E.addField<int>("a");
  a(e0) = 1;
  a(e1) = 2;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(43,  (int)b(v0));
  ASSERT_EQ(177, (int)b(v1));
  ASSERT_EQ(168, (int)b(v2));
}

TEST(assembly, matrix_ve) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> c = V.addField<int>("c");

  Set E(V,V);
  ElementRef e0 = E.add(v0, v1);
  ElementRef e1 = E.add(v1, v2);
  FieldRef<int> a = E.addField<int>("a");
  a(e0) = 1;
  a(e1) = 2;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ((int)c(v0), 1);
  ASSERT_EQ((int)c(v1), 3);
  ASSERT_EQ((int)c(v2), 2);
}

TEST(assembly, matrix_ev) {
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

TEST(assembly, matrix_vv) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> a = V.addField<int>("a");
  FieldRef<int> b = V.addField<int>("b");
  a(v0) = 1;
  a(v1) = 1;
  a(v2) = 1;

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(2, (int)b(v0));
  ASSERT_EQ(4, (int)b(v1));
  ASSERT_EQ(2, (int)b(v2));
}

TEST(assembly, blocked) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> x = V.addField<int>("x");
  FieldRef<int> z = V.addField<int>("z");
  x(v0) = 1;
  x(v1) = 2;
  x(v2) = 3;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.runSafe();

  ASSERT_EQ(81,  z(v0));
  ASSERT_EQ(324, z(v1));
  ASSERT_EQ(729, z(v2));
}

TEST(assembly, block_component_write) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int,2> x = V.addField<int,2>("a");

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(1, x(v0)(0));
  ASSERT_EQ(0, x(v0)(1));
  ASSERT_EQ(1, x(v1)(0));
  ASSERT_EQ(0, x(v1)(1));
  ASSERT_EQ(0, x(v2)(0));
  ASSERT_EQ(0, x(v2)(1));
}

TEST(DISABLED_assembly, issue45) {
  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int,2> a = V.addField<int,2>("a");
  FieldRef<int,2> b = V.addField<int,2>("b");
  a(v0) = {1, 2};
  a(v1) = {3, 4};
  a(v2) = {5, 6};

  Set E(V,V);
  E.add(v0,v1);
  E.add(v1,v2);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(1, (int)b(v0)(0));
  ASSERT_EQ(0, (int)b(v0)(1));
  ASSERT_EQ(3, (int)b(v1)(0));
  ASSERT_EQ(0, (int)b(v1)(1));
  ASSERT_EQ(0, (int)b(v2)(0));
  ASSERT_EQ(0, (int)b(v2)(1));
}

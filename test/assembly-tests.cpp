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

TEST(assembly, edges_heterogeneous) {
  Set P;
  ElementRef p0 = P.add();
  ElementRef p1 = P.add();
  ElementRef p2 = P.add();
  FieldRef<int> c = P.addField<int>("c");
  c(p0) = 2;
  c(p1) = 3;
  c(p2) = 4;

  Set V;
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  FieldRef<int> b = V.addField<int>("b");
  b(v0) = 1;
  b(v1) = 2;
  b(v2) = 3;

  Set E(P,V);
  ElementRef e0 = E.add(p0,v0);
  ElementRef e1 = E.add(p1,v0);
  ElementRef e2 = E.add(p1,v1);
  ElementRef e3 = E.add(p1,v2);
  ElementRef e4 = E.add(p2,v2);
  FieldRef<int> a = E.addField<int>("a");
  a(e0) = 3;
  a(e1) = 4;
  a(e2) = 5;
  a(e3) = 6;
  a(e4) = 7;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("P", &P);
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(6, (int)c(p0));
  ASSERT_EQ(64, (int)c(p1));
  ASSERT_EQ(42, (int)c(p2));
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

TEST(assembly, matrix_ve_heterogeneous) {
  Set V0;
  FieldRef<int> b0 = V0.addField<int>("b");
  std::vector<ElementRef> v0;
  v0.reserve(4);
  for (int i = 0; i < 4; ++i) {
    v0[i] = V0.add();
  }

  Set V1;
  FieldRef<int> b1 = V1.addField<int>("b");
  std::vector<ElementRef> v1;
  v1.reserve(9);
  for (int i = 0; i < 9; ++i) {
    v1[i] = V1.add();
  }

  Set E(V1,V1,V0,V1,V1);
  FieldRef<int> a = E.addField<int>("a");
  ElementRef e0 = E.add(v1[0],v1[1],v0[0],v1[3],v1[4]);
  ElementRef e1 = E.add(v1[1],v1[2],v0[1],v1[4],v1[5]);
  ElementRef e2 = E.add(v1[3],v1[4],v0[2],v1[6],v1[7]);
  ElementRef e3 = E.add(v1[4],v1[5],v0[3],v1[7],v1[8]);
  a(e0) = 1;
  a(e1) = 2;
  a(e2) = 3;
  a(e3) = 4;

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V0", &V0);
  func.bind("V1", &V1);
  func.bind("E", &E);
  func.runSafe();

  for (int i = 0; i < 4; ++i) {
    ASSERT_EQ(i + 1, (int)b0(v0[i]));
  }
  
  const std::vector<int> b1Expected = {1, 4, 4, 6, 20, 16, 9, 24, 16};
  for (int i = 0; i < 9; ++i) {
    ASSERT_EQ(b1Expected[i], (int)b1(v1[i]));
  }
}

TEST(assembly, matrix_ev_heterogeneous) {
  Set V0;
  FieldRef<int> a0 = V0.addField<int>("a");
  std::vector<ElementRef> v0;
  v0.reserve(4);
  for (int i = 0; i < 4; ++i) {
    v0[i] = V0.add();
    a0(v0[i]) = i + 1;
  }

  Set V1;
  FieldRef<int> a1 = V1.addField<int>("a");
  std::vector<ElementRef> v1;
  v1.reserve(9);
  for (int i = 0; i < 9; ++i) {
    v1[i] = V1.add();
    a1(v1[i]) = i + 1;
  }

  Set E(V1,V1,V0,V1,V1);
  FieldRef<int> b0 = E.addField<int>("b0");
  FieldRef<int> b1 = E.addField<int>("b1");
  ElementRef e0 = E.add(v1[0],v1[1],v0[0],v1[3],v1[4]);
  ElementRef e1 = E.add(v1[1],v1[2],v0[1],v1[4],v1[5]);
  ElementRef e2 = E.add(v1[3],v1[4],v0[2],v1[6],v1[7]);
  ElementRef e3 = E.add(v1[4],v1[5],v0[3],v1[7],v1[8]);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V0", &V0);
  func.bind("V1", &V1);
  func.bind("E", &E);
  func.runSafe();

  ASSERT_EQ(1, (int)b0(e0));
  ASSERT_EQ(2, (int)b0(e1));
  ASSERT_EQ(3, (int)b0(e2));
  ASSERT_EQ(4, (int)b0(e3));
  
  ASSERT_EQ(37, (int)b1(e0));
  ASSERT_EQ(47, (int)b1(e1));
  ASSERT_EQ(67, (int)b1(e2));
  ASSERT_EQ(77, (int)b1(e3));
}

TEST(assembly, matrix_vv_heterogeneous) {
  Set V0;
  FieldRef<int> a0 = V0.addField<int>("a");
  FieldRef<int> b0 = V0.addField<int>("b");
  std::vector<ElementRef> v0;
  v0.reserve(4);
  for (int i = 0; i < 4; ++i) {
    v0[i] = V0.add();
    a0(v0[i]) = i + 1;
  }

  Set V1;
  FieldRef<int> a1 = V1.addField<int>("a");
  FieldRef<int> b1 = V1.addField<int>("b");
  std::vector<ElementRef> v1;
  v1.reserve(9);
  for (int i = 0; i < 9; ++i) {
    v1[i] = V1.add();
    a1(v1[i]) = i + 1;
  }

  Set E(V1,V1,V0,V1,V1);
  E.add(v1[0],v1[1],v0[0],v1[3],v1[4]);
  E.add(v1[1],v1[2],v0[1],v1[4],v1[5]);
  E.add(v1[3],v1[4],v0[2],v1[6],v1[7]);
  E.add(v1[4],v1[5],v0[3],v1[7],v1[8]);

  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V0", &V0);
  func.bind("V1", &V1);
  func.bind("E", &E);
  func.runSafe();

  const std::vector<int> b0Expected = {37, 47, 67, 77};
  for (int i = 0; i < 4; ++i) {
    ASSERT_EQ(b0Expected[i], (int)b0(v0[i]));
  }
  
  const std::vector<int> b1Expected = {1, 4, 4, 6, 20, 16, 9, 24, 16};
  for (int i = 0; i < 9; ++i) {
    ASSERT_EQ(b1Expected[i], (int)b1(v1[i]));
  }
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

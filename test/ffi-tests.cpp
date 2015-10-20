#include "simit-test.h"

#include "tensor.h"
#include "ir.h"
#include "intrinsics.h"
#include "ir_printer.h"
#include "graph.h"
#include "program.h"
#include "error.h"
#include "types.h"

#include "ffi.h"

using namespace std;
using namespace testing;
using namespace simit::ir;
using namespace simit::backend;

extern "C" void test_ext_func(simit_float a, simit_float b, simit_float *c) {
  *c = a+b;
}

TEST(ffi, extern_func) {
  Var a("a", Float);
  Var b("b", Float);
  Var c("c", Float);
  
  // this test is akin to declaring an external func called "test_ext_func"
  // and then creating another func "tst_func" that calls the external func.
  
  Func ext_func = Func("test_ext_func", {a, b}, {c}, Func::External);
  Stmt call_to_ext_func = CallStmt::make({c}, ext_func, {a,b});
  //Stmt body = AssignStmt::make(c, call_to_ext_func);
  Func tst_func = Func("test_extern_func", {a,b}, {c}, call_to_ext_func);
  
  unique_ptr<Backend> backend = getTestBackend();
  simit::Function function = backend->compile(tst_func);
  
  
  simit_float aArg = 1.0;
  simit_float bArg = 4.0;
  simit_float cRes = -1.0;

  function.bind("a", &aArg);
  function.bind("b", &bArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_ASSERT_FLOAT_EQ(1.0+4.0, cRes);
}

extern "C" void ext_vec3f_add(simit_float *a, simit_float *b, simit_float *c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

TEST(ffi, vector_add_using_extern) {
  simit::Set points;
  simit::FieldRef<simit_float> a = points.addField<simit_float>("a");
  simit::FieldRef<simit_float> b = points.addField<simit_float>("b");
  simit::FieldRef<simit_float> c = points.addField<simit_float>("c");

  simit::ElementRef p0 = points.add();
  a.set(p0, 42.0);
  b.set(p0, 24.0);
  c.set(p0, -1.0);
  
  simit::ElementRef p1 = points.add();
  a.set(p1, 20.0);
  b.set(p1, 14.0);
  c.set(p1, -1.0);
  
  simit::ElementRef p2 = points.add();
  a.set(p2, 12.0);
  b.set(p2, 21.0);
  c.set(p2, -1.0);
  
  
  simit::Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);

  func.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(66.0, (int)c.get(p0));
  SIMIT_EXPECT_FLOAT_EQ(34.0, (int)c.get(p1));
  SIMIT_EXPECT_FLOAT_EQ(33.0, (int)c.get(p2));

}

extern "C" void ext_csr_gemv(simit_float* vals, int* row_start, int* col_idx,
                             int rows, int cols, int nnz, int rowblock, int colblock,
                             simit_float* x, simit_float* y) {
  int* csrRowStart;
  int* csrColIdx;
  simit_float* csrVals;
  
  convert_to_csr(vals, row_start, col_idx, rows, cols, nnz, rowblock, colblock, &csrRowStart, &csrColIdx, &csrVals);
  
  std::cout << "Done with conversion.\n";
  
  // spmv
  for (int i=0; i<rows; i++) {
    y[i] = 0;
    for (int j=csrRowStart[i]; j<csrRowStart[i+1]; j++) {
      y[i] += csrVals[j] * x[csrColIdx[j]];
    }
  }
  free(csrRowStart);
  free(csrColIdx);
  free(csrVals);
}

TEST(ffi, to_csr) {
  // Points
  simit::Set points;
  simit::FieldRef<simit_float> b = points.addField<simit_float>("b");
  simit::FieldRef<simit_float> c = points.addField<simit_float>("c");

  simit::ElementRef p0 = points.add();
  simit::ElementRef p1 = points.add();
  simit::ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Taint c
  c.set(p0, 42.0);
  c.set(p2, 42.0);

  // Springs
  simit::Set springs(points,points);
  simit::FieldRef<simit_float> a = springs.addField<simit_float>("a");

  simit::ElementRef s0 = springs.add(p0,p1);
  simit::ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  simit::Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();
  
  SIMIT_EXPECT_FLOAT_EQ(3.0, c.get(p0));
  SIMIT_EXPECT_FLOAT_EQ(13.0, c.get(p1));
  SIMIT_EXPECT_FLOAT_EQ(10.0, c.get(p2));
  
}

TEST(ffi, to_csr_from_blocked) {
  // Points
  simit::Set points;
  simit::FieldRef<simit_float,2> b = points.addField<simit_float,2>("b");
  simit::FieldRef<simit_float,2> c = points.addField<simit_float,2>("c");

  simit::ElementRef p0 = points.add();
  simit::ElementRef p1 = points.add();
  simit::ElementRef p2 = points.add();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  simit::Set springs(points,points);
  simit::FieldRef<simit_float,2,2> a = springs.addField<simit_float,2,2>("a");

  simit::ElementRef s0 = springs.add(p0,p1);
  simit::ElementRef s1 = springs.add(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  simit::Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();

  // Check that outputs are correct
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  simit::TensorRef<simit_float,2> c0 = c.get(p0);
  ASSERT_EQ(16.0, c0(0));
  ASSERT_EQ(36.0, c0(1));

  simit::TensorRef<simit_float,2> c1 = c.get(p1);
  ASSERT_EQ(116.0, c1(0));
  ASSERT_EQ(172.0, c1(1));

  simit::TensorRef<simit_float,2> c2 = c.get(p2);
  ASSERT_EQ(100.0, c2(0));
  ASSERT_EQ(136.0, c2(1));
}


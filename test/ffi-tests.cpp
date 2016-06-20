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
using namespace simit;
using namespace simit::ir;
using namespace simit::ffi;

static bool noargsVisited = false;
extern "C" void noargs() {
  noargsVisited = true;
}

TEST(ffi, noargs) {
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.runSafe();
  ASSERT_TRUE(noargsVisited);
  noargsVisited = false;
}

extern "C"
void sadd(float a, float b, float* c) {
  *c = a+b;
}
extern "C"
void dadd(double a, double b, double* c) {
  *c = a+b;
}

TEST(ffi, scalar_add) {
  Var a("a", ir::Float);
  Var b("b", ir::Float);
  Var c("c", ir::Float);
  
  Func ext_func = Func("add", {a, b}, {c}, Func::External);
  Stmt call_to_ext_func = CallStmt::make({c}, ext_func, {a,b});
  Func tst_func = Func("test_extern_func", {a,b}, {c}, call_to_ext_func);
  
  unique_ptr<backend::Backend> backend = getTestBackend();
  Function function = backend->compile(tst_func);

  simit_float aArg = (simit_float)1.0;
  simit_float bArg = (simit_float)4.0;
  simit_float cRes = (simit_float)-1.0;

  function.bind("a", &aArg);
  function.bind("b", &bArg);
  function.bind("c", &cRes);

  function.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(1.0+4.0, cRes);
}

extern "C"
void snegtwo(float a, float b, float* c, float* d) {
  *c = -a;
  *d = -b;
}
extern "C"
void dnegtwo(double a, double b, double* c, double* d) {
  *c = -a;
  *d = -b;
}

TEST(ffi, scalar_neg_two) {
  Var a("a", ir::Float);
  Var b("b", ir::Float);
  Var c("c", ir::Float);
  Var d("d", ir::Float);

  Func ext_func = Func("negtwo", {a, b}, {c, d}, Func::External);
  Stmt call_to_ext_func = CallStmt::make({c, d}, ext_func, {a,b});
  Func tst_func = Func("test_extern_func", {a,b}, {c,d}, call_to_ext_func);

  unique_ptr<backend::Backend> backend = getTestBackend();
  Function function = backend->compile(tst_func);

  simit_float aArg = (simit_float)1.0;
  simit_float bArg = (simit_float)2.0;
  simit_float cRes = (simit_float)0.0;
  simit_float dRes = (simit_float)0.0;

  function.bind("a", &aArg);
  function.bind("b", &bArg);
  function.bind("c", &cRes);
  function.bind("d", &dRes);

  function.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(-aArg, cRes);
  SIMIT_EXPECT_FLOAT_EQ(-bArg, dRes);
}

template<typename Float>
void vecadd(int aN, Float* a, int bN, Float* b, int cN, Float* c) {
  iassert(aN == bN && bN == cN);
  for (int i=0; i<aN; ++i) {
    c[i] = a[i] + b[i];
  }
}
extern "C"
void svecadd(int aN, float* a, int bN, float* b, int cN, float* c) {
  vecadd<float>(aN,a, bN,b, cN,c);
}
extern "C"
void dvecadd(int aN, double* a, int bN, double* b, int cN, double* c) {
  vecadd<double>(aN,a, bN,b, cN,c);
}

TEST(ffi, vector_add) {
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

template<typename Float>
void gemv(int BN,int BM, int BNN,int BMM, int* BrowPtr, int* BcolIdx, Float* B,
          int cN, Float* c, int aN, Float* a) {
  int* csrRowStart;
  int* csrColIdx;
  Float* csrVals;

  simit::ffi::convertToCSR(B, BrowPtr, BcolIdx, BN, BM, BNN, BMM,
                           &csrRowStart, &csrColIdx, &csrVals);

  // spmv
  for (int i=0; i<BN; i++) {
    a[i] = 0;
    for (int j=csrRowStart[i]; j<csrRowStart[i+1]; j++) {
      a[i] += csrVals[j] * c[csrColIdx[j]];
    }
  }
  free(csrRowStart);
  free(csrColIdx);
  free(csrVals);
}
extern "C"
void sgemv(int BN,int BM, int BNN,int BMM, int* BrowPtr,int* BcolIdx, float* B,
           int cN, float* c, int aN, float* a) {
  gemv<float>(BN, BM, BNN, BMM, BrowPtr, BcolIdx, B, cN, c, aN, a);
}
extern "C"
void dgemv(int BN,int BM, int BNN,int BMM, int* BrowPtr,int* BcolIdx, double* B,
           int cN, double* c, int aN, double* a) {
  gemv<double>(BN, BM, BNN, BMM, BrowPtr, BcolIdx, B, cN, c, aN, a);
}

TEST(ffi, gemv) {
  // Points
  Set points;
  FieldRef<simit_float> b = points.addField<simit_float>("b");
  FieldRef<simit_float> c = points.addField<simit_float>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, 1.0);
  b.set(p1, 2.0);
  b.set(p2, 3.0);

  // Taint c
  c.set(p0, 42.0);
  c.set(p2, 42.0);

  // Springs
  Set springs(points,points);
  FieldRef<simit_float> a = springs.addField<simit_float>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, 1.0);
  a.set(s1, 2.0);

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();

  func.bind("points", &points);
  func.bind("springs", &springs);

  func.runSafe();
  
  SIMIT_EXPECT_FLOAT_EQ(3.0, c.get(p0));
  SIMIT_EXPECT_FLOAT_EQ(13.0, c.get(p1));
  SIMIT_EXPECT_FLOAT_EQ(10.0, c.get(p2));
}

TEST(ffi, gemv_blocked) {
  // Points
  Set points;
  FieldRef<simit_float,2> b = points.addField<simit_float,2>("b");
  FieldRef<simit_float,2> c = points.addField<simit_float,2>("c");

  ElementRef p0 = points.add();
  ElementRef p1 = points.add();
  ElementRef p2 = points.add();

  b.set(p0, {1.0, 2.0});
  b.set(p1, {3.0, 4.0});
  b.set(p2, {5.0, 6.0});

  // Taint c
  c.set(p0, {42.0, 42.0});
  c.set(p2, {42.0, 42.0});

  // Springs
  Set springs(points,points);
  FieldRef<simit_float,2,2> a = springs.addField<simit_float,2,2>("a");

  ElementRef s0 = springs.add(p0,p1);
  ElementRef s1 = springs.add(p1,p2);

  a.set(s0, {1.0, 2.0, 3.0, 4.0});
  a.set(s1, {5.0, 6.0, 7.0, 8.0});

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("points", &points);
  func.bind("springs", &springs);
  func.runSafe();

  // Check that outputs are correct
  // TODO: add support for comparing a tensorref like so: b0 == {1.0, 2.0, 3.0}
  TensorRef<simit_float,2> c0 = c.get(p0);
  SIMIT_EXPECT_FLOAT_EQ(16.0, c0(0));
  SIMIT_EXPECT_FLOAT_EQ(36.0, c0(1));

  TensorRef<simit_float,2> c1 = c.get(p1);
  SIMIT_EXPECT_FLOAT_EQ(116.0, c1(0));
  SIMIT_EXPECT_FLOAT_EQ(172.0, c1(1));

  TensorRef<simit_float,2> c2 = c.get(p2);
  SIMIT_EXPECT_FLOAT_EQ(100.0, c2(0));
  SIMIT_EXPECT_FLOAT_EQ(136.0, c2(1));
}


// Tests that use Eigen
#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

template<typename Float>
Eigen::SparseMatrix<Float,Eigen::RowMajor>
csr2eigen(int N, int M, int* rowPtr, int* colIdx, Float* vals) {
  std::vector< Eigen::Triplet<double>> coords;
  coords.reserve(rowPtr[N]);
  for (int i=0; i<N; ++i) {
    for (int ij=rowPtr[i]; ij<rowPtr[i+1]; ++ij) {
      int j = colIdx[ij];
      coords.push_back({i,j,vals[ij]});
    }
  }
  Eigen::SparseMatrix<Float,Eigen::RowMajor> mat(N, M);
  mat.setFromTriplets(coords.begin(), coords.end());
  mat.makeCompressed();
  return mat;
}

extern "C" void dmatrix_neg(int Bn,int Bm,int Bnn,int Bmm,
                            int* BrowPtr, int* BcolIdx, double* B,
                            int An,int Am,int Ann,int Amm,
                            int** ArowPtr, int** AcolIdx, double** A) {

  auto mat = csr2eigen(Bn, Bm, BrowPtr, BcolIdx, B);
  mat = -mat;
  mat.makeCompressed();

  auto nnz = mat.nonZeros();
  *ArowPtr = static_cast<int*>(ffi::simit_malloc((An+1) * sizeof(int)));
  *AcolIdx = static_cast<int*>(ffi::simit_malloc(   nnz * sizeof(int)));
  *A    = static_cast<double*>(ffi::simit_malloc(   nnz * sizeof(double)));
}


TEST(DISABLED_ffi, matrix_neg) {
  Set V;
  FieldRef<simit_float> a = V.addField<simit_float>("a");
  FieldRef<simit_float> b = V.addField<simit_float>("b");
  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  ElementRef v2 = V.add();
  b(v0) = 1.0;
  b(v1) = 2.0;
  b(v2) = 3.0;

  Set E(V,V);
  FieldRef<simit_float> e = E.addField<simit_float>("e");
  ElementRef e0 = E.add(v0,v1);
  ElementRef e1 = E.add(v1,v2);
  e(e0) = 1.0;
  e(e1) = 2.0;

  // Compile program and bind arguments
  Function func = loadFunction(TEST_FILE_NAME, "main");
  if (!func.defined()) FAIL();
  func.bind("V", &V);
  func.bind("E", &E);
  func.runSafe();

  // Check that inputs are preserved
  ASSERT_EQ(1.0, b(v0));
  ASSERT_EQ(2.0, b(v1));
  ASSERT_EQ(3.0, b(v2));

  // Check that outputs are correct
  ASSERT_EQ(-3.0, a(v0));
  ASSERT_EQ(-11.0, a(v1));
  ASSERT_EQ(-10.0, a(v2));
}

#endif

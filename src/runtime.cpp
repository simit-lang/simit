#include "runtime.h"

#include <cmath>
#include <time.h>
#include <chrono>
#include <vector>

#include "timers.h"
#include "stdio.h"

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
using namespace Eigen;
#endif

extern "C" {
int loc(int v0, int v1, int *neighbors_start, int *neighbors) {
  int l = neighbors_start[v0];
  while(neighbors[l] != v1) l++;
  return l;
}

double atan2_f64(double y, double x) {
  return atan2(y, x);
}

float atan2_f32(float y, float x) {
  double d_y = y;
  double d_x = x;
  return (float)atan2(d_y, d_x);
}

double tan_f64(double x) {
  return tan(x);
}

float tan_f32(float x) {
  double d_x = x;
  return (float)tan(d_x);
}

double asin_f64(double x) {
  return asin(x);
}

float asin_f32(float x) {
  double d_x = x;
  return (float)asin(d_x);
}

double acos_f64(double x) {
  return acos(x);
}

float acos_f32(float x) {
  double d_x = x;
  return (float)acos(d_x);
}

double max_f64(double a,double b) {
  return max(a,b);
}

float max_f32(float a, float b) {
  double d_a = a;
  double d_b = b;
  return (float)max(d_a,d_b);
}

double min_f64(double a,double b) {
  return min(a,b);
}

float min_f32(float a, float b) {
  double d_a = a;
  double d_b = b;
  return (float)min(d_a,d_b);
}

double det3_f64(double * a){
  return a[0] * (a[4]*a[8]-a[5]*a[7])
       - a[1] * (a[3]*a[8]-a[5]*a[6])
       + a[2] * (a[3]*a[7]-a[4]*a[6]);
}

float det3_f32(float * a){
  return a[0] * (a[4]*a[8]-a[5]*a[7])
       - a[1] * (a[3]*a[8]-a[5]*a[6])
       + a[2] * (a[3]*a[7]-a[4]*a[6]);
}

void inv3_f64(double * a, double * inv){
  double cof00 = a[4]*a[8]-a[5]*a[7];
  double cof01 =-a[3]*a[8]+a[5]*a[6];
  double cof02 = a[3]*a[7]-a[4]*a[6];

  double cof10 =-a[1]*a[8]+a[2]*a[7];
  double cof11 = a[0]*a[8]-a[2]*a[6];
  double cof12 =-a[0]*a[7]+a[1]*a[6];

  double cof20 = a[1]*a[5]-a[2]*a[4];
  double cof21 =-a[0]*a[5]+a[2]*a[3];
  double cof22 = a[0]*a[4]-a[1]*a[3];

  double determ = a[0] * cof00 + a[1] * cof01 + a[2]*cof02;

  determ = 1.0/determ;
  inv[0] = cof00 * determ;
  inv[1] = cof10 * determ;
  inv[2] = cof20 * determ;

  inv[3] = cof01 * determ;
  inv[4] = cof11 * determ;
  inv[5] = cof21 * determ;

  inv[6] = cof02 * determ;
  inv[7] = cof12 * determ;
  inv[8] = cof22 * determ;
}

void inv3_f32(float * a, float * inv){
  float cof00 = a[4]*a[8]-a[5]*a[7];
  float cof01 =-a[3]*a[8]+a[5]*a[6];
  float cof02 = a[3]*a[7]-a[4]*a[6];

  float cof10 =-a[1]*a[8]+a[2]*a[7];
  float cof11 = a[0]*a[8]-a[2]*a[6];
  float cof12 =-a[0]*a[7]+a[1]*a[6];

  float cof20 = a[1]*a[5]-a[2]*a[4];
  float cof21 =-a[0]*a[5]+a[2]*a[3];
  float cof22 = a[0]*a[4]-a[1]*a[3];

  float determ = a[0] * cof00 + a[1] * cof01 + a[2]*cof02;

  determ = 1.0/determ;
  inv[0] = cof00 * determ;
  inv[1] = cof10 * determ;
  inv[2] = cof20 * determ;

  inv[3] = cof01 * determ;
  inv[4] = cof11 * determ;
  inv[5] = cof21 * determ;

  inv[6] = cof02 * determ;
  inv[7] = cof12 * determ;
  inv[8] = cof22 * determ;
}

double complexNorm_f64(double r, double i) {
  return sqrt(r*r+i*i);
}

float complexNorm_f32(float r, float i) {
  return sqrt(r*r+i*i);
}

void simitStoreTime(int i, double value) {
  simit::ir::TimerStorage::getInstance().storeTime(i, value);
}

double simitClock() {
  using namespace std::chrono;
  auto t = high_resolution_clock::now();
  time_point<high_resolution_clock,microseconds> usec = time_point_cast<microseconds>(t);
  return (double)(usec.time_since_epoch().count());
}
} // extern "C"


/// Temporary external spmm implementation until Simit supports assembling
/// matrix indices during computation.
template <typename Float>
int spmm(int Bn,  int Bm,  int* Browptr, int* Bcolidx,
         int Bnn, int Bmm, Float* Bvals,
         int Cn,  int Cm,  int* Crowptr, int* Ccolidx,
         int Cnn, int Cmm, Float* Cvals,
         int An,  int Am,  int** Arowptr, int** Acolidx,
         int Ann, int Amm, Float** Avals) {
#ifdef EIGEN
  auto B = csr2eigen<Float,RowMajor>(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals);
  auto C = csr2eigen<Float,RowMajor>(Cn, Cm, Crowptr, Ccolidx, Cnn, Cmm, Cvals);

  SparseMatrix<Float,RowMajor> A(An, Am);
  A = B*C;
  eigen2csr(A, An, Am, Arowptr, Acolidx, Ann, Amm, Avals);
#else
  ierror << "extern spmm requires Eigen";
#endif
  return 0;
}
extern "C" int sspmm(int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                     int Bnn, int Bmm, float* Bvals,
                     int Cn,  int Cm,  int* Crowptr, int* Ccolidx,
                     int Cnn, int Cmm, float* Cvals,
                     int An,  int Am,  int** Arowptr, int** Acolidx,
                     int Ann, int Amm, float** Avals) {
  return spmm(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals,
              Cn, Cm, Crowptr, Ccolidx, Cnn, Cmm, Cvals,
              An, Am, Arowptr, Acolidx, Ann, Amm, Avals);
}
extern "C" int dspmm(int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                     int Bnn, int Bmm, double* Bvals,
                     int Cn,  int Cm,  int* Crowptr, int* Ccolidx,
                     int Cnn, int Cmm, double* Cvals,
                     int An,  int Am,  int** Arowptr, int** Acolidx,
                     int Ann, int Amm, double** Avals) {
  return spmm(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals,
              Cn, Cm, Crowptr, Ccolidx, Cnn, Cmm, Cvals,
              An, Am, Arowptr, Acolidx, Ann, Amm, Avals);
}


// Solvers
#define SOLVER_ERROR                                            \
do {                                                            \
  ierror << "Solvers require that Simit was built with Eigen."; \
} while (false)

template <typename Float>
void solve(int n,  int m,  int* rowptr, int* colidx,
           int nn, int mm, Float* Avals, Float* bvals, Float* xvals) {
#ifdef EIGEN
  auto A = csr2eigen<Float,ColMajor>(n, m, rowptr, colidx, nn, mm, Avals);
  auto x = new Map<Matrix<Float,Dynamic,1>>(xvals, m);
  auto b = new Map<Matrix<Float,Dynamic,1>>(bvals, n);

  SparseLU<SparseMatrix<Float, ColMajor>> solver;
  solver.compute(A);
  *x = solver.solve(*b);
#else
  SOLVER_ERROR;
#endif
}
extern "C" void cMatSolve_f64(int n,  int m,  int* rowptr, int* colidx,
                              int nn, int mm, double* A, double* x, double* b) {
  return solve(n, m, rowptr, colidx, nn, mm, A, x, b);
}
extern "C" void cMatSolve_f32(int n,  int m,  int* rowptr, int* colidx,
                              int nn, int mm, float* A, float* x, float* b) {
  return solve(n, m, rowptr, colidx, nn, mm, A, x, b);
}

/// LU factorization. Returns a solver object that can be used with
/// `lusolve` and `lumatsolve`. The solver object must be freed using
/// `lufree`.
template <typename Float>
int lu(int An,  int Am,  int* Arowptr, int* Acolidx,
       int Ann, int Amm, Float* Avals,
       void** solverPtr) {
#ifdef EIGEN
  auto A = csr2eigen<Float,Eigen::ColMajor>(An, Am, Arowptr, Acolidx,
                                            Ann, Amm, Avals);
  auto solver = new SparseLU<SparseMatrix<Float,ColMajor>>();
  solver->compute(A);
  *solverPtr = static_cast<void*>(solver);
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" int slu(int An,  int Am,  int* Arowptr, int* Acolidx,
                   int Ann, int Amm, float* Avals,
                   void** solver) {
  return lu(An, Am, Arowptr, Acolidx, Ann, Amm, Avals, solver);
}
extern "C" int dlu(int An,  int Am,  int* Arowptr, int* Acolidx,
                   int Ann, int Amm, double* Avals,
                   void** solver) {
  return lu(An, Am, Arowptr, Acolidx, Ann, Amm, Avals, solver);
}


/// Free an LU solver.
template <typename Float>
int lufree(void** solverPtr) {
#ifdef EIGEN
  auto solver=static_cast<SparseLU<SparseMatrix<Float,ColMajor>>*>(*solverPtr);
  delete solver;
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" int slufree(void** solverPtr) {
  return lufree<float>(solverPtr);
}
extern "C" int dlufree(void** solverPtr){
  return lufree<double>(solverPtr);
}

/// Solve `t=L^{-1}*b` and `x=L'^{-1}*t`, where `A=LL'` is the matrix that was
/// factorized with the provided solver using `chol`.
template <typename Float>
int lusolve(void** solverPtr, int nb, Float *bvals, int nx, Float *xvals) {
#ifdef EIGEN
  auto solver=static_cast<SparseLU<SparseMatrix<Float,ColMajor>>*>(*solverPtr);
  auto b = dense2eigen(nb, bvals);
  auto x = Eigen::Matrix<Float,Eigen::Dynamic,1>(nx);
  x = solver->solve(b);
  for (int i=0; i<nx; ++i) {
    xvals[i] = x(i);
  }
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" {
  int slusolve(void** solverPtr, int bn, float *bvals, int xn, float *xvals) {
    return lusolve(solverPtr, bn, bvals, xn, xvals);
  }
  int dlusolve(void** solverPtr, int bn, double *bvals, int xn, double *xvals){
    return lusolve(solverPtr, bn, bvals, xn, xvals);
  }
}

/// Solve `T=L^{-1}*B` and `X=L'^{-1}*T`, where `A=LL'` is the matrix that was
/// factorized with the provided solver using `chol`.
template <typename Float>
int lumatsolve(void** solverPtr,
                int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                int Bnn, int Bmm, Float* Bvals,
                int Xn,  int Xm,  int** Xrowptr, int** Xcolidx,
                int Xnn, int Xmm, Float** Xvals){
#ifdef EIGEN
  auto solver=static_cast<SparseLU<SparseMatrix<Float,ColMajor>>*>(*solverPtr);
  auto B = csr2eigen<Float,ColMajor>(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals);
  SparseMatrix<Float> X(Xn, Xm);
  X = solver->solve(B);
  X = X.transpose();
  eigen2csr<Float>(X, Xn, Xm, Xrowptr, Xcolidx, Xnn, Xmm, Xvals);
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" int slumatsolve(void** solverPtr,
                            int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                            int Bnn, int Bmm, float* Bvals,
                            int Xn,  int Xm,  int** Xrowptr, int** Xcolidx,
                            int Xnn, int Xmm, float** Xvals) {
  return lumatsolve(solverPtr,
                     Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals,
                     Xn, Xm, Xrowptr, Xcolidx, Xnn, Xmm, Xvals);
}
extern "C" int dlumatsolve(void** solverPtr,
                            int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                            int Bnn, int Bmm, double* Bvals,
                            int Xn,  int Xm,  int** Xrowptr, int** Xcolidx,
                            int Xnn, int Xmm, double** Xvals) {
  return lumatsolve(solverPtr,
                     Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals,
                     Xn, Xm, Xrowptr, Xcolidx, Xnn, Xmm, Xvals);
}


/// Cholesky factorization. Returns a solver object that can be used with
/// `lltsolve` and `lltmatsolve`. The solver object must be freed using
/// `cholfree`.
template <typename Float>
int chol(int An,  int Am,  int* Arowptr, int* Acolidx,
         int Ann, int Amm, Float* Avals,
         void** solverPtr) {
#ifdef EIGEN
  auto A = csr2eigen<Float,Eigen::ColMajor>(An, Am, Arowptr, Acolidx,
                                            Ann, Amm, Avals);
  auto solver = new SimplicialCholesky<SparseMatrix<Float>>();
  solver->compute(A);
  *solverPtr = static_cast<void*>(solver);
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" int schol(int An,  int Am,  int* Arowptr, int* Acolidx,
                     int Ann, int Amm, float* Avals,
                     void** solver) {
  return chol(An, Am, Arowptr, Acolidx, Ann, Amm, Avals, solver);
}
extern "C" int dchol(int An,  int Am,  int* Arowptr, int* Acolidx,
                     int Ann, int Amm, double* Avals,
                     void** solver) {
  return chol(An, Am, Arowptr, Acolidx, Ann, Amm, Avals, solver);
}

/// Free a Cholesky solver.
template <typename Float>
int cholfree(void** solverPtr) {
#ifdef EIGEN
  auto solver=static_cast<SimplicialCholesky<SparseMatrix<Float>>*>(*solverPtr);
  delete solver;
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" int scholfree(void** solverPtr) {
  return cholfree<float>(solverPtr);
}
extern "C" int dcholfree(void** solverPtr){
  return cholfree<double>(solverPtr);
}

/// Solve `t=L^{-1}*b` and `x=L'^{-1}*t`, where `A=LL'` is the matrix that was
/// factorized with the provided solver using `chol`.
template <typename Float>
int lltsolve(void** solverPtr, int nb, Float *bvals, int nx, Float *xvals) {
#ifdef EIGEN
  auto solver=static_cast<SimplicialCholesky<SparseMatrix<Float>>*>(*solverPtr);
  auto b = dense2eigen(nb, bvals);
  auto x = Eigen::Matrix<Float,Eigen::Dynamic,1>(nx);
  x = solver->solve(b);
  for (int i=0; i<nx; ++i) {
    xvals[i] = x(i);
  }
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" {
int slltsolve(void** solverPtr, int bn, float *bvals, int xn, float *xvals) {
  return lltsolve(solverPtr, bn, bvals, xn, xvals);
}
int dlltsolve(void** solverPtr, int bn, double *bvals, int xn, double *xvals){
  return lltsolve(solverPtr, bn, bvals, xn, xvals);
}
}

/// Solve `T=L^{-1}*B` and `X=L'^{-1}*T`, where `A=LL'` is the matrix that was
/// factorized with the provided solver using `chol`.
template <typename Float>
int lltmatsolve(void** solverPtr,
                 int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                 int Bnn, int Bmm, Float* Bvals,
                 int Xn,  int Xm,  int** Xrowptr, int** Xcolidx,
                 int Xnn, int Xmm, Float** Xvals){
#ifdef EIGEN
  auto solver=static_cast<SimplicialCholesky<SparseMatrix<Float,ColMajor>>*>(*solverPtr);
  auto B = csr2eigen<Float,ColMajor>(Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals);
  SparseMatrix<Float> X(Xn, Xm);
  X = solver->solve(B);
  X = X.transpose();
  eigen2csr<Float>(X, Xn, Xm, Xrowptr, Xcolidx, Xnn, Xmm, Xvals);
#else
  SOLVER_ERROR;
#endif
  return 0;
}
extern "C" int slltmatsolve(void** solverPtr,
                            int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                            int Bnn, int Bmm, float* Bvals,
                            int Xn,  int Xm,  int** Xrowptr, int** Xcolidx,
                            int Xnn, int Xmm, float** Xvals) {
  return lltmatsolve(solverPtr,
                      Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals,
                      Xn, Xm, Xrowptr, Xcolidx, Xnn, Xmm, Xvals);
}
extern "C" int dlltmatsolve(void** solverPtr,
                            int Bn,  int Bm,  int* Browptr, int* Bcolidx,
                            int Bnn, int Bmm, double* Bvals,
                            int Xn,  int Xm,  int** Xrowptr, int** Xcolidx,
                            int Xnn, int Xmm, double** Xvals) {
  return lltmatsolve(solverPtr,
                      Bn, Bm, Browptr, Bcolidx, Bnn, Bmm, Bvals,
                      Xn, Xm, Xrowptr, Xcolidx, Xnn, Xmm, Xvals);
}

/// cross product between 2 vectors3D
template <typename Float>
void cross(int an, Float* a, int bn, Float* b, int cn, Float* c){
  assert(an==3 && bn==3);
  c[0] = a[1]*b[2]-a[2]*b[1];
  c[1] = a[2]*b[0]-a[0]*b[2];
  c[2] = a[0]*b[1]-a[1]*b[0];
}
extern "C" void scross(int an, float* a, int bn, float* b, int cn, float* c) {
  return cross(an, a, bn, b, cn, c);
}
extern "C" void dcross(int an, double* a, int bn, double* b, int cn, double* c) {
  return cross(an, a, bn, b, cn, c);
}



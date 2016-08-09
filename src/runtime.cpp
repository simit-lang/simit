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

// Solvers
#ifdef EIGEN
using namespace Eigen;
#endif

#define SOLVER_ERROR                                            \
do {                                                            \
  ierror << "Solvers require that Simit was built with Eigen."; \
} while (false)

template <typename Float>
void solve(int n,  int m,  int* rowptr, int* colidx,
           int nn, int mm, Float* Avals, Float* xvals, Float* bvals) {
#ifdef EIGEN
  auto A = csr2eigen<Float, Eigen::ColMajor>(n, m, rowptr, colidx, nn, mm, Avals);
  auto b = new Map<Matrix<Float,Dynamic,1>>(bvals, n);
  auto x = new Map<Matrix<Float,Dynamic,1>>(xvals, m);

  ConjugateGradient<SparseMatrix<Float>,Lower,IdentityPreconditioner> solver;
  solver.setMaxIterations(50);
  solver.compute(A);
  *b = solver.solve(*x);
#else
  SOLVER_ERROR;
#endif
}

extern "C" {
void cMatSolve_f64(int n,  int m,  int* rowptr, int* colidx,
                   int nn, int mm, double* A, double* x, double* b) {
  return solve(n, m, rowptr, colidx, nn, mm, A, x, b);
}
void cMatSolve_f32(int n,  int m,  int* rowptr, int* colidx,
                   int nn, int mm, float* A, float* x, float* b) {
  return solve(n, m, rowptr, colidx, nn, mm, A, x, b);
}
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
extern "C" {
int schol(int An,  int Am,  int* Arowptr, int* Acolidx,
          int Ann, int Amm, float* Avals,
          void** solver) {
  return chol(An, Am, Arowptr, Acolidx, Ann, Amm, Avals, solver);
}
int dchol(int An,  int Am,  int* Arowptr, int* Acolidx,
          int Ann, int Amm, double* Avals,
          void** solver) {
  return chol(An, Am, Arowptr, Acolidx, Ann, Amm, Avals, solver);
}
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
extern "C" {
int scholfree(void** solverPtr) {
  return cholfree<float>(solverPtr);
}
int dcholfree(void** solverPtr){
  return cholfree<double>(solverPtr);
}
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

#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#include <time.h>
#include <vector>

extern "C" {

int loc(int v0, int v1, int *neighbors_start, int *neighbors);

double atan2_f64(double y, double x);
float  atan2_f32(float y, float x);

double tan_f64(double x);
float  tan_f32(float x);

double asin_f64(double x);
float  asin_f32(float x);

double acos_f64(double x);
float  acos_f32(float x);

double complexNorm_f64(double r, double i);
float  complexNorm_f32(float r, float i);

void simitStoreTime(int i, double value);
double simitClock();

double det3_f64(double* a);
float  det3_f32(float* a);

void inv3_f64(double* a, double* inv);
void inv3_f32(float* a, float* inv);

void cMatSolve_f64(int n,  int m,  int* rowPtr, int* colIdx,
                   int nn, int mm, double* A,
                   double* x, double* b);

void cMatSolve_f32(int n,  int m,  int* rowPtr, int* colIdx,
                   int nn, int mm, float* A,
                   float* x, float* b);

}

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>

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
#endif

#endif

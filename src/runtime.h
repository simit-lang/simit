#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>

extern "C" {
double cMatSolve(double* bufferA, double* bufferX, double* bufferC,
                 int rows, int columns) {
  using namespace Eigen;
  auto Amat = new Map<Matrix<double,Dynamic,Dynamic,RowMajor>>(bufferA, rows,
                                                               columns);
  auto xvec = new Eigen::Map<Eigen::Matrix<double,Dynamic,1>>(bufferX, rows);
  auto cvec = new Eigen::Map<Eigen::Matrix<double,Dynamic,1>>(bufferC, rows);

  // TODO: this may be overkill
  // we can probably get away with LLT or LDLT instead
  *cvec = Amat->colPivHouseholderQr().solve(*xvec);

  return 1.0;
}
}
#endif

extern "C" {

int loc(int v0, int v1, int *neighbors_start, int *neighbors) {
  int l = neighbors_start[v0];
  while(neighbors[l] != v1) l++;
  return l;
}

// dot product
double dot(double* a, double* b, int len) {
  double result = 0.0;
  for (int i=0; i<len; i++)
    result += a[i] * b[i];
  
  return result;
}

}
#endif

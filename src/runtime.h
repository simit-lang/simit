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

int loc(int v0, int v1) {
  printf("(%d, %d)", v0, v1);
//  int l = neighbors_summary[v0];
//  while(neighbors[l] != v1) l++;
//  return l;
  return v0+v1;
}

}
#endif

#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>

extern "C" {
double cMatSolve(double* bufferA, double* bufferX,
  double* bufferC, int rows, int columns) {
  auto Amat = new Eigen::Map<Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic,
                                      Eigen::RowMajor>>(bufferA, rows, columns);

  auto xvec = new Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, 1>>(bufferX,
    rows);

  auto cvec = new Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, 1>>(bufferC,
    rows);

  // TODO: this may be overkill
  // we can probably get away with LLT or LDLT instead
  *cvec = Amat->colPivHouseholderQr().solve(*xvec);
  
  return 1.0;
}
}
#endif

extern "C" {

}
#endif

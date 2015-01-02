#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>

extern "C" {
double cMatSolve_f64(double* bufferA, double* bufferX, double* bufferC,
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

float cMatSolve_f32(float* bufferA, float* bufferX, float* bufferC,
                int rows, int columns) {
  using namespace Eigen;
  auto Amat = new Map<Matrix<float,Dynamic,Dynamic,RowMajor>>(bufferA, rows,
                                                              columns);
  auto xvec = new Eigen::Map<Eigen::Matrix<float,Dynamic,1>>(bufferX, rows);
  auto cvec = new Eigen::Map<Eigen::Matrix<float,Dynamic,1>>(bufferC, rows);

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
double dot_f64(double* a, double* b, int len) {
  double result = 0.0;
  for (int i=0; i<len; i++)
    result += a[i] * b[i];
  
  return result;
}

float dot_f32(float* a, float* b, int len) {
  float result = 0.0;
  for (int i=0; i<len; i++)
    result += a[i] * b[i];
  
  return result;
}

// norm
double norm_f64(double* a, int len) {
  return sqrt(dot_f64(a, a, len));
}

float norm_f32(float* a, int len) {
  return sqrt(dot_f32(a, a, len));
}

// atan2 wrapper
double atan2_f64(double y, double x) {
  return atan2(y, x);
}
float atan2_f32(float y, float x) {
  double d_y = y;
  double d_x = x;
  return (float)atan2(d_y, d_x);
}

// tan wrapper
double tan_f64(double x) {
  return tan(x);
}
float tan_f32(float x) {
  double d_x = x;
  return (float)tan(d_x);
}

// asin wrapper
double asin_f64(double x) {
  return asin(x);
}
float asin_f32(float x) {
  double d_x = x;
  return (float)asin(d_x);
}

// acos wrapper
double acos_f64(double x) {
  return acos(x);
}
float acos_f32(float x) {
  double d_x = x;
  return (float)acos(d_x);
}

}
#endif

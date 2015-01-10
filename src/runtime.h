#ifndef SIMIT_RUNTIME_H
#define SIMIT_RUNTIME_H

#ifdef EIGEN
#include <Eigen/Core>
#include <Eigen/Dense>

extern "C" {
// appease GCC
void cMatSolve_f64(double* bufferA, double* bufferX, double* bufferC,
                   int rows, int columns);
void cMatSolve_f32(float* bufferA, float* bufferX, float* bufferC,
                   int rows, int columns);
int loc(int v0, int v1, int *neighbors_start, int *neighbors);
double dot_f64(double* a, double* b, int len);
float dot_f32(float* a, float* b, int len);
double norm_f64(double* a, int len);
float norm_f32(float* a, int len);
double atan2_f64(double y, double x);
float atan2_f32(float y, float x);
double tan_f64(double x);
float tan_f32(float x);
double asin_f64(double x);
float asin_f32(float x);
double acos_f64(double x);
float acos_f32(float x);
double det3_f64(double* a);
float det3_f32(float* a);
void inv3_f64(double* a, double* inv);
void inv3_f32(float* a, float* inv);

void cMatSolve_f64(double* bufferA, double* bufferX, double* bufferC,
                 int rows, int columns) {
  using namespace Eigen;
  auto Amat = new Map<Matrix<double,Dynamic,Dynamic,RowMajor>>(bufferA, rows,
                                                               columns);
  auto xvec = new Eigen::Map<Eigen::Matrix<double,Dynamic,1>>(bufferX, rows);
  auto cvec = new Eigen::Map<Eigen::Matrix<double,Dynamic,1>>(bufferC, rows);

  // TODO: this may be overkill
  // we can probably get away with LLT or LDLT instead
  *cvec = Amat->colPivHouseholderQr().solve(*xvec);
}

void cMatSolve_f32(float* bufferA, float* bufferX, float* bufferC,
                int rows, int columns) {
  using namespace Eigen;
  auto Amat = new Map<Matrix<float,Dynamic,Dynamic,RowMajor>>(bufferA, rows,
                                                              columns);
  auto xvec = new Eigen::Map<Eigen::Matrix<float,Dynamic,1>>(bufferX, rows);
  auto cvec = new Eigen::Map<Eigen::Matrix<float,Dynamic,1>>(bufferC, rows);

  // TODO: this may be overkill
  // we can probably get away with LLT or LDLT instead
  *cvec = Amat->colPivHouseholderQr().solve(*xvec);
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

//  0 1 2
//  3 4 5
//  6 7 8
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
}
#endif

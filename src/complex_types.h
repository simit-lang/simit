#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

namespace simit {

struct float_complex {
  float real;
  float imag;
  float_complex() : real(0), imag(0) {}
  float_complex(float r, float i) : real(r), imag(i) {}
};
  
struct double_complex {
  double real;
  double imag;
  double_complex() : real(0), imag(0) {}
  double_complex(double r, double i) : real(r), imag(i) {}
  double_complex(float_complex other) : real(other.real), imag(other.imag) {}
};

}

#endif

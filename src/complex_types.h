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

  friend inline bool operator==(const double_complex& c1,
                                const double_complex& c2) {
    return c1.real == c2.real && c1.imag == c2.imag;
  }
  
  friend inline bool operator!=(const double_complex& c1,
                                const double_complex& c2) {
    return !(c1 == c2);
  }
};

}

#endif

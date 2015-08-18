#ifndef SIMIT_COMPARE_H
#define SIMIT_COMPARE_H

#include <cstddef>

namespace simit {
namespace util {

bool almostEqual(double a, double b, double maxRelativeError=0.0001);
bool almostEqual(float a, float b, float maxRelativeError=0.0001);

//template <typename T>
//bool compare

/// Compares two arrays and returns true if the content is equal.
template <typename T> inline
bool compareEq(const T* ldata, const T* rdata, size_t num) {
  for (size_t i=0; i < num; ++i) {
    if (ldata[i] != rdata[i]) {
      return false;
    }
  }
  return true;
}
template <> inline
bool compareEq<float>(const float* ldata, const float* rdata, size_t num) {
  for (size_t i=0; i < num; ++i) {
    if (!util::almostEqual(ldata[i], rdata[i], 0.001f)) {
      return false;
    }
  }
  return true;
}
template <> inline
bool compareEq<double>(const double* ldata, const double* rdata, size_t num) {
  for (size_t i=0; i < num; ++i) {
    if (!util::almostEqual(ldata[i], rdata[i], 0.001)) {
      return false;
    }
  }
  return true;
}

template <typename T> inline
bool compareEq(const void* ldata, const void* rdata, size_t num) {
  return compareEq(static_cast<const T*>(ldata),
                   static_cast<const T*>(rdata), num);
}

/// Compares two arrays and returns true if the content is not equal.
template <typename T> inline
bool compareNe(const T* ldata, const T* rdata, size_t num) {
  for (size_t i=0; i < num; ++i) {
    if (ldata[i] != rdata[i]) {
      return false;
    }
  }
  return true;
}

template <typename T> inline
bool compareNe(const void* ldata, const void* rdata, size_t num) {
  return compareNe(static_cast<const T*>(ldata),
                   static_cast<const T*>(rdata), num);
}


}}
#endif

#include "compare.h"

#include <cmath>

namespace simit {
namespace util {

bool almostEqual(double a, double b, double maxRelativeError) {
  if (a == b) {
    return true;
  }
  double relativeError;
  if (fabs(b) > fabs(a)) {
    relativeError = fabs((a - b) / b);
  }
  else {
    relativeError = fabs((a - b) / a);
  }
  if (relativeError <= maxRelativeError) {
    return true;
  }
  return false;
}

bool almostEqual(float a, float b, float maxRelativeError) {
  if (a == b) {
    return true;
  }
  double relativeError;
  if (fabs(b) > fabs(a)) {
    relativeError = fabs((a - b) / b);
  }
  else {
    relativeError = fabs((a - b) / a);
  }
  if (relativeError <= maxRelativeError) {
    return true;
  }
  return false;
}

}}

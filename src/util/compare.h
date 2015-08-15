#ifndef SIMIT_COMPARE_H
#define SIMIT_COMPARE_H

namespace simit {
namespace util {

bool almostEqual(double a, double b, double maxRelativeError=0.0001);
bool almostEqual(float a, float b, float maxRelativeError=0.0001);

}}
#endif

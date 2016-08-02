#ifndef SIMIT_INTRINSICS_H
#define SIMIT_INTRINSICS_H

#include <string>
#include <map>

namespace simit {
namespace ir {
class Func;

/// Intrinsic functions
/// TODO: Factor intrinsics into standard libraries that must be imported
namespace intrinsics {

// Scalar math
const Func& mod();
const Func& sin();
const Func& cos();
const Func& tan();
const Func& asin();
const Func& acos();
const Func& atan2();
const Func& sqrt();
const Func& log();
const Func& exp();
const Func& pow();
//const Func& ceil();  // TODO: Add
//const Func& floor(); // TODO: Add

// Complex numbers
const Func& createComplex();
const Func& complexNorm();
const Func& complexConj();
const Func& complexGetReal();
const Func& complexGetImag();

// Vector/Matrix math
const Func& norm();
const Func& dot();
const Func& det();

// Solvers
const Func& inv();
const Func& solve();

// String manipulation
const Func& strcmp();
const Func& strlen();
const Func& strcpy();
const Func& strcat();

// Clock
const Func& clock();
const Func& storeTime();

// Internal functions
const Func& malloc();
const Func& free();
const Func& loc();

const Func& byName(const std::string& name);
const std::map<std::string,Func> &byNames();

}}}
#endif

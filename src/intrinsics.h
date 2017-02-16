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
const Func& cbrt();
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
const Func& det2();
const Func& det4();
const Func& inv();
const Func& inv2();
const Func& inv4();

// Vector/Vector math
const Func& cross();

// Solvers
const Func& solve();
const Func& lu();
const Func& lufree();
const Func& lusolve();
const Func& lumatsolve();
const Func& chol();
const Func& cholfree();
const Func& lltsolve();
const Func& lltmatsolve();

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

const std::map<std::string,Func> &byNames();

}}}
#endif

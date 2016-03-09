#ifndef SIMIT_INTRINSICS_H
#define SIMIT_INTRINSICS_H

#include <string>
#include <map>

namespace simit {
namespace ir {
class Func;

/// Intrinsic functions
namespace intrinsics {

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
const Func& norm();
const Func& dot();
const Func& det();
const Func& inv();
const Func& solve();
const Func& loc();
const Func& free();
const Func& malloc();
const Func& strcmp();
const Func& strlen();
const Func& strcpy();
const Func& strcat();
const Func& simitClock();
const Func& simitStoreTime();

const Func& byName(const std::string& name);
const std::map<std::string,Func> &byNames();

}}}
#endif

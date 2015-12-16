#include "intrinsics.h"

#include <cassert>
#include "var.h"
#include "func.h"

namespace simit {
namespace ir {
namespace intrinsics {

static Func modVar;
void modInit() {
  modVar = Func("mod",
                {Var("x", Int), Var("y", Int)},
                {Var("r", Int)},
                Func::Intrinsic);
}

static Func sinVar;
void sinInit() {
  sinVar = Func("sin",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func cosVar;
void cosInit() {
  cosVar = Func("cos",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func tanVar;
void tanInit() {
  tanVar = Func("tan",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func asinVar;
void asinInit() {
  asinVar = Func("asin",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}

static Func acosVar;
void acosInit() {
  acosVar = Func("acos",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}

static Func atan2Var;
void atan2Init() {
  atan2Var = Func("atan2",
                  {Var("y", Float), Var("x", Float)},
                  {Var("r", Float)},
                  Func::Intrinsic);
}

static Func sqrtVar;
void sqrtInit() {
  sqrtVar = Func("sqrt",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}

static Func logVar;
void logInit() {
  logVar = Func("log",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func expVar;
void expInit() {
  expVar = Func("exp",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func powVar;
void powInit() {
  powVar = Func("pow",
                {Var("x", Float), Var("y", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func normVar;
void normInit() {
  normVar = Func("norm",
                 {},
                 {Var("r", Float)},
                 Func::Intrinsic);
}

static Func dotVar;
void dotInit() {
  dotVar = Func("dot",
                {},
                {Var("r", Float)},
                Func::Intrinsic);
}


static Func detVar;
void detInit() {
  detVar = Func("det",
                {},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func invVar;
void invInit() {
  invVar = Func("inv",
                {},
                {Var("r", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                Func::Intrinsic);
}

static Func solveVar;
void solveInit() {
  solveVar = Func("solve",
                  {},
                  {Var("r", Float)},
                  Func::Intrinsic);
}

static Func locVar;
void locInit() {
  locVar = Func("loc",
                {},
                {Var("r", Int)},
                Func::Intrinsic);
}

static Func simitClockVar;
void simitClockInit() {
  simitClockVar = Func("simitClock",
                {},
                {Var("r", Float)},
                Func::Intrinsic);
}

static Func simitStoreTimeVar;
void simitStoreTimeInit() {
  simitStoreTimeVar = Func("simitStoreTime",
                {Var("i", Int), Var("val", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}

// We lazily initialize all the intrinsics. No need to call all the constructors
// unless we will use them.

const Func& mod() {
  if (!modVar.defined()) {
    modInit();
  }
  return modVar;
}

const Func& sin() {
  if (!sinVar.defined()) {
    sinInit();
  }
  return sinVar;
}

const Func& cos() {
  if (!cosVar.defined()) {
    cosInit();
  }
  return cosVar;
}

const Func& tan() {
  if (!tanVar.defined()) {
    tanInit();
  }
  return tanVar;
}

const Func& asin() {
  if (!asinVar.defined()) {
    asinInit();
  }
  return asinVar;
}

const Func& acos() {
  if (!acosVar.defined()) {
    acosInit();
  }
  return acosVar;
}

const Func& atan2() {
  if (!atan2Var.defined()) {
    atan2Init();
  }
  return atan2Var;
}
const Func& sqrt() {
  if (!sqrtVar.defined()) {
    sqrtInit();
  }
  return sqrtVar;
}

const Func& log() {
  if (!logVar.defined()) {
    logInit();
  }
  return logVar;
}

const Func& exp() {
  if (!expVar.defined()) {
    expInit();
  }
  return expVar;
}


const Func& pow() {
  if (!powVar.defined()) {
    powInit();
  }
  return powVar;
}

const Func& norm() {
  if (!normVar.defined()) {
    normInit();
  }
  return normVar;
}

const Func& dot() {
  if (!dotVar.defined()) {
    dotInit();
  }
  return dotVar;
}

const Func& det() {
  if (!detVar.defined()) {
    detInit();
  }
  return detVar;
}

const Func& inv() {
  if (!invVar.defined()) {
    invInit();
  }
  return invVar;
}

const Func& solve() {
  if (!solveVar.defined()) {
    solveInit();
  }
  return solveVar;
}

const Func& loc() {
  if (!locVar.defined()) {
    locInit();
  }
  return locVar;
}

const Func& simitClock() {
  if (!simitClockVar.defined()) {
    simitClockInit();
  }
  return simitClockVar;
}

const Func& simitStoreTime() {
  if (!simitStoreTimeVar.defined()) {
    simitStoreTimeInit();
  }
  return simitStoreTimeVar;
}


const std::map<std::string,Func> &byNames() {
  static std::map<std::string,Func> byNameMap;
  if (byNameMap.size() == 0) {
    modInit();
    sinInit();
    cosInit();
    tanInit();
    asinInit();
    acosInit();
    atan2Init();
    sqrtInit();
    logInit();
    expInit();
    powInit();
    normInit();
    dotInit();
    detInit();
    invInit();
    solveInit();
    simitClockInit();
    simitStoreTimeInit();
    byNameMap.insert({{"mod",modVar},
                      {"sin",sinVar},
                      {"cos",cosVar},
                      {"tan",tanVar},
                      {"asin",asinVar},
                      {"acos",acosVar},
                      {"atan2",atan2Var},
                      {"sqrt",sqrtVar},
                      {"log",logVar},
                      {"exp",expVar},
                      {"pow",powVar},
                      {"norm",normVar},
                      {"dot",dotVar},
                      {"det",detVar},
                      {"inv",invVar},
                      {"solve",solveVar},
                      {"simitClock",simitClockVar},
                      {"simitStoreTime",simitStoreTimeVar}});
  }
  return byNameMap;
}

}}}

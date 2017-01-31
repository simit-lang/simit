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
const Func& mod() {
  if (!modVar.defined()) {
    modInit();
  }
  return modVar;
}

static Func sinVar;
void sinInit() {
  sinVar = Func("sin",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& sin() {
  if (!sinVar.defined()) {
    sinInit();
  }
  return sinVar;
}

static Func cosVar;
void cosInit() {
  cosVar = Func("cos",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& cos() {
  if (!cosVar.defined()) {
    cosInit();
  }
  return cosVar;
}

static Func tanVar;
void tanInit() {
  tanVar = Func("tan",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& tan() {
  if (!tanVar.defined()) {
    tanInit();
  }
  return tanVar;
}

static Func asinVar;
void asinInit() {
  asinVar = Func("asin",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}
const Func& asin() {
  if (!asinVar.defined()) {
    asinInit();
  }
  return asinVar;
}

static Func acosVar;
void acosInit() {
  acosVar = Func("acos",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}
const Func& acos() {
  if (!acosVar.defined()) {
    acosInit();
  }
  return acosVar;
}

static Func atan2Var;
void atan2Init() {
  atan2Var = Func("atan2",
                  {Var("y", Float), Var("x", Float)},
                  {Var("r", Float)},
                  Func::Intrinsic);
}
const Func& atan2() {
  if (!atan2Var.defined()) {
    atan2Init();
  }
  return atan2Var;
}

static Func sqrtVar;
void sqrtInit() {
  sqrtVar = Func("sqrt",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}
const Func& sqrt() {
  if (!sqrtVar.defined()) {
    sqrtInit();
  }
  return sqrtVar;
}

static Func cbrtVar;
void cbrtInit() {
  cbrtVar = Func("cbrt",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}
const Func& cbrt() {
  if (!cbrtVar.defined()) {
    cbrtInit();
  }
  return cbrtVar;
}

static Func absVar;
void absInit() {
  absVar = Func("abs",
                 {Var("x", Float)},
                 {Var("r", Float)},
                 Func::Intrinsic);
}
const Func& abs() {
  if (!absVar.defined()) {
    absInit();
  }
  return absVar;
}

static Func maxVar;
void maxInit() {
  maxVar = Func("max",
                {Var("a", Float), Var("b", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& max() {
  if (!maxVar.defined()) {
    maxInit();
  }
  return maxVar;
}

static Func minVar;
void minInit() {
  minVar = Func("min",
                {Var("a", Float), Var("b", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& min() {
  if (!minVar.defined()) {
    minInit();
  }
  return minVar;
}

static Func logVar;
void logInit() {
  logVar = Func("log",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& log() {
  if (!logVar.defined()) {
    logInit();
  }
  return logVar;
}

static Func expVar;
void expInit() {
  expVar = Func("exp",
                {Var("x", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& exp() {
  if (!expVar.defined()) {
    expInit();
  }
  return expVar;
}

static Func powVar;
void powInit() {
  powVar = Func("pow",
                {Var("x", Float), Var("y", Float)},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& pow() {
  if (!powVar.defined()) {
    powInit();
  }
  return powVar;
}

static Func createComplexVar;
void createComplexInit() {
  createComplexVar = Func("createComplex",
                          {Var("real", Float), Var("imag", Float)},
                          {Var("r", Complex)},
                          Func::Intrinsic);
}
const Func& createComplex() {
  if (!createComplexVar.defined()) {
    createComplexInit();
  }
  return createComplexVar;
}

static Func complexNormVar;
void complexNormInit() {
  complexNormVar = Func("complexNorm",
                        {Var("c", Complex)},
                        {Var("r", Float)},
                        Func::Intrinsic);
}
const Func& complexNorm() {
  if (!complexNormVar.defined()) {
    complexNormInit();
  }
  return complexNormVar;
}

static Func complexGetRealVar;
void complexGetRealInit() {
  complexGetRealVar = Func("complexGetReal",
                           {Var("c", Complex)},
                           {Var("r", Float)},
                           Func::Intrinsic);
}
const Func& complexGetReal() {
  if (!complexGetRealVar.defined()) {
    complexGetRealInit();
  }
  return complexGetRealVar;
}

static Func complexGetImagVar;
void complexGetImagInit() {
  complexGetImagVar = Func("complexGetImag",
                           {Var("c", Complex)},
                           {Var("r", Float)},
                           Func::Intrinsic);
}
const Func& complexGetImag() {
  if (!complexGetImagVar.defined()) {
    complexGetImagInit();
  }
  return complexGetImagVar;
}

static Func complexConjVar;
void complexConjInit() {
  complexConjVar = Func("complexConj",
                        {Var("c", Complex)},
                        {Var("r", Complex)},
                        Func::Intrinsic);
}
const Func& complexConj() {
  if (!complexConjVar.defined()) {
    complexConjInit();
  }
  return complexConjVar;
}

static Func normVar;
void normInit() {
  normVar = Func("norm",
                 {Var()},
                 {Var("r", Float)},
                 Func::Intrinsic);
}
const Func& norm() {
  if (!normVar.defined()) {
    normInit();
  }
  return normVar;
}

static Func dotVar;
void dotInit() {
  dotVar = Func("dot",
                {Var(), Var()},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& dot() {
  if (!dotVar.defined()) {
    dotInit();
  }
  return dotVar;
}

static Func detVar;
void detInit() {
  detVar = Func("det",
                {Var("m", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& det() {
  if (!detVar.defined()) {
    detInit();
  }
  return detVar;
}

static Func det2Var;
void det2Init() {
  det2Var = Func("det2",
                {Var("m", TensorType::make(ScalarType::Float,
                                           {IndexDomain(2),IndexDomain(2)}))},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& det2() {
  if (!det2Var.defined()) {
    det2Init();
  }
  return det2Var;
}

static Func det4Var;
void det4Init() {
  det4Var = Func("det4",
                {Var("m", TensorType::make(ScalarType::Float,
                                           {IndexDomain(4),IndexDomain(4)}))},
                {Var("r", Float)},
                Func::Intrinsic);
}
const Func& det4() {
  if (!det4Var.defined()) {
    det4Init();
  }
  return det4Var;
}

static Func invVar;
void invInit() {
  invVar = Func("inv",
                {Var("m", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                {Var("r", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                Func::Intrinsic);
}
const Func& inv() {
  if (!invVar.defined()) {
    invInit();
  }
  return invVar;
}

static Func inv2Var;
void inv2Init() {
  inv2Var = Func("inv2",
                {Var("m", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                {Var("r", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                Func::Intrinsic);
}
const Func& inv2() {
  if (!inv2Var.defined()) {
    inv2Init();
  }
  return inv2Var;
}

static Func inv4Var;
void inv4Init() {
  inv4Var = Func("inv4",
                {Var("m", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                {Var("r", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3),IndexDomain(3)}))},
                Func::Intrinsic);
}
const Func& inv4() {
  if (!inv4Var.defined()) {
    inv4Init();
  }
  return inv4Var;
}

static Func crossVar;
void crossInit() {
  crossVar = Func("cross",
                {Var("a", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3)})),
                 Var("b", TensorType::make(ScalarType::Float,
                						   {IndexDomain(3)}))},
                {Var("r", TensorType::make(ScalarType::Float,
                                           {IndexDomain(3)}))},
                Func::External);
}
const Func& cross() {
  if (!crossVar.defined()) {
    crossInit();
  }
  return crossVar;
}

static Func solveVar;
void solveInit() {
  solveVar = Func("__solve",
                  {Var("A", Type()), Var("b", Type()), Var("c", Type())},
                  {Var("r", Float)},
                  Func::Intrinsic);
}
const Func& solve() {
  if (!solveVar.defined()) {
    solveInit();
  }
  return solveVar;
}

static Func luVar;
void luInit() {
  luVar = Func("lu",
                 {Var("A", Type())},
                 {Var("solver", Type(Type::Opaque))},
                 Func::External);
}
const Func& lu() {
  if (!luVar.defined()) {
    luInit();
  }
  return luVar;
}

static Func lufreeVar;
void lufreeInit() {
  lufreeVar = Func("lufree",
                     {Var("A", Type(Type::Opaque))},
                     {},
                     Func::External);
}
const Func& lufree() {
  if (!lufreeVar.defined()) {
    lufreeInit();
  }
  return lufreeVar;
}

static Func lusolveVar;
void lusolveInit() {
  lusolveVar = Func("lusolve",
                    {Var("solver", Type(Type::Opaque)), Var("b", Type())},
                    {Var("x", Type())},
                    Func::External);
}
const Func& lusolve() {
  if (!lusolveVar.defined()) {
    lusolveInit();
  }
  return lusolveVar;
}

static Func lumatsolveVar;
void lumatsolveInit() {
  lumatsolveVar = Func("lumatsolve",
                       {Var("solver", Type(Type::Opaque)), Var("B", Type())},
                       {Var("X", Type())},
                       Func::External);
}
const Func& lumatsolve() {
  if (!lumatsolveVar.defined()) {
    lumatsolveInit();
  }
  return lumatsolveVar;
}

static Func cholVar;
void cholInit() {
  cholVar = Func("chol",
                 {Var("A", Type())},
                 {Var("solver", Type(Type::Opaque))},
                 Func::External);
}
const Func& chol() {
  if (!cholVar.defined()) {
    cholInit();
  }
  return cholVar;
}

static Func cholfreeVar;
void cholfreeInit() {
  cholfreeVar = Func("cholfree",
                     {Var("A", Type(Type::Opaque))},
                     {},
                     Func::External);
}
const Func& cholfree() {
  if (!cholfreeVar.defined()) {
    cholfreeInit();
  }
  return cholfreeVar;
}

static Func lltsolveVar;
void lltsolveInit() {
  lltsolveVar = Func("lltsolve",
                 {Var("solver", Type(Type::Opaque)), Var("b", Type())},
                 {Var("x", Type())},
                 Func::External);
}
const Func& lltsolve() {
  if (!lltsolveVar.defined()) {
    lltsolveInit();
  }
  return lltsolveVar;
}

static Func lltmatsolveVar;
void lltmatsolveInit() {
  lltmatsolveVar = Func("lltmatsolve",
                      {Var("solver", Type(Type::Opaque)), Var("B", Type())},
                      {Var("X", Type())},
                      Func::External);
}
const Func& lltmatsolve() {
  if (!lltmatsolveVar.defined()) {
    lltmatsolveInit();
  }
  return lltmatsolveVar;
}

static Func strcmpVar;
void strcmpInit() {
  strcmpVar = Func("strcmp",
                   {Var("s", String), Var("t", String)},
                   {Var("r", Int)},
                   Func::Intrinsic);
}
const Func& strcmp() {
  if (!strcmpVar.defined()) {
    strcmpInit();
  }
  return strcmpVar;
}

static Func strlenVar;
void strlenInit() {
  strlenVar = Func("strlen",
                   {Var("s", String)},
                   {Var("r", Int)},
                   Func::Intrinsic);
}
const Func& strlen() {
  if (!strlenVar.defined()) {
    strlenInit();
  }
  return strlenVar;
}

static Func strcpyVar;
void strcpyInit() {
  strcpyVar = Func("strcpy",
                   {Var("s", String), Var("t", String)},
                   {Var("r", String)},
                   Func::Intrinsic);
}
const Func& strcpy() {
  if (!strcpyVar.defined()) {
    strcpyInit();
  }
  return strcpyVar;
}

static Func strcatVar;
void strcatInit() {
  strcatVar = Func("strcat",
                   {Var("s", String), Var("t", String)},
                   {Var("r", String)},
                   Func::Intrinsic);
}
const Func& strcat() {
  if (!strcatVar.defined()) {
    strcatInit();
  }
  return strcatVar;
}

static Func clockVar;
void clockInit() {
  clockVar = Func("clock",
                  {},
                  {Var("r", Float)},
                  Func::Intrinsic);
}
const Func& clock() {
  if (!clockVar.defined()) {
    clockInit();
  }
  return clockVar;
}

static Func storeTimeVar;
void storeTimeInit() {
  storeTimeVar = Func("storeTime",
                      {Var("i", Int), Var("val", Float)},
                      {Var("r", Float)},
                      Func::Intrinsic);
}
const Func& storeTime() {
  if (!storeTimeVar.defined()) {
    storeTimeInit();
  }
  return storeTimeVar;
}

static Func mallocVar;
void mallocInit() {
  mallocVar = Func("malloc",
                   {Var("s", Int)},
                   {Var("r", String)},
                   Func::Intrinsic);
}
const Func& malloc() {
  if (!mallocVar.defined()) {
    mallocInit();
  }
  return mallocVar;
}

static Func freeVar;
void freeInit() {
  freeVar = Func("free",
                 {Var("s", String)},
                 {},
                 Func::Intrinsic);
}
const Func& free() {
  if (!freeVar.defined()) {
    freeInit();
  }
  return freeVar;
}

static Func locVar;
void locInit() {
  locVar = Func("__loc",
                {},
                {Var("r", Int)},
                Func::Intrinsic);
}
const Func& loc() {
  if (!locVar.defined()) {
    locInit();
  }
  return locVar;
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
    cbrtInit();
    absInit();
    maxInit();
    minInit();
    logInit();
    expInit();
    powInit();
    createComplexInit();
    complexNormInit();
    complexGetRealInit();
    complexGetImagInit();
    complexConjInit();
    normInit();
    dotInit();
    detInit();
	det2Init();
	det4Init();
    invInit();
	inv2Init();
	inv4Init();
    crossInit();
    solveInit();
    luInit();
    lufreeInit();
    lusolveInit();
    lumatsolveInit();
    cholInit();
    cholfreeInit();
    lltsolveInit();
    lltmatsolveInit();
    strcmpInit();
    strlenInit();
    strcpyInit();
    strcatInit();
    clockInit();
    storeTimeInit();
    mallocInit();
    freeInit();
    locInit();
    byNameMap.insert({{"mod",modVar},
                      {"sin",sinVar},
                      {"cos",cosVar},
                      {"tan",tanVar},
                      {"asin",asinVar},
                      {"acos",acosVar},
                      {"atan2",atan2Var},
                      {"sqrt",sqrtVar},
                      {"cbrt",cbrtVar},
                      {"abs",absVar},
                      {"max",maxVar},
                      {"min",minVar},
                      {"log",logVar},
                      {"exp",expVar},
                      {"pow",powVar},
                      {"createComplex",createComplexVar},
                      {"complexNorm",complexNormVar},
                      {"complexGetReal",complexGetRealVar},
                      {"complexGetImag",complexGetImagVar},
                      {"complexConj",complexConjVar},
                      {"norm",normVar},
                      {"dot",dotVar},
                      {"det",detVar},
					  {"det2",det2Var},
					  {"det4",det4Var},
                      {"inv",invVar},
					  {"inv2",inv2Var},
					  {"inv4",inv4Var},
                      {"cross",crossVar},
					  {"__solve",solveVar},
                      {"lu", luVar},
                      {"lufree", lufreeVar},
                      {"lusolve", lusolveVar},
                      {"lumatsolve", lumatsolveVar},
                      {"chol", cholVar},
                      {"cholfree", cholfreeVar},
                      {"lltsolve", lltsolveVar},
                      {"lltmatsolve", lltmatsolveVar},
                      {"strcmp", strcmpVar},
                      {"strlen", strlenVar},
                      {"strcpy", strcpyVar},
                      {"strcat", strcatVar},
                      {"clock",clockVar},
                      {"storeTime",storeTimeVar},
                      {"malloc", mallocVar},
                      {"free", freeVar},
                      {"__loc", locVar}});
  }
  return byNameMap;
}

}}}

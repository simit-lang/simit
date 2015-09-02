#include "simit-test.h"

#include "tensor.h"
#include "graph.h"
#include "ir.h"

using namespace simit::ir;

TEST(Function, bindSet) {
  Type vertexType = ElementType::make("vertex", {Field("field", Int)});
  Type vertexSetType = SetType::make(vertexType, {});
  Var V("V", vertexSetType);

  Stmt printSet = Print::make(FieldRead::make(V, "field"));

  // Create environment and compile
  Environment env;
  env.addExtern(V);
  simit::Function function = getTestBackend()->compile(printSet, env);
  std::cout << function << std::endl;

  // Create and bind arguments
  simit::Set Varg;
  auto field = Varg.addField<double>("field");
  Varg.add(); Varg.add(); Varg.add();
  function.bind("V", &Varg);

  function.runSafe();
}

TEST(Function, bindTensor) {

}

TEST(Function, bindSparseTensor) {

}

#include "gtest/gtest.h"

#include "ir.h"
#include "lower/index_expressions/lower_scatter_workspace.h"
#include "tensor_index.h"
#include "graph.h"

using namespace std;
using namespace simit;
using namespace simit::ir;

TEST(DISABLED_IndexExpression, add) {
  Var v("V", SetType::make(ElementType::make("v", {}), {}));

  IndexSet is(v);
  IndexDomain dim(is);

  Type type = TensorType::make(ScalarType::Float, {dim,dim});

  Expr B = Var("B", type);
  Expr C = Var("C", type);

  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Expr add = IndexExpr::make({i,j}, B(i,j) + C(i,j));
  Var A("A", add.type());

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add));

  std::cout << loops << std::endl;
}

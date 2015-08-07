#include "gtest/gtest.h"

#include "ir.h"
#include "lower_index_expressions.h"
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

  vector<TensorIndex> tensorIndices;
  Stmt loops = lower(A, to<IndexExpr>(add), &tensorIndices);

  std::cout << loops << std::endl;
//  std::cout << tensorIndices << std::endl;
}

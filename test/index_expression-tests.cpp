#include "gtest/gtest.h"

#include "ir.h"
#include "lower/index_expressions/lower_scatter_workspace.h"
#include "tensor_index.h"
#include "graph.h"

using namespace std;
using namespace simit;
using namespace simit::ir;

TEST(IndexExpression_DISABLED, add) {
  Var v("V", SetType::make(ElementType::make("v", {}), {}));
  IndexSet is(v);
  IndexDomain dim(is);

  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type type = TensorType::make(ScalarType::Float, {dim,dim});

  Expr B = Var("B", type);
  Expr C = Var("C", type);

  Expr add = IndexExpr::make({i,j}, B(i,j) + C(i,j));

  Var A("A", add.type());
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add));

  std::cout << loops << std::endl;
}

TEST(IndexExpression_DISABLED, mul) {
  Var v("V", SetType::make(ElementType::make("v", {}), {}));
  IndexSet is(v);
  IndexDomain dim(is);

  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type type = TensorType::make(ScalarType::Float, {dim,dim});

  Expr B = Var("B", type);
  Expr C = Var("C", type);

  Expr add = IndexExpr::make({i,j}, B(i,j) * C(i,j));

  Var A("A", add.type());
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add));

  std::cout << loops << std::endl;
}

TEST(IndexExpression_DISABLED, addmul) {
  Var v("V", SetType::make(ElementType::make("v", {}), {}));
  IndexSet is(v);
  IndexDomain dim(is);

  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type type = TensorType::make(ScalarType::Float, {dim,dim});

  Expr B = Var("B", type);
  Expr C = Var("C", type);
  Expr D = Var("D", type);

  Expr addmul = IndexExpr::make({i,j}, (B(i,j) + C(i,j)) * D(i,j));

  Var A("A", addmul.type());
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(addmul));

  std::cout << loops << std::endl;
}

TEST(IndexExpression_DISABLED, muladd) {
  Var v("V", SetType::make(ElementType::make("v", {}), {}));
  IndexSet is(v);
  IndexDomain dim(is);

  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type type = TensorType::make(ScalarType::Float, {dim,dim});

  Expr B = Var("B", type);
  Expr C = Var("C", type);
  Expr D = Var("D", type);

  Expr muladd = IndexExpr::make({i,j}, (B(i,j) * C(i,j)) + D(i,j));

  Var A("A", muladd.type());
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(muladd));

  std::cout << loops << std::endl;
}

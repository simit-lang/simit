#include "simit-test.h"

#include "ir.h"
#include "lower/index_expressions/lower_scatter_workspace.h"
#include "tensor_index.h"
#include "graph.h"

using namespace std;
using namespace simit::ir;

TEST(IndexExpression, add) {
  Type vertexType = ElementType::make("vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr add = IndexExpr::make({i,j}, B(i,j) + C(i,j));

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add));
  std::cout << loops << std::endl;

//  simit::Function function = getTestBackend()->compile(loops);
//  std::cout << function << std::endl;
}

TEST(IndexExpression, mul) {
  Type vertexType = ElementType::make("vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr add = IndexExpr::make({i,j}, B(i,j) * C(i,j));

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add));
  std::cout << loops << std::endl;

}

TEST(IndexExpression, addmul) {
  Type vertexType = ElementType::make("vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr D = Var("D", tensorType);
  Expr addmul = IndexExpr::make({i,j}, (B(i,j) + C(i,j)) * D(i,j));

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(addmul));
  std::cout << loops << std::endl;
}

TEST(IndexExpression, muladd) {
  Type vertexType = ElementType::make("vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr D = Var("D", tensorType);
  Expr muladd = IndexExpr::make({i,j}, (B(i,j) * C(i,j)) + D(i,j));

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(muladd));
  std::cout << loops << std::endl;
}

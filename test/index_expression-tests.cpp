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
  Var A("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr add = IndexExpr::make({i,j}, B(i,j) + C(i,j));

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);

  std::cout << env << std::endl << std::endl;
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add), &env);
  std::cout << loops << std::endl << std::endl;
  std::cout << env << std::endl << std::endl;

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

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add), &env);
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

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);
  env.addExtern(to<VarExpr>(D)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(addmul), &env);
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

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);
  env.addExtern(to<VarExpr>(D)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(muladd), &env);
  std::cout << loops << std::endl;
}

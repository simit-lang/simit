#include "simit-test.h"

#include "ir.h"
#include "environment.h"
#include "lower/index_expressions/lower_scatter_workspace.h"

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
//  std::cout << env << std::endl << std::endl;
  std::cout << std::endl;

  simit::Function function = getTestBackend()->compile(loops, env);

  // 1.0 2.0 0.0
  // 3.0 4.0 0.0
  // 0.0 0.0 0.0
  int A_row_ptr[4] = {0, 2, 4, 4};
  int A_col_ind[4] = {0, 1, 0, 1};
  double A_vals[4] = {1.0, 2.0, 3.0, 4.0};
  function.bind("A", A_row_ptr, A_col_ind, A_vals);

  // 0.0 0.0 0.0
  // 0.0 0.1 0.2
  // 0.0 0.3 0.4
  int B_row_ptr[4] = {0, 0, 2, 4};
  int B_col_ind[4] = {1, 2, 1, 2};
  double B_vals[4] = {0.1, 0.2, 0.3, 0.4};
  function.bind("B", B_row_ptr, B_col_ind, B_vals);

  // 1.0 2.0 0.0
  // 3.0 4.1 0.2
  // 0.0 0.3 0.4
  int C_row_ptr[4] = {0, 2, 5, 7};
  int C_col_ind[7] = {0, 1, 0, 1, 2, 1, 2};
  double C_vals[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  function.bind("C", C_row_ptr, C_col_ind, C_vals);

  std::cout << function << std::endl;
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

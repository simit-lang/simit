#include "simit-test.h"

#include "graph.h"
#include "ir.h"
#include "environment.h"
#include "lower/index_expressions/lower_scatter_workspace.h"

using namespace std;
using namespace simit::ir;

TEST(IndexExpression, add) {
  Type vertexType = ElementType::make("Vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var V("V", vertexSetType);

  IndexDomain dim({V});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var A("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr add = IndexExpr::make({i,j}, B(i,j) + C(i,j));

  Environment env;
  env.addExtern(V);
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add), &env);
  simit::Function function = getTestBackend()->compile(loops, env);

  /// The size of V determines the dimensions of the matrices (we don't support
  /// sparse matrices with range dimensions yet).
  simit::Set Varg;
  Varg.add(); Varg.add(); Varg.add();
  function.bind("V", &Varg);

  // 1.0 2.0 0.0
  // 3.0 4.0 0.0
  // 0.0 0.0 0.0
  int B_rowPtr[4]  = {0, 2, 4, 4};
  int B_colInd[4]  = {0, 1, 0, 1};
  double B_vals[4] = {1.0, 2.0, 3.0, 4.0};
  function.bind("B", B_rowPtr, B_colInd, B_vals);

  // 0.0 0.0 0.0
  // 0.0 0.1 0.2
  // 0.0 0.3 0.4
  int C_rowPtr[4]  = {0, 0, 2, 4};
  int C_colInd[4]  = {1, 2, 1, 2};
  double C_vals[4] = {0.1, 0.2, 0.3, 0.4};
  function.bind("C", C_rowPtr, C_colInd, C_vals);

  // 1.0 2.0 0.0
  // 3.0 4.1 0.2
  // 0.0 0.3 0.4
  int A_rowPtr[4] = {0, 2, 5, 7};
  int A_colInd[7] = {0, 1, 0, 1, 2, 1, 2};
  double A_vals[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  function.bind("A", A_rowPtr, A_colInd, A_vals);

  function.runSafe();

  SIMIT_EXPECT_FLOAT_EQ(1.0, B_vals[0]);
  SIMIT_EXPECT_FLOAT_EQ(2.0, B_vals[1]);
  SIMIT_EXPECT_FLOAT_EQ(3.0, B_vals[2]);
  SIMIT_EXPECT_FLOAT_EQ(4.0, B_vals[3]);

  SIMIT_EXPECT_FLOAT_EQ(0.1, C_vals[0]);
  SIMIT_EXPECT_FLOAT_EQ(0.2, C_vals[1]);
  SIMIT_EXPECT_FLOAT_EQ(0.3, C_vals[2]);
  SIMIT_EXPECT_FLOAT_EQ(0.4, C_vals[3]);

  SIMIT_EXPECT_FLOAT_EQ(1.0, A_vals[0]);
  SIMIT_EXPECT_FLOAT_EQ(2.0, A_vals[1]);
  SIMIT_EXPECT_FLOAT_EQ(3.0, A_vals[2]);
  SIMIT_EXPECT_FLOAT_EQ(4.1, A_vals[3]);
  SIMIT_EXPECT_FLOAT_EQ(0.2, A_vals[4]);
  SIMIT_EXPECT_FLOAT_EQ(0.3, A_vals[5]);
  SIMIT_EXPECT_FLOAT_EQ(0.4, A_vals[6]);
}

TEST(IndexExpression, mul) {
  Type vertexType = ElementType::make("Vertex", {});
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
  Type vertexType = ElementType::make("Vertex", {});
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
  Type vertexType = ElementType::make("Vertex", {});
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

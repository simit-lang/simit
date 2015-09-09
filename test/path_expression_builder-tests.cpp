#include "simit-test.h"
#include "path_indices-tests.h"

#include "graph.h"
#include "ir.h"
#include "path_expressions.h"
#include "path_expression_analysis.h"
#include "path_indices.h"

using namespace std;
using namespace simit;
using namespace simit::ir;

static const Type vType = ElementType::make("Vertex", {});
static const Type VType = SetType::make(vType, {});
static const ir::Var V("V", VType);
static const ir::Var U("U", VType);

static const Type eType = ElementType::make("Edge", {});
static const Type EType = SetType::make(eType, {V,V});
static const ir::Var E("E", EType);
static const ir::Var F("F", EType);

static const IndexDomain dim({V});
static const IndexVar i("i", dim);
static const IndexVar j("j", dim);
static const IndexVar k("k", dim, ReductionOperator::Sum);

static
Func f("f", {Var("e", eType), Var("v", TupleType::make(eType, 2))},
       {Var("R",ir::TensorType::make(ir::typeOf<simit_float>(),{dim,dim}))},
       Func::External);

TEST(PathExpressionBuilder, add) {
  Var A("A", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var B("B", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var C("C", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));

  Stmt mapB = Map::make({B}, f, {}, E);
  Stmt mapC = Map::make({C}, f, {}, F);
  Expr iexpr = IndexExpr::make({i,j}, Expr(B)(i,j) + Expr(C)(i,j));

  PathExpressionBuilder builder;
  builder.computePathExpression(to<Map>(mapB));
  builder.computePathExpression(to<Map>(mapC));
  builder.computePathExpression(A, to<IndexExpr>(iexpr));

  Set Vs;
  Set Es(Vs,Vs);
  Set Fs(Vs,Vs);
  createTestGraph0(&Vs, &Es, &Fs);

  builder.bind(V, &Vs);
  builder.bind(E, &Es);
  builder.bind(F, &Fs);

  pe::PathExpression pe = builder.getPathExpression(A);
  ASSERT_TRUE(pe.isBound());

  pe::PathIndexBuilder indexBuilder;
  pe::PathIndex pi = indexBuilder.buildSegmented(pe, 0);
  VERIFY_INDEX(pi, nbrs({{0,1,3}, {0,1,2}, {1,2,3}, {0,2,3}}));
}

TEST(PathExpressionBuilder, mul) {
  Var A("A", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var B("B", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var C("C", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));

  Stmt mapB = Map::make({B}, f, {}, E);
  Stmt mapC = Map::make({C}, f, {}, F);
  Expr iexpr = IndexExpr::make({i,j}, Expr(B)(i,j) * Expr(C)(i,j));

  PathExpressionBuilder builder;
  builder.computePathExpression(to<Map>(mapB));
  builder.computePathExpression(to<Map>(mapC));
  builder.computePathExpression(A, to<IndexExpr>(iexpr));

  Set Vs;
  Set Es(Vs,Vs);
  Set Fs(Vs,Vs);
  createTestGraph0(&Vs, &Es, &Fs);

  builder.bind(V, &Vs);
  builder.bind(E, &Es);
  builder.bind(F, &Fs);

  pe::PathExpression pe = builder.getPathExpression(A);
  ASSERT_TRUE(pe.isBound());

  pe::PathIndexBuilder indexBuilder;
  pe::PathIndex pi = indexBuilder.buildSegmented(pe, 0);
  VERIFY_INDEX(pi, nbrs({{0,1}, {0,1}, {2}, {}}));

  pe::PathExpression peBound = builder.getPathExpression(A);

  ASSERT_TRUE(peBound.isBound());

  std::cout << "A: " << pe << std::endl;
}

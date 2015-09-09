#include "simit-test.h"

#include "graph.h"
#include "ir.h"
#include "path_expressions.h"
#include "path_expression_analysis.h"

using namespace simit;
using namespace simit::ir;

static const Type vType = ElementType::make("Vertex", {});
static const Type VType = SetType::make(vType, {});
static const ir::Var V("V", VType);

static const Type eType = ElementType::make("Edge", {});
static const Type EType = SetType::make(eType, {V,V});
static const ir::Var E("E", EType);

static const IndexDomain dim({V});
static const IndexVar i("i", dim);
static const IndexVar j("j", dim);

static
Func f("f", {Var("e", eType), Var("v", TupleType::make(eType, 2))},
       {Var("R",ir::TensorType::make(ir::typeOf<simit_float>(),{dim,dim}))},
       Func::External);

TEST(PathExpressionBuilder, add) {
  Var A("A", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var B("B", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var C("C", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));

  Stmt mapB = Map::make({B}, f, {}, E);
  Stmt mapC = Map::make({C}, f, {}, E);

  Expr iexpr = IndexExpr::make({i,j}, Expr(B)(i,j) + Expr(C)(i,j));
  std::cout << iexpr << std::endl;

  PathExpressionBuilder builder;
  builder.computePathExpression(to<Map>(mapB));
  builder.computePathExpression(to<Map>(mapC));
  builder.computePathExpression(A, to<IndexExpr>(iexpr));

  pe::PathExpression Bpe = builder.getPathExpression(B);
  pe::PathExpression Cpe = builder.getPathExpression(C);

  pe::PathExpression pe = builder.getPathExpression(A);
  std::cout << "A: " << pe << std::endl;

  Set Vs("V");
  Set Es("E", Vs,Vs);
  Box box = createBox(&Vs, &Es, 3, 2, 1);

  builder.bind(V, &Vs);
  builder.bind(E, &Es);

  pe::PathExpression peBound = builder.getPathExpression(A);

  ASSERT_TRUE(peBound.isBound());

  std::cout << "A: " << pe << std::endl;
}

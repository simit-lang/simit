#include "simit-test.h"

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

TEST(PathExpressionBuilder, add) {
  Var A("A", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var B("B", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));
  Var C("C", ir::TensorType::make(ir::typeOf<simit_float>(), {dim,dim}));

  Func f("f", {Var("e", eType), Var("v", TupleType::make(eType, 2))}, {B,C},
         Func::External);
  Stmt map = Map::make({B,C}, f, {}, E);

  Expr iexpr = IndexExpr::make({i,j}, Expr(B)(i,j) + Expr(C)(i,j));
  std::cout << iexpr << std::endl;

  PathExpressionBuilder builder;
  builder.computePathExpression(to<Map>(map));
  builder.computePathExpression(A, to<IndexExpr>(iexpr));

  pe::PathExpression Bpe = builder.getPathExpression(B);
  pe::PathExpression Cpe = builder.getPathExpression(C);

  pe::PathExpression Ape = builder.getPathExpression(A);
  std::cout << "A: " << Ape << std::endl;
}

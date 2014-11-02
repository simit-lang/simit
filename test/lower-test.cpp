#include "gtest/gtest.h"

#include "types.h"
#include "ir.h"
#include "indexvar.h"
#include "lower.h"

using namespace std;
using namespace testing;
using namespace simit;
using namespace simit::ir;

const Expr a = Var("a", vec3f);
const Expr b = Var("b", vec3f);

const IndexVar i = IndexVar("i", IndexDomain(IndexSet(3)));
const IndexVar j = IndexVar("j", IndexDomain(IndexSet(3)));

TEST(FlattenIndexExpressions, IndexExprInOuterProduct) {
  Expr aexpr = IndexExpr::make({i}, IndexedTensor::make(a, {i}));

  Expr mul = Mul::make(IndexedTensor::make(aexpr,{i}),
                       IndexedTensor::make(b,{j}));
  Expr nest = IndexExpr::make({i,j}, mul);

  Expr flat = flattenIndexExpressions(nest);

  ASSERT_TRUE(isa<IndexExpr>(flat));
  const IndexExpr *indexExpr = to<IndexExpr>(flat);

  ASSERT_EQ(indexExpr->resultVars.size(), 2u);
  ASSERT_EQ(indexExpr->resultVars[0], i);
  ASSERT_EQ(indexExpr->resultVars[1], j);
}

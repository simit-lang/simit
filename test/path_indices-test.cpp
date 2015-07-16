#include "gtest/gtest.h"

#include <map>
#include <set>
#include <iostream>

#include "graph.h"
#include "path_expressions.h"
#include "path_indices.h"

using namespace simit;
using namespace simit::pe;
using namespace std;

#define VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs)   \
do {                                                         \
  int i = 0;                                                 \
  for (auto e : index) {                                     \
    ASSERT_EQ(expectedNumNbrs[i], index.numNeighbors(e));    \
    int j = 0;                                               \
    for (auto n : index.neighbors(e)) {                      \
      ASSERT_EQ(expectedNbrs[i][j], n)                       \
          << "expects neighbor " << j << " of element " << i \
          << " to be " << expectedNbrs[i][j];                \
      ++j;                                                   \
    }                                                        \
    ++i;                                                     \
  }                                                          \
} while(0)


TEST(PathIndex, EV) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  Var e("e", E);
  Var v("v", V);
  PathExpression ev = EV::make(e, v);
  PathIndex evIndex = builder.buildSegmented(ev, 0);
  ASSERT_EQ(4u, evIndex.numElements());
  ASSERT_EQ(4u*2, evIndex.numNeighbors());
  VERIFY_INDEX(evIndex,
               vector<unsigned>({2, 2, 2, 2}),
               vector<vector<unsigned>>({{0, 1}, {1, 2}, {2, 3}, {3, 4}}));

  // Check that EV get's memoized
  Var f("f", E);
  Var u("u", V);
  PathExpression fu = EV::make(f, u);
  PathIndex fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_EQ(evIndex, fuIndex);

  // Check that different EV get's a different index
  Set U;
  Set F(V,V);
  fu = fu.bind({{f,F}, {u,U}});
  fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_NE(evIndex, fuIndex);

  // Check that EV evaluated backwards get's a different index
  // TODO
}


TEST(PathIndex, VE) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  Var v("v", V);
  Var e("e", E);
  PathExpression ve = VE::make(v, e);
  PathIndex veIndex = builder.buildSegmented(ve, 0);
  ASSERT_EQ(5u, veIndex.numElements());
  ASSERT_EQ(8u, veIndex.numNeighbors());
  VERIFY_INDEX(veIndex,
               vector<unsigned>({1, 2, 2, 2, 1}),
               vector<vector<unsigned>>({{0}, {0, 1}, {1, 2}, {2, 3}, {3}}));

  // Check that VE get's memoized
  Var u("u", V);
  Var f("f", E);
  PathExpression uf = VE::make(u,f);
  PathIndex ufIndex = builder.buildSegmented(uf, 0);
  ASSERT_EQ(veIndex, ufIndex);

  // Check that different VE get's a different index
  Set U;
  Set F(V,V);
  uf = uf.bind({{f,F}, {u,U}});
  ufIndex = builder.buildSegmented(uf, 0);
  ASSERT_NE(veIndex, ufIndex);

  // Check that VE evaluated backwards get's a different index
  // TODO
}


TEST(PathIndex, VEV) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);  // v-e-v-e-v

  Var vi("vi", V);
  Var e("e", E);
  Var vj("vj", V);
  PathExpression ve = VE::make(vi, e);
  PathExpression ev = EV::make(e, vj);
  QuantifiedVar qe = QuantifiedVar(QuantifiedVar::Existential, e);
  PathExpression vev = And::make({vi,vj}, {qe}, ve, ev);
  PathIndex vevIndex = builder.buildSegmented(vev, 0);
  ASSERT_EQ(3u, vevIndex.numElements());
  ASSERT_EQ(7u, vevIndex.numNeighbors());
  VERIFY_INDEX(vevIndex,
               vector<unsigned>({2, 3, 2}),
               vector<vector<unsigned>>({{0, 1}, {0, 1, 2}, {1, 2}}));

  // Check that VEV get's memoized
  Var ui("ui", V);
  Var f("e", E);
  Var uj("uj", V);
  PathExpression uf = VE::make(ui, f);
  PathExpression fu = EV::make(f, uj);
  QuantifiedVar qf = QuantifiedVar(QuantifiedVar::Existential, f);
  PathExpression ufu = And::make({ui,uj}, {qf}, uf, fu);
  PathIndex ufuIndex = builder.buildSegmented(ufu, 0);
  ASSERT_EQ(vevIndex, ufuIndex);

  // Check that different VEV get's a different index
  Set U;
  Set F(U,U);
  ufu = ufu.bind({{ui,U}, {f,F}, {uj,U}});
  ufuIndex = builder.buildSegmented(ufu, 0);
  ASSERT_NE(vevIndex, ufuIndex);

  // Check that VEV evaluated backwards get's a different index
  // TODO
}


TEST(PathIndex, Alias) {
  Set V;
  Set E(V,V);

  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  E.add(v0,v1);
  E.add(v1,v0);

  Var vi("vi", V);
  Var e("e", E);
  Var vj("vj", V);
  PathExpression ve = VE::make(vi, e);
  PathExpression ev = EV::make(e, vj);

  QuantifiedVar quantifiedVar = QuantifiedVar(QuantifiedVar::Existential, e);
  PathExpression vev = And::make({vi,vj}, {quantifiedVar}, ve, ev);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(vev, 0);

  ASSERT_EQ(2u, index.numElements());
  ASSERT_EQ(4u, index.numNeighbors());

  vector<unsigned> expectedNumNbrs = {2, 2};
  vector<vector<unsigned>> expectedNbrs={{0, 1}, {0, 1}};
  VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs);
}

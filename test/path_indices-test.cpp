#include "gtest/gtest.h"
#include "path_expressions-test.h"

#include <map>
#include <set>
#include <iostream>

#include "graph.h"
#include "path_expressions.h"
#include "path_indices.h"

using namespace simit;
using namespace simit::pe;
using namespace std;

typedef vector<vector<unsigned>> nbrs;

#define VERIFY_INDEX(index, expectedNbrs)                                      \
do {                                                                           \
  auto expectedNeighbors = expectedNbrs;                                       \
  ASSERT_EQ(expectedNeighbors.size(), index.numElements());                    \
  unsigned i = 0;                                                              \
  unsigned int totalNbrs=0;                                                    \
  for (auto e : index) {                                                       \
    ASSERT_EQ(expectedNeighbors[i].size(), index.numNeighbors(e));             \
    unsigned j = 0;                                                            \
    for (auto n : index.neighbors(e)) {                                        \
      ASSERT_EQ(expectedNeighbors[i][j], n)                                    \
          << "expects neighbor " << j << " of element " << i                   \
          << " to be " << expectedNeighbors[i][j];                             \
      ++j;                                                                     \
    }                                                                          \
    totalNbrs += j;                                                            \
    ++i;                                                                       \
  }                                                                            \
  ASSERT_EQ(totalNbrs, index.numNeighbors());                                  \
} while(0)


TEST(PathIndex, Link) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  // Test e-v links
  Var e("e");
  Var v("v");
  PathExpression ev = Link::make(e, v, Link::ev);
  ev.bind(E,V);
  PathIndex evIndex = builder.buildSegmented(ev, 0);
  VERIFY_INDEX(evIndex, nbrs({{0,1}, {1,2}, {2,3}, {3,4}}));

  // Check that EV get's memoized
  Var f("f");
  Var u("u");
  PathExpression fu = Link::make(f, u, Link::ev);
  fu.bind(E,V);
  PathIndex fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_EQ(evIndex, fuIndex);

  // Check that different ev get's a different index
  Set U;
  Set F(V,V);
  fu.bind(F,U);
  fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_NE(evIndex, fuIndex);

  // Test v-e links
  PathExpression ve = Link::make(v, e, Link::ve);
  ve.bind(V,E);
  PathIndex veIndex = builder.buildSegmented(ve, 0);
  VERIFY_INDEX(veIndex, nbrs({{0}, {0,1}, {1,2}, {2,3}, {3}}));

  // Check that ve get's memoized
  PathExpression uf = Link::make(u,f, Link::ve);
  uf.bind(V,E);
  PathIndex ufIndex = builder.buildSegmented(uf, 0);
  ASSERT_EQ(veIndex, ufIndex);

  // Check that different VE get's a different index
  uf.bind(U,F);
  ufIndex = builder.buildSegmented(uf, 0);
  ASSERT_NE(veIndex, ufIndex);

  // Test that ev evaluated backwards gets the same index as ve and vice versa
  // TODO

  // Test ve where some variables do not have neighbors
  Set G(V,V);
  G.add(box(0,0,0), box(4,0,0));
  Var g("g");
  PathExpression vg = Link::make(v, g, Link::ve);
  vg.bind(V,G);
  PathIndex vgIndex = builder.buildSegmented(vg, 0);
  VERIFY_INDEX(vgIndex, nbrs({{0}, {}, {}, {}, {0}}));
}


TEST(PathIndex, ExistAnd) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);  // v-e-v-e-v

  // Test vev expressions (there exist an e s.t. (vi-e and e-vj))
  PathExpression ve = makeVE();
  PathExpression ev = makeEV();
  ve.bind(V,E);
  ev.bind(E,V);

  Var vi("vi");
  Var e("e");
  Var vj("vj");
  PathExpression vev = And::make({vi,vj}, {{QuantifiedVar::Exist,e}},
                                 ve(vi, e), ev(e, vj));
  PathIndex vevIndex = builder.buildSegmented(vev, 0);
  VERIFY_INDEX(vevIndex, nbrs({{0,1}, {0,1,2}, {1,2}}));

  // Check that vev get's memoized
  Var u("u");
  Var f("f");
  PathExpression uf = Link::make(u, f, Link::ve);
  PathExpression fu = Link::make(f, u, Link::ev);
  uf.bind(V,E);
  fu.bind(E,V);

  Var ui("ui");
  Var ff("f");
  Var uj("uj");
  PathExpression ufu = And::make({ui,uj}, {{QuantifiedVar::Exist,ff}},
                                 uf(ui, ff), fu(ff,uj));
  PathIndex ufuIndex = builder.buildSegmented(ufu, 0);
  ASSERT_EQ(vevIndex, ufuIndex);

  // Check that a different vev get's a different index
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  fu.bind(F,U);
  ufuIndex = builder.buildSegmented(ufu, 0);
  ASSERT_NE(vevIndex, ufuIndex);

  // Check that vev evaluated backwards get's a different index
  // TODO

  // Test vevev expression
  Var vk("vk");
  PathExpression vevev = And::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                   vev(vi,vk), vev(vk, vj));
  PathIndex vevevIndex = builder.buildSegmented(vevev, 0);
  VERIFY_INDEX(vevevIndex, nbrs({{0,1,2}, {0,1,2}, {0,1,2}}));

  // Test vevgv expressions: v-e-v-e-v
  //                          ---g---
  Set G(V,V);
  G.add(box(0,0,0), box(2,0,0));

  Var g("g");
  PathExpression vg = makeVE();
  PathExpression gv = makeEV();
  vg.bind(V,G);
  gv.bind(G,V);
  PathExpression vgv = And::make({vi,vj}, {{QuantifiedVar::Exist,g}},
                                 vg(vi, g), gv(g, vj));
  PathIndex vgvIndex = builder.buildSegmented(vgv, 0);
  PathExpression vevgv = And::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                   vev(vi,vk), vgv(vk, vj));
  PathIndex vevgvIndex = builder.buildSegmented(vevgv, 0);
  VERIFY_INDEX(vevgvIndex, nbrs({{0,2}, {0,2}, {0,2}}));
}


TEST(PathIndex, Or) {
  PathIndexBuilder builder;

  //  -f-
  // v-e-v-e-v-f-v
  //  -----f-----
  Set V;
  Set E(V,V);
  Set F(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);
  ElementRef v3 = V.add();
  F.add(box(2,0,0), v3);
  F.add(box(0,0,0), v3);
  F.add(box(0,0,0), box(1,0,0));

  // test (ve or ve)
  PathExpression ve = makeVE();
  ve.bind(V,E);
  Var v("v");
  Var e("e");
  PathExpression veORve = Or::make({v,e}, {}, ve(v,e), ve(e,v));
  PathIndex veORveIndex = builder.buildSegmented(veORve, 0);
  VERIFY_INDEX(veORveIndex, nbrs({{0}, {0,1}, {1}, {}}));

  // test (vev or vfv):
  PathExpression ev = makeEV();
  ev.bind(E,V);
  Var vi("vi");
  Var vj("vj");
  PathExpression vev = And::make({vi,vj}, {{QuantifiedVar::Exist,e}},
                                 ve(vi, e), ev(e, vj));
  PathExpression vf = makeVE();
  PathExpression fv = makeEV();
  vf.bind(V,F);
  fv.bind(F,V);
  Var f("f");
  PathExpression vfv = And::make({vi,vj}, {{QuantifiedVar::Exist,f}},
                                 vf(vi, f), fv(f, vj));
  PathExpression vevORvfv = Or::make({vi,vj}, {}, vev(vi,vj), vfv(vi,vj));
  PathIndex vevORvfvIndex = builder.buildSegmented(vevORvfv, 0);
  VERIFY_INDEX(vevORvfvIndex, nbrs({{0,1,3}, {0,1,2}, {1,2,3}, {0,2,3}}));

  // TODO: Test optimization PathIndex(pe) == PathIndex(pe or pe)
}


TEST(PathIndex, Alias) {
  Set V;
  Set E(V,V);

  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  E.add(v0,v1);
  E.add(v1,v0);

  Var v("v");
  Var e("e");
  PathExpression ve = Link::make(v, e, Link::ve);
  PathExpression ev = Link::make(e, v, Link::ev);
  ve.bind(V,E);
  ev.bind(E,V);

  Var vi("vi");
  Var ee("e");
  Var vj("vj");
  PathExpression vev = And::make({vi,vj}, {{QuantifiedVar::Exist, ee}},
                                 ve(vi,ee), ev(ee,vj));

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(vev, 0);


  VERIFY_INDEX(index, vector<vector<unsigned>>({{0,1}, {0,1}}));
}

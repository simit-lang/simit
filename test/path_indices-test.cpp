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

typedef vector<vector<unsigned>> nbrs;

#define VERIFY_INDEX(index, expectedNbrs)                                      \
do {                                                                           \
  auto expectedNeighbors = expectedNbrs;                                       \
  int i = 0;                                                                   \
  for (auto e : index) {                                                       \
    ASSERT_EQ(expectedNeighbors[i].size(), index.numNeighbors(e));             \
    int j = 0;                                                                 \
    for (auto n : index.neighbors(e)) {                                        \
      ASSERT_EQ(expectedNeighbors[i][j], n)                                    \
          << "expects neighbor " << j << " of element " << i                   \
          << " to be " << expectedNeighbors[i][j];                             \
      ++j;                                                                     \
    }                                                                          \
    ++i;                                                                       \
  }                                                                            \
} while(0)


TEST(PathIndex, Link) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  Var e("e");
  Var v("v");

  // Test e-v links
  PathExpression ev = Link::make(e, v, Link::ev);
  ev.bind(E,V);
  PathIndex evIndex = builder.buildSegmented(ev, 0);
  ASSERT_EQ(4u, evIndex.numElements());
  ASSERT_EQ(4u*2, evIndex.numNeighbors());
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
  ASSERT_EQ(5u, veIndex.numElements());
  ASSERT_EQ(8u, veIndex.numNeighbors());
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
  ASSERT_EQ(5u, vgIndex.numElements());
  ASSERT_EQ(2u, vgIndex.numNeighbors());
  VERIFY_INDEX(vgIndex, nbrs({{0}, {}, {}, {}, {0}}));
}


TEST(PathIndex, ExistAnd) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);  // v-e-v-e-v

  // Test vev expressions (there exist an e s.t. (vi-e and e-vj))
  Var v("v");
  Var e("e");
  PathExpression ve = Link::make(v, e, Link::ve);
  PathExpression ev = Link::make(e, v, Link::ev);
  ve.bind(V,E);
  ev.bind(E,V);

  Var vi("vi");
  Var ee("e");
  Var vj("vj");
  PathExpression vev = And::make({vi,vj}, {{QuantifiedVar::Exist,ee}},
                                 ve(vi, ee), ev(ee, vj));
  PathIndex vevIndex = builder.buildSegmented(vev, 0);
  ASSERT_EQ(3u, vevIndex.numElements());
  ASSERT_EQ(7u, vevIndex.numNeighbors());
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
  ASSERT_EQ(3u, vevevIndex.numElements());
  ASSERT_EQ(9u, vevevIndex.numNeighbors());
  VERIFY_INDEX(vevevIndex, nbrs({{0,1,2}, {0,1,2}, {0,1,2}}));

  // Test vevgv expressions  v-e-v-e-v
  //                          ---g---
  Set G(V,V);
  G.add(box(0,0,0), box(2,0,0));

  Var g("g");
  PathExpression vg = Link::make(v, g, Link::ve);
  PathExpression gv = Link::make(g, v, Link::ev);
  vg.bind(V,G);
  gv.bind(G,V);
  PathExpression vgv = And::make({vi,vj}, {{QuantifiedVar::Exist,g}},
                                 vg(vi, g), gv(g, vj));
  PathIndex vgvIndex = builder.buildSegmented(vgv, 0);
  PathExpression vevgv = And::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                   vev(vi,vk), vgv(vk, vj));
  PathIndex vevgvIndex = builder.buildSegmented(vevgv, 0);
  ASSERT_EQ(3u, vevgvIndex.numElements());
  ASSERT_EQ(6u, vevgvIndex.numNeighbors());
  VERIFY_INDEX(vevgvIndex, nbrs({{0,2}, {0,2}, {0,2}}));
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

  ASSERT_EQ(2u, index.numElements());
  ASSERT_EQ(4u, index.numNeighbors());
  VERIFY_INDEX(index, vector<vector<unsigned>>({{0,1}, {0,1}}));
}

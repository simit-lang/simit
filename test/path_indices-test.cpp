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
  int i = 0;                                                                   \
  for (auto e : index) {                                                       \
    ASSERT_EQ(expectedNbrs[i].size(), index.numNeighbors(e));                  \
    int j = 0;                                                                 \
    for (auto n : index.neighbors(e)) {                                        \
      ASSERT_EQ(expectedNbrs[i][j], n)                                         \
          << "expects neighbor " << j << " of element " << i                   \
          << " to be " << expectedNbrs[i][j];                                  \
      ++j;                                                                     \
    }                                                                          \
    ++i;                                                                       \
  }                                                                            \
} while(0)


TEST(PathIndex, Link) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  Var e("e");
  Var v("v");

  // Test e-v links
  PathExpression ev = Link::make(e, v, Link::ev);
  ev.bind({{e,E}, {v,V}});
  PathIndex evIndex = builder.buildSegmented(ev, 0);
  ASSERT_EQ(4u, evIndex.numElements());
  ASSERT_EQ(4u*2, evIndex.numNeighbors());
  VERIFY_INDEX(evIndex, nbrs({{0,1}, {1,2}, {2,3}, {3,4}}));

  // Check that EV get's memoized
  Var f("f");
  Var u("u");
  PathExpression fu = Link::make(f, u, Link::ev);
  fu.bind({{f,E}, {u,V}});
  PathIndex fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_EQ(evIndex, fuIndex);

  // Check that different EV get's a different index
  Set U;
  Set F(V,V);
  fu.bind({{f,F}, {u,U}});
  fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_NE(evIndex, fuIndex);

  // Test v-e links
  PathExpression ve = Link::make(v, e, Link::ve);
  ve.bind({{e,E}, {v,V}});
  PathIndex veIndex = builder.buildSegmented(ve, 0);
  ASSERT_EQ(5u, veIndex.numElements());
  ASSERT_EQ(8u, veIndex.numNeighbors());
  VERIFY_INDEX(veIndex, nbrs({{0}, {0,1}, {1,2}, {2,3}, {3}}));

  // Check that VE get's memoized
  PathExpression uf = Link::make(u,f, Link::ve);
  uf.bind({{f,E}, {u,V}});
  PathIndex ufIndex = builder.buildSegmented(uf, 0);
  ASSERT_EQ(veIndex, ufIndex);

  // Check that different VE get's a different index
  uf.bind({{f,F}, {u,U}});
  ufIndex = builder.buildSegmented(uf, 0);
  ASSERT_NE(veIndex, ufIndex);

  // Test that ev evaluated backwards gets the same index as ve and vice versa
  // TODO
}


TEST(PathIndex, ExistAnd_vev) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);  // v-e-v-e-v

  // Test vev expressions (there exist an e s.t. (vi-e and e-vj))
  Var v("v");
  Var e("e");
  PathExpression ve = Link::make(v, e, Link::ve);
  PathExpression ev = Link::make(e, v, Link::ev);
  ve.bind({{v,V}, {e,E}});
  ev.bind({{v,V}, {e,E}});

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
  uf.bind({{u,V}, {f,E}});
  fu.bind({{u,V}, {f,E}});

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
  uf.bind({{u,U}, {f,F}});
  fu.bind({{u,U}, {f,F}});
  ufuIndex = builder.buildSegmented(ufu, 0);
  ASSERT_NE(vevIndex, ufuIndex);

  // Check that VEV evaluated backwards get's a different index
  // TODO

  // Test VEVEV expression
  Var vk("vk");
  PathExpression vevev = And::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                   vev(vi,vk), vev(vk, vj));
  PathIndex vevevIndex = builder.buildSegmented(vevev, 0);
  ASSERT_EQ(3u, vevevIndex.numElements());
  ASSERT_EQ(9u, vevevIndex.numNeighbors());
  VERIFY_INDEX(vevevIndex, nbrs({{0,1,2}, {0,1,2}, {0,1,2}}));
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
  ve.bind({{v,V}, {e,E}});
  ev.bind({{v,V}, {e,E}});

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

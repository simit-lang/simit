#include "simit-test.h"
#include "path_indices-tests.h"
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
  PathExpression ev2 = Link::make(e, v, Link::ev);
  ev2.bind(E,V);
  PathIndex ev2Index = builder.buildSegmented(ev, 0);
  ASSERT_EQ(evIndex, ev2Index);

  // Check that different ev get's a different index
  Var f("f");
  Var u("u");
  PathExpression fu = Link::make(f, u, Link::ev);
  Set U;
  Set F(V,V);
  fu.bind(F,U);
  PathIndex fuIndex = builder.buildSegmented(fu, 0);
  ASSERT_NE(evIndex, fuIndex);

  // Test v-e links
  PathExpression ve = Link::make(v, e, Link::ve);
  ve.bind(V,E);
  PathIndex veIndex = builder.buildSegmented(ve, 0);
  VERIFY_INDEX(veIndex, nbrs({{0}, {0,1}, {1,2}, {2,3}, {3}}));

  // Check that ve get's memoized
  PathExpression ve2 = Link::make(v, e, Link::ve);
  ve2.bind(V,E);
  PathIndex ve2Index = builder.buildSegmented(ve2, 0);
  ASSERT_EQ(veIndex, ve2Index);

  // Check that different VE get's a different index
  PathExpression uf = Link::make(u,f, Link::ve);
  uf.bind(U,F);
  PathIndex ufIndex = builder.buildSegmented(uf, 0);
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


TEST(PathIndex, And) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Set F(V,V);
  createTestGraph0(&V, &E, &F);

  // test (ve or ve)
  PathExpression ve = makeVE();
  ve.bind(V,E);
  Var v("v");
  Var e("e");
  PathExpression veANDve = And::make({v,e}, {}, ve(v,e), ve(e,v));
  PathIndex veANDveIndex = builder.buildSegmented(veANDve, 0);
  VERIFY_INDEX(veANDveIndex, nbrs({{0}, {0,1}, {1}, {}}));

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
  PathExpression vevANDvfv = And::make({vi,vj}, {}, vev(vi,vj), vfv(vi,vj));
  PathIndex vevANDvfvIndex = builder.buildSegmented(vevANDvfv, 0);
  VERIFY_INDEX(vevANDvfvIndex, nbrs({{0,1}, {0,1}, {2}, {}}));

  // TODO: Test optimization PathIndex(pe or pe) == PathIndex(pe)
}


TEST(PathIndex, Or) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Set F(V,V);
  createTestGraph0(&V, &E, &F);

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
  Var v2("v2");
  Var e2("e2");
  PathExpression ve2 = Link::make(v2, e2, Link::ve);
  PathExpression ev2 = Link::make(e2, v2, Link::ev);
  ve2.bind(V,E);
  ev2.bind(E,V);

  Var vi2("vi2");
  Var ee2("f");
  Var vj2("vj2");
  PathExpression vev2 = And::make({vi2,vj2}, {{QuantifiedVar::Exist,ee2}},
                                  ve(vi2, ee2), ev(ee2,vj2));
  PathIndex vev2Index = builder.buildSegmented(vev2, 0);
  ASSERT_EQ(vevIndex, vev2Index);

  // Check that a different vev get's a different index
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
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  fu.bind(F,U);
  PathIndex ufuIndex = builder.buildSegmented(ufu, 0);
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
  ASSERT_NE(vev, vgv);
  ASSERT_TRUE(vev < vgv || vev > vgv);
  ASSERT_NE(vevIndex, vgvIndex);

  PathExpression vevgv = And::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                   vev(vi,vk), vgv(vk, vj));
  PathIndex vevgvIndex = builder.buildSegmented(vevgv, 0);
  VERIFY_INDEX(vevgvIndex, nbrs({{0,2}, {0,2}, {0,2}}));
}


TEST(PathIndex, ExistOr) {
  PathIndexBuilder builder;

  Set V;
  Set E(V,V);
  Set G(V,V);
  createTestGraph1(&V, &E, &G);

  // Test vev expressions (there exist an e s.t. (vi-e and e-vj))
  PathExpression ve = makeVE();
  PathExpression ev = makeEV();
  ve.bind(V,E);
  ev.bind(E,V);

  Var vi("vi");
  Var e("e");
  Var vj("vj");
  PathExpression vev = Or::make({vi,vj}, {{QuantifiedVar::Exist,e}},
                                ve(vi, e), ev(e, vj));
  PathIndex vevIndex = builder.buildSegmented(vev, 0);
  VERIFY_INDEX(vevIndex, nbrs({{1,2}, {0,1,2,3,4}, {0,1,2,3,4}, {1,2}, {1,2}}));

  // Check that vev gets memoized
  Var vi2("vi2");
  Var ee2("e2");
  Var vj2("vj2");
  PathExpression vev2 = Or::make({vi2,vj2}, {{QuantifiedVar::Exist,ee2}},
                                 ve(vi2,ee2), ev(ee2,vj2));
  PathIndex vev2Index = builder.buildSegmented(vev2, 0);
  ASSERT_EQ(vevIndex, vev2Index);

  // Check that a different vev get's a different index
  Var u("u");
  Var f("f");
  PathExpression uf = Link::make(u, f, Link::ve);
  PathExpression fu = Link::make(f, u, Link::ev);
  uf.bind(V,E);
  fu.bind(E,V);

  Var ui("ui");
  Var ff("f");
  Var uj("uj");
  PathExpression ufu = Or::make({ui,uj}, {{QuantifiedVar::Exist,ff}},
                                uf(ui, ff), fu(ff,uj));
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  fu.bind(F,U);
  PathIndex ufuIndex = builder.buildSegmented(ufu, 0);
  ASSERT_NE(vevIndex, ufuIndex);

  // Check that vev evaluated backwards get's a different index
  // TODO

  // Test vevev expression
  Var vk("vk");
  PathExpression vevev = Or::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                  vev(vi,vk), vev(vk, vj));
  PathIndex vevevIndex = builder.buildSegmented(vevev, 0);
  VERIFY_INDEX(vevevIndex, nbrs({{0,1,2,3,4}, {0,1,2,3,4}, {0,1,2,3,4},
                                 {0,1,2,3,4}, {0,1,2,3,4}}));

  // Test vevgv expressions: v   v-e-v-e-v   v-g-v
  vev = And::make({vi,vj}, {{QuantifiedVar::Exist,e}}, ve(vi, e), ev(e, vj));

  Var g("g");
  PathExpression vg = makeVE();
  PathExpression gv = makeEV();
  vg.bind(V,G);
  gv.bind(G,V);
  PathExpression vgv = And::make({vi,vj}, {{QuantifiedVar::Exist,g}},
                                 vg(vi, g), gv(g, vj));
  PathIndex vgvIndex = builder.buildSegmented(vgv, 0);
  PathExpression vevgv = Or::make({vi,vj}, {{QuantifiedVar::Exist,vk}},
                                  vev(vi,vk), vgv(vk, vj));
  PathIndex vevgvIndex = builder.buildSegmented(vevgv, 0);
  VERIFY_INDEX(vevgvIndex, nbrs({{3,4}, {0,1,2,3,4}, {0,1,2,3,4},
                                 {3,4}, {3,4}}));
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

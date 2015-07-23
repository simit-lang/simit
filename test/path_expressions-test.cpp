#include "gtest/gtest.h"
#include "path_expressions-test.h"

#include "path_expressions.h"
#include "graph.h"

using namespace std;
using namespace simit;
using namespace simit::pe;

#define CHECK_EQ(v1, v2)             \
do {                                 \
  ASSERT_EQ(v1, v2);                 \
  ASSERT_FALSE(v1 < v2 || v2 < v1);  \
} while (false)

#define CHECK_NE(v1, v2)             \
do {                                 \
  ASSERT_NE(v1, v2);                 \
  ASSERT_TRUE(v1 < v2 || v2 < v1);   \
} while (false)


TEST(PathExpression, Link) {
  Var e = Var("e");
  Var v = Var("v");
  PathExpression ev = Link::make(e, v, Link::ev);

  ASSERT_EQ(ev.getNumPathEndpoints(), 2u);
  ASSERT_EQ(ev.getPathEndpoint(0), e);
  ASSERT_EQ(ev.getPathEndpoint(1), v);
  CHECK_EQ(ev, ev);
  ASSERT_FALSE(ev.isBound());

  // Check that different links are equal (equal means that the variables of
  // both links are bound to the same sets)
  Var f = Var("f");
  Var u = Var("u");
  PathExpression fu = Link::make(f, u, Link::ve);
  CHECK_EQ(ev, fu);

  // Bind the same sets to ev and fu
  Set V;
  Set E(V,V);
  V.setName("V");
  E.setName("E");
  ev.bind({{v,V}, {e,E}});
  CHECK_EQ(ev, fu);
  fu.bind({{u,V}, {f,E}});
  ASSERT_TRUE(ev.isBound());
  ASSERT_TRUE(fu.isBound());
  CHECK_EQ(ev, fu);


  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  U.setName("U");
  F.setName("F");
  fu.bind({{u,U}, {f,F}});
  CHECK_NE(ev, fu);

  // Check that bound ev != bound ve
  PathExpression ve = Link::make(v, e, Link::ve);
  ve.bind({{v,V}, {e,E}});
  ASSERT_NE(ev, ve);
}


TEST(PathExpression, Renamed) {
  PathExpression ve = makeVE("v", "e");
  PathExpression rve1 = ve(Var("u"), Var("f"));
  PathExpression rve2 = ve(Var("w"), Var("g"));

  CHECK_EQ(rve1, rve2);
  CHECK_EQ(rve1, ve);
  CHECK_EQ(ve, rve1);
}


TEST(PathExpression, ExistAnd_vev) {
  Var v("v");
  Var e("e");
  PathExpression ve = Link::make(v, e, Link::ve);
  PathExpression ev = Link::make(e, v, Link::ev);

  Var vi("vi");
  Var ee("e");
  Var vj("vj");
  PathExpression vev = And::make({vi,vj}, {{QuantifiedVar::Exist,ee}},
                                 ve(vi,ee), ev(ee,vj));
  Var vk("vk");
  Var vl("vl");
  Var vm("vm");
  PathExpression vevev = And::make({vk,vl}, {{QuantifiedVar::Exist,vm}},
                                   vev(vk,vm), vev(vm,vl));
  ASSERT_EQ(vev.getPathEndpoint(0), vi);
  ASSERT_EQ(vev.getPathEndpoint(1), vj);
  CHECK_EQ(vev, vev);
  CHECK_NE(vev, ve);
  CHECK_NE(ve, vev);

  // Check that two different quantified ands are equal
  Var u("u");
  Var f("f");
  PathExpression uf = Link::make(u, f, Link::ve);
  PathExpression fu = Link::make(f, u, Link::ev);

  Var ui("ui");
  Var ff("f");
  Var uj("uj");
  PathExpression ufu = And::make({ui,uj}, {{QuantifiedVar::Exist,ff}},
                                 uf(ui,ff), fu(ff,uj));
  CHECK_EQ(vev, ufu);

  // Bind the same sets to vev and ufu
  Set V;
  Set E(V,V);
  ve.bind({{v,V}, {e,E}});
  ev.bind({{v,V}, {e,E}});
  CHECK_EQ(ufu, vev);
  uf.bind({{u,V}, {f,E}});
  fu.bind({{u,V}, {f,E}});
  ASSERT_TRUE(vev.isBound());
  ASSERT_TRUE(ufu.isBound());
  CHECK_EQ(vev, ufu);

  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  uf.bind({{u,U}, {f,F}});
  fu.bind({{u,U}, {f,F}});
  ASSERT_TRUE(ufu.isBound());
  CHECK_NE(vev, ufu);
//
//  // TODO: Test eve and compare eve with vev
//  // TODO: Test or and compare or and and expressions
}

#include "gtest/gtest.h"

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
  PathExpression bev = ev.bind({{v,V}, {e,E}});
  PathExpression bfu = fu.bind({{u,V}, {f,E}});
  ASSERT_TRUE(bev.isBound());
  ASSERT_TRUE(bfu.isBound());
  CHECK_EQ(bev, fu);
  CHECK_EQ(bev, bfu);

  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  U.setName("U");
  F.setName("F");
  bfu = fu.bind({{u,U}, {f,F}});
  CHECK_NE(bev, bfu);

  // Check that bound ev != bound ve
  PathExpression bve = Link::make(v, e, Link::ve).bind({{v,V}, {e,E}});
  ASSERT_NE(bev, bve);
}


TEST(PathExpression, ExistAnd_vev) {
  Var vi("vi");
  Var  e("e");
  Var vj("vj");
  PathExpression ve = Link::make(vi, e, Link::ve);
  PathExpression ev = Link::make(e, vj, Link::ev);
  PathExpression vev = QuantifiedAnd::make({vi,vj}, {{QVar::Exist,e}}, ve, ev);
  ASSERT_EQ(vev.getPathEndpoint(0), vi);
  ASSERT_EQ(vev.getPathEndpoint(1), vj);
  CHECK_EQ(vev, vev);

  Var ui("ui");
  Var  f("f");
  Var uj("uj");
  PathExpression uf = Link::make(ui, f, Link::ve);
  PathExpression fu = Link::make(f, uj, Link::ev);
  PathExpression ufu = QuantifiedAnd::make({ui,uj}, {{QVar::Exist,f}}, uf, fu);
  CHECK_EQ(vev, ufu);

  // Bind the same sets to vev and ufu
  Set V;
  Set E(V,V);
  PathExpression bvev = vev.bind({{vi,V}, {e,E}, {vj,V}});
  PathExpression bufu = ufu.bind({{ui,V}, {f,E}, {uj,V}});
  ASSERT_TRUE(bvev.isBound());
  ASSERT_TRUE(bufu.isBound());
  CHECK_EQ(ufu, bvev);
  CHECK_EQ(bvev, bufu);

  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  bufu = ufu.bind({{ui,U}, {f,F}, {uj,U}});
  ASSERT_TRUE(bufu.isBound());
  CHECK_NE(bvev, bufu);

  // TODO: Test eve and compare eve with vev
  // TODO: Test or and compare or and and expressions
}

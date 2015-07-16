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


TEST(PathExpression, EV) {
  Var e = Var("e");
  Var v = Var("v");
  PathExpression ev = EV::make(e, v);
  ASSERT_EQ(ev.getPathEndpoint(0), e);
  ASSERT_EQ(ev.getPathEndpoint(1), v);
  CHECK_EQ(ev, ev);
  ASSERT_FALSE(ev.isBound());

  // Check that two different EV are equal (equal means that if the variables of
  // both EV expressions are bound to the same sets, the resulting bound
  // expressions are equal)
  Var f = Var("f");
  Var u = Var("u");
  PathExpression fu = EV::make(f, u);
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
}


TEST(PathExpression, VE) {
  Var v = Var("v");
  Var e = Var("e");
  PathExpression ve = VE::make(v, e);
  ASSERT_EQ(ve.getPathEndpoint(0), v);
  ASSERT_EQ(ve.getPathEndpoint(1), e);
  CHECK_EQ(ve, ve);
  ASSERT_FALSE(ve.isBound());

  // Check that two different EV are equal (equal means that if the variables of
  // both EV expressions are bound to the same sets, the resulting bound
  // expressions are equal)
  Var u = Var("u");
  Var f = Var("f");
  PathExpression uf = VE::make(u, f);
  CHECK_EQ(ve, uf);

  // Bind the same sets to ev and fu
  Set V;
  Set E(V,V);
  V.setName("V");
  E.setName("E");
  PathExpression bve = ve.bind({{v,V}, {e,E}});
  PathExpression buf = uf.bind({{u,V}, {f,E}});
  ASSERT_TRUE(bve.isBound());
  ASSERT_TRUE(buf.isBound());
  CHECK_EQ(uf, bve);
  CHECK_EQ(bve, buf);

  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  U.setName("U");
  F.setName("F");
  buf = uf.bind({{u,U}, {f,F}});
  CHECK_NE(bve, buf);
}


TEST(PathExpression, And) {
  Var vi("vi");
  Var  e("e");
  Var vj("vj");
  PathExpression ve = VE::make(vi, e);
  PathExpression ev = EV::make(e, vj);
  QuantifiedVar q = QuantifiedVar(QuantifiedVar::Existential, e);
  PathExpression vev = And::make({vi,vj}, {q}, ve, ev);
  ASSERT_EQ(vev.getPathEndpoint(0), vi);
  ASSERT_EQ(vev.getPathEndpoint(1), vj);
  CHECK_EQ(vev, vev);

  Var ui("ui");
  Var  f("f");
  Var uj("uj");
  PathExpression uf = VE::make(ui, f);
  PathExpression fu = EV::make(f, uj);
  QuantifiedVar qf = QuantifiedVar(QuantifiedVar::Existential, f);
  PathExpression ufu = And::make({ui,uj}, {qf}, uf, fu);
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

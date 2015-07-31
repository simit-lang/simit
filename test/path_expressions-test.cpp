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
  ASSERT_FALSE(v1 < v2 || v1 > v2);  \
} while (false)

#define CHECK_NE(v1, v2)             \
do {                                 \
  ASSERT_NE(v1, v2);                 \
  ASSERT_TRUE(v1 < v2 || v1 > v2);   \
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
  ev.bind(E,V);
  CHECK_EQ(ev, fu);
  fu.bind(E,V);
  ASSERT_TRUE(ev.isBound());
  ASSERT_TRUE(fu.isBound());
  CHECK_EQ(ev, fu);


  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  U.setName("U");
  F.setName("F");
  fu.bind(F,U);
  CHECK_NE(ev, fu);

  // Check that bound ev != bound ve
  PathExpression ve = Link::make(v, e, Link::ve);
  ve.bind(V,E);
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


TEST(PathExpression, And) {
  Var v("v");
  Var e("e");
  PathExpression ve = makeVE();
  PathExpression veANDve = And::make({v,e}, {}, ve(v,e), ve(e,v));
  PathExpression veANDve2 = And::make({v,e}, {}, ve(v,e), ve(e,v));
  CHECK_EQ(veANDve, veANDve2);
  CHECK_NE(veANDve, ve);
  ASSERT_EQ(veANDve.getPathEndpoint(0), v);
  ASSERT_EQ(veANDve.getPathEndpoint(1), e);

  // Check that two different ors are equal
  Var u("u");
  Var f("f");
  PathExpression uf = makeVE();
  PathExpression ufANDuf = And::make({u,f}, {}, uf(u,f), uf(f,u));
  CHECK_EQ(veANDve, ufANDuf);

  // Bind the same sets to ev and uf
  Set V;
  Set E(V,V);
  ve.bind(V,E);
  CHECK_EQ(veANDve, ufANDuf);
  uf.bind(V,E);
  ASSERT_TRUE(veANDve.isBound());
  ASSERT_TRUE(ufANDuf.isBound());
  CHECK_EQ(veANDve, ufANDuf);

  // Bind different sets to ve and fu
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  ASSERT_TRUE(ufANDuf.isBound());
  CHECK_NE(veANDve, ufANDuf);
}


TEST(PathExpression, Or) {
  Var v("v");
  Var e("e");
  PathExpression ve = makeVE();
  PathExpression veORve = Or::make({v,e}, {}, ve(v,e), ve(e,v));
  PathExpression veORve2 = Or::make({v,e}, {}, ve(v,e), ve(e,v));
  CHECK_EQ(veORve, veORve2);
  CHECK_NE(veORve, ve);
  ASSERT_EQ(veORve.getPathEndpoint(0), v);
  ASSERT_EQ(veORve.getPathEndpoint(1), e);

  // Check that two different ors are equal
  Var u("u");
  Var f("f");
  PathExpression uf = makeVE();
  PathExpression ufORuf = Or::make({u,f}, {}, uf(u,f), uf(f,u));
  CHECK_EQ(veORve, ufORuf);

  // Bind the same sets to ev and uf
  Set V;
  Set E(V,V);
  ve.bind(V,E);
  CHECK_EQ(veORve, ufORuf);
  uf.bind(V,E);
  ASSERT_TRUE(veORve.isBound());
  ASSERT_TRUE(ufORuf.isBound());
  CHECK_EQ(veORve, ufORuf);

  // Bind different sets to ve and fu
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  ASSERT_TRUE(ufORuf.isBound());
  CHECK_NE(veORve, ufORuf);
}


TEST(PathExpression, ExistAnd) {
  PathExpression ve = makeVE();
  PathExpression ev = makeEV();

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

  PathExpression uf = makeVE("u", "f");
  PathExpression fu = makeEV("f", "u");

  Var ui("ui");
  Var f("f");
  Var uj("uj");
  PathExpression ufu = And::make({ui,uj}, {{QuantifiedVar::Exist,f}},
                                 uf(ui,f), fu(f,uj));
  CHECK_EQ(vev, ufu);

  // Bind the same sets to ev and uf
  Set V;
  Set E(V,V);
  ve.bind(V,E);
  ev.bind(E,V);
  CHECK_EQ(ufu, vev);
  uf.bind(V,E);
  fu.bind(E,V);
  ASSERT_TRUE(vev.isBound());
  ASSERT_TRUE(ufu.isBound());
  CHECK_EQ(vev, ufu);

  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  fu.bind(F,U);
  ASSERT_TRUE(ufu.isBound());
  CHECK_NE(vev, ufu);

  // TODO: Test eve and compare eve with vev
  // TODO: Test or and compare or and and expressions
}


TEST(PathExpression, ExistOr) {
  PathExpression ve = makeVE();
  PathExpression ev = makeEV();

  Var vi("vi");
  Var ee("e");
  Var vj("vj");
  PathExpression vev = Or::make({vi,vj}, {{QuantifiedVar::Exist,ee}},
                                ve(vi,ee), ev(ee,vj));
  Var vk("vk");
  Var vl("vl");
  Var vm("vm");
  PathExpression vevev = Or::make({vk,vl}, {{QuantifiedVar::Exist,vm}},
                                  vev(vk,vm), vev(vm,vl));
  ASSERT_EQ(vev.getPathEndpoint(0), vi);
  ASSERT_EQ(vev.getPathEndpoint(1), vj);
  CHECK_EQ(vev, vev);
  CHECK_NE(vev, ve);
  CHECK_NE(ve, vev);

  // Check that two different quantified ands are equal

  PathExpression uf = makeVE("u", "f");
  PathExpression fu = makeEV("f", "u");

  Var ui("ui");
  Var f("f");
  Var uj("uj");
  PathExpression ufu = Or::make({ui,uj}, {{QuantifiedVar::Exist,f}},
                                uf(ui,f), fu(f,uj));
  CHECK_EQ(vev, ufu);

  // Bind the same sets to ev and uf
  Set V;
  Set E(V,V);
  ve.bind(V,E);
  ev.bind(E,V);
  CHECK_EQ(ufu, vev);
  uf.bind(V,E);
  fu.bind(E,V);
  ASSERT_TRUE(vev.isBound());
  ASSERT_TRUE(ufu.isBound());
  CHECK_EQ(vev, ufu);

  // Bind different sets to ev and fu
  Set U;
  Set F(U,U);
  uf.bind(U,F);
  fu.bind(F,U);
  ASSERT_TRUE(ufu.isBound());
  CHECK_NE(vev, ufu);

  // TODO: Test eve and compare eve with vev
  // TODO: Test or and compare or and and expressions

}

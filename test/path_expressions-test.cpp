#include "gtest/gtest.h"
#include "path_expressions-test.h"

#include "path_expressions.h"

using namespace std;
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
  Var e = Var("e", Set("E"));
  Var v = Var("v", Set("V"));
  PathExpression ev = Link::make(e, v, Link::ev);

  ASSERT_EQ(ev.getNumPathEndpoints(), 2u);
  ASSERT_EQ(ev.getPathEndpoint(0), e);
  ASSERT_EQ(ev.getPathEndpoint(1), v);
  CHECK_EQ(ev, ev);

  // Check that links with the same vars are equal
  PathExpression ev2 = Link::make(e, v, Link::ev);
  CHECK_EQ(ev, ev2);

  // Check that links with different vars are not equal
  Var f = Var("f", Set("F"));
  Var u = Var("u", Set("U"));
  PathExpression fu = Link::make(f, u, Link::ve);
  CHECK_NE(ev, fu);

  // Check that links are different from an undefined pexprs
  CHECK_NE(ev, PathExpression());
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
  CHECK_NE(veANDve, ve);
  ASSERT_EQ(veANDve.getPathEndpoint(0), v);
  ASSERT_EQ(veANDve.getPathEndpoint(1), e);

  // Check that ands with the same vars are equal
  PathExpression veANDve2 = And::make({v,e}, {}, ve(v,e), ve(e,v));
  CHECK_EQ(veANDve, veANDve2);

  // Check that ands with different vars are not equal
  Var u("u");
  Var f("f");
  PathExpression uf = makeVE();
  PathExpression ufANDuf = And::make({u,f}, {}, uf(u,f), uf(f,u));
  CHECK_NE(veANDve, ufANDuf);
}

TEST(PathExpression, Or) {
  Var v("v");
  Var e("e");
  PathExpression ve = makeVE();
  PathExpression veORve = Or::make({v,e}, {}, ve(v,e), ve(e,v));
  CHECK_NE(veORve, ve);
  ASSERT_EQ(veORve.getPathEndpoint(0), v);
  ASSERT_EQ(veORve.getPathEndpoint(1), e);

  // Check that ors with the same vars are equal
  PathExpression veORve2 = Or::make({v,e}, {}, ve(v,e), ve(e,v));
  CHECK_EQ(veORve, veORve2);

  // Check that ors with different vars are not equal
  Var u("u");
  Var f("f");
  PathExpression uf = makeVE();
  PathExpression ufORuf = Or::make({u,f}, {}, uf(u,f), uf(f,u));
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

  // Check that two quantified ands with the same variable are equal
  PathExpression vevev2 = And::make({vk,vl}, {{QuantifiedVar::Exist,vm}},
                                    vev(vk,vm), vev(vm,vl));
  CHECK_EQ(vevev, vevev2);

  // Check that two quantified ands with different variables are not equal
  PathExpression uf = makeVE("u", "f");
  PathExpression fu = makeEV("f", "u");

  Var ui("ui");
  Var f("f");
  Var uj("uj");
  PathExpression ufu = And::make({ui,uj}, {{QuantifiedVar::Exist,f}},
                                 uf(ui,f), fu(f,uj));
  CHECK_NE(vev, ufu);

  // TODO: Test eve and compare eve with vev
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

  // Check that two quantified ors with the same variable are equal
  PathExpression vevev2 = Or::make({vk,vl}, {{QuantifiedVar::Exist,vm}},
                                    vev(vk,vm), vev(vm,vl));
  CHECK_EQ(vevev, vevev2);

  // Check that two quantified ors with different variables are not equal
  PathExpression uf = makeVE("u", "f");
  PathExpression fu = makeEV("f", "u");

  Var ui("ui");
  Var f("f");
  Var uj("uj");
  PathExpression ufu = Or::make({ui,uj}, {{QuantifiedVar::Exist,f}},
                                 uf(ui,f), fu(f,uj));
  CHECK_NE(vev, ufu);

  // TODO: Test eve and compare eve with vev
}

TEST(DISABLED_PathExpression, Permute) {
  Var v("v");
  Var e("e");
  Var u("u");

  PathExpression ve = makeVE();
  PathExpression ue = makeVE();
  PathExpression ev = makeEV();
  PathExpression eu = makeEV();

  PathExpression vev = And::make({v,v}, {{QuantifiedVar::Exist,e}},
                                 ve(v,e), ev(e,v));
  PathExpression veu = And::make({v,u}, {{QuantifiedVar::Exist,e}},
                                 ve(v,e), eu(e,u));
  PathExpression uev = And::make({u,v}, {{QuantifiedVar::Exist,e}},
                                 ue(u,e), ev(e,v));

  CHECK_NE(veu, vev);
  CHECK_NE(uev, vev);
  CHECK_NE(veu, uev);
  // Permute veu2 = uev
  PathExpression veu2 = uev; // TODO: Permute uev -> veu
  CHECK_EQ(veu, veu2);
}

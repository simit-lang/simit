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
  Set V;
  Set E(V,V);
  Box chain = createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  Var e("e");
  Var v("v");
  PathExpression ev = EV::make(e, v);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(ev, 0, {{e,&E}, {v,&V}});

  ASSERT_EQ(4u, index.numElements());
  ASSERT_EQ(4u*2, index.numNeighbors());

  vector<unsigned> expectedNumNbrs = {2, 2, 2, 2};
  vector<vector<unsigned>> expectedNbrs = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
  VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs);
}

TEST(PathIndex, VE) {
  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  Var v("v");
  Var e("e");
  PathExpression ve = VE::make(v, e);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(ve, 0, {{v,&V}, {e,&E}});

  ASSERT_EQ(5u, index.numElements());
  ASSERT_EQ(8u, index.numNeighbors());

  vector<unsigned> expectedNumNbrs = {1, 2, 2, 2, 1};
  vector<vector<unsigned>> expectedNbrs={{0}, {0, 1}, {1, 2}, {2, 3}, {3}};
  VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs);
}

TEST(PathIndex, VEV) {
  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);  // v-e-v-e-v

  Var vi("vi");
  Var  e("e");
  Var vj("vj");
  PathExpression ve = VE::make(vi, e);
  PathExpression ev = EV::make(e, vj);

  Formula::QVar quantifiedVar = Formula::QVar(Formula::QVar::Existential, e);
  PathExpression vev = And::make({vi,vj}, {quantifiedVar}, ve, ev);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(vev, 0, {{vi,&V}, {e,&E}, {vj,&V}});

  ASSERT_EQ(3u, index.numElements());
  ASSERT_EQ(7u, index.numNeighbors());

  vector<unsigned> expectedNumNbrs = {2, 3, 2};
  vector<vector<unsigned>> expectedNbrs={{0, 1}, {0, 1, 2}, {1, 2}};
  VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs);
}

TEST(PathIndex, Duplicates) {
  Set V;
  Set E(V,V);

  ElementRef v0 = V.add();
  ElementRef v1 = V.add();
  E.add(v0,v1);
  E.add(v1,v0);

  Var vi("vi");
  Var  e("e");
  Var vj("vj");
  PathExpression ve = VE::make(vi, e);
  PathExpression ev = EV::make(e, vj);

  Formula::QVar quantifiedVar = Formula::QVar(Formula::QVar::Existential, e);
  PathExpression vev = And::make({vi,vj}, {quantifiedVar}, ve, ev);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(vev, 0, {{vi,&V}, {e,&E}, {vj,&V}});

  ASSERT_EQ(2u, index.numElements());
  ASSERT_EQ(4u, index.numNeighbors());

  vector<unsigned> expectedNumNbrs = {2, 2};
  vector<vector<unsigned>> expectedNbrs={{0, 1}, {0, 1}};
  VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs);
}

TEST(PathIndex, Memoization) {
  Set V;
  Set E(V,V);
  Box box = createBox(&V, &E, 3, 1, 1);  // v-e-v-e-v

  PathIndexBuilder builder;

  Var vi("vi");
  Var  e("e");
  Var vj("vj");

  PathExpression ve = VE::make(vi, e);
  PathExpression ev = EV::make(e, vj);

  // PathIndex implement equality by identity, so the two indices are the same
  // iff the builder's memoization worked
  PathIndex evindex1 = builder.buildSegmented(ev, 0, {{vi,&V}, {e,&E}, {vj,&V}});
  PathIndex evindex2 = builder.buildSegmented(ev, 0, {{vi,&V}, {e,&E}, {vj,&V}});
  ASSERT_EQ(evindex1, evindex2);

  Formula::QVar quantifiedVar = Formula::QVar(Formula::QVar::Existential, e);
  PathExpression vev = And::make({vi,vj}, {quantifiedVar}, ve, ev);

  PathIndex index1 = builder.buildSegmented(vev, 0, {{vi,&V}, {e,&E}, {vj,&V}});
  PathIndex index2 = builder.buildSegmented(vev, 0, {{vi,&V}, {e,&E}, {vj,&V}});
  ASSERT_EQ(index1, index2);


  // PathExpression implements equality by value, so two indices created from
  // equivalent, but different path expressions should be the same
//  Var vi2("vi");
//  Var  e2("e");
//  Var vj2("vj");
//
//  PathExpression ve2 = VE::make(vi2, e2);
//  PathExpression ev2 = EV::make(e2, vj2);
//
//  Formula::QVar quantifiedVar2 = Formula::QVar(Formula::QVar::Existential, e2);
//  PathExpression vev2 = And::make({vi2,vj2}, {quantifiedVar2}, ve2, ev2);
//
//  PathIndex index3 = builder.buildSegmented(vev2,0,{{vi2,&V},{e2,&E},{vj2,&V}});
//  ASSERT_EQ(index1, index3);


  // PathIndices created in opposite directions over the same expressions should
  // not be the same
//  // TODO: Possible optimization is to discover symmetric path expressions, and
//  //       return the same path index when they are evaluated in both directions
//  PathIndex index4 = builder.buildSegmented(vev1, 1);
//  ASSERT_NEQ(index1, index3);
}

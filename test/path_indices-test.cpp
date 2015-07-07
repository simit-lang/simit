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

#define VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs) \
do {                                                                      \
  int i = 0;                                                              \
  for (auto &e : index) {                                                 \
    ASSERT_EQ(expectedNumNbrs[i], index.numNeighbors(e));                 \
    int j = 0;                                                            \
    for (auto &n : index.neighbors(e)) {                                  \
      ASSERT_EQ(expectedNbrs[i][j++], (unsigned)n.getIdent());            \
    }                                                                     \
    ++i;                                                                  \
  }                                                                       \
} while(0)

TEST(PathIndex, EV) {
  Set V;
  Set E(V,V);
  Box chain = createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  ElementVar e = ElementVar("E");
  ElementVar v = ElementVar("V");
  PathExpression ev = EV::make(e, v);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(ev, 0, {{e, E}, {v, V}});

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

  ElementVar v = ElementVar("V");
  ElementVar e = ElementVar("E");
  PathExpression ve = VE::make(v, e);

  PathIndexBuilder builder;
  PathIndex index = builder.buildSegmented(ve, 0, {{v, V}, {e, E}});

  ASSERT_EQ(5u, index.numElements());
  ASSERT_EQ(8u, index.numNeighbors());

  vector<unsigned> expectedNumNbrs = {1, 2, 2, 2, 1};
  vector<vector<unsigned>> expectedNbrs={{0}, {0, 1}, {1, 2}, {2, 3}, {3}};
//  VERIFY_INDEX(index, expectedNumNbrs, expectedNbrs);
}

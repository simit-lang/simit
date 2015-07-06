#include "gtest/gtest.h"

#include <map>
#include <set>
#include <iostream>

#include "graph.h"
#include "path_expressions.h"
#include "path_indices.h"

using namespace simit;
using namespace simit::pe;

TEST(PathIndex, EV) {
  Set V;
  Set E(V,V);
  Box chain = createBox(&V, &E, 5, 1, 1);  // v-e-v-e-v-e-v-e-v

  ElementVar e = ElementVar("E");
  ElementVar v = ElementVar("V");
  PathExpression ev = EV::make(e, v, 0);

  PathIndexBuilder builder;
  PathIndex index = builder.buildCSR(ev, 0, {{e, E}, {v, V}});

  std::cout << index << std::endl;

  ASSERT_EQ(4u, index.numElements());
  ASSERT_EQ(4u*2, index.numNeighbors());

  std::vector<int> expectedElements = {0, 1, 2, 3};
  std::vector<std::pair<int,int>> expectedNeighbors = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
  int i = 0;
  for (auto &e : index) {
    ASSERT_EQ(expectedElements[i++], e.getIdent());
    
  }
}

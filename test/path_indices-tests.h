#ifndef PATH_INDICES_TESTS_H
#define PATH_INDICES_TESTS_H

#include <vector>
#include "graph.h"

using namespace simit;

typedef std::vector<std::vector<unsigned>> nbrs;

#define VERIFY_INDEX(index, expectedNbrs)                                      \
do {                                                                           \
  auto expectedNeighbors = expectedNbrs;                                       \
  ASSERT_EQ(expectedNeighbors.size(), index.numElements());                    \
  unsigned i = 0;                                                              \
  unsigned int totalNbrs=0;                                                    \
  for (auto e : index) {                                                       \
    ASSERT_EQ(expectedNeighbors[i].size(), index.numNeighbors(e))              \
        << "element " << i << " has the wrong number of neighbors";            \
    unsigned j = 0;                                                            \
    for (auto n : index.neighbors(e)) {                                        \
      ASSERT_EQ(expectedNeighbors[i][j], n)                                    \
          << "expects neighbor " << j << " of element " << i                   \
          << " to be " << expectedNeighbors[i][j];                             \
      ++j;                                                                     \
    }                                                                          \
    totalNbrs += j;                                                            \
    ++i;                                                                       \
  }                                                                            \
  ASSERT_EQ(totalNbrs, index.numNeighbors());                                  \
} while(0)

///  -f-
/// v-e-v-e-v-f-v
///  -----f-----
inline void createTestGraph0(Set* V, Set* E, Set* F) {
  Box box = createBox(V, E, 3, 1, 1);
  ElementRef v3 = V->add();
  F->add(box(2,0,0), v3);
  F->add(box(0,0,0), v3);
  F->add(box(0,0,0), box(1,0,0));
}

/// v  v-e-v-e-v  v-f-v
inline void createTestGraph1(Set* V, Set* E, Set* F) {
  V->add();
  Box box = createBox(V, E, 2, 1, 1);
  ElementRef v3 = V->add();
  ElementRef v4 = V->add();
  F->add(v3, v4);
}

#endif

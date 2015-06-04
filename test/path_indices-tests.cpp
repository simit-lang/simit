#include "gtest/gtest.h"

#include "graph.h"
#include "path_expressions.h"
#include "path_indices.h"

using namespace std;
using namespace simit;
using namespace simit::pe;

TEST(PathIndex, EV) {
  simit::Set V;
  simit::Set E(V,V);
  V.setName("V");
  E.setName("E");

  Box chain = createBox(&V, &E, 5, 1, 1);

  PathExpression ev(new EV(&E, &V, 0));
//  cout << ev << endl;

  PathIndexBuilder piBuilder;
  PathIndex pi = piBuilder.build(ev, 0);

//  PathIndex::Sinks sit = pi.getSinks(chain(1,1,1));
//  for (auto &sink : sit) {
//  }
}

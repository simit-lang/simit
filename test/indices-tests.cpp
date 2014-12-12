#include "gtest/gtest.h"

#include "graph.h"
#include "indices.h"

using namespace std;
using namespace simit;
using namespace simit::internal;

TEST(VertexToEdgeEndpointIndex, chain) {
  Set<> points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();
  
  Set<2> edges(points, points);
  edges.add(p0, p1);
  edges.add(p0,p2);
  
  internal::VertexToEdgeEndpointIndex edgeindex(edges);

  ASSERT_EQ(edgeindex.getTotalEdges(), 2);
  ASSERT_EQ(edgeindex.getNumEdgesForElement(p0, 0), 2);
  ASSERT_EQ(edgeindex.getNumEdgesForElement(p0, 1), 0);
  ASSERT_EQ(edgeindex.getWhichEdgesForElement(p1, 1)[0], 0);
}

TEST(VertexToEdgeIndex, chain) {
  Set<> points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();
  
  Set<2> edges(points, points);
  edges.add(p0, p1);
  edges.add(p2,p0);
  
  internal::VertexToEdgeIndex edgeindex(edges);

  ASSERT_EQ(edgeindex.getTotalEdges(), 2);
  ASSERT_EQ(edgeindex.getNumEdgesForElement(p0, points), 2);
  ASSERT_EQ(edgeindex.getWhichEdgesForElement(p1, points)[0], 0);
}

TEST(NeighborIndex, triangles) {
  Set<> points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();
  auto p3 = points.add();
  
  Set<3> edges(points, points, points);
  edges.add(p0, p1, p2);
  edges.add(p1, p2, p3);
  
  internal::NeighborIndex nIndex(edges);
  ASSERT_EQ(nIndex.getNumNeighbors(p0), 3);
  ASSERT_EQ(nIndex.getNumNeighbors(p1), 4);
  ASSERT_EQ(nIndex.getNeighbors(p1)[0], 0);
}

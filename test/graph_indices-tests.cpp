#include "gtest/gtest.h"

#include "graph.h"
#include "graph_indices.h"

using namespace std;
using namespace simit;
using namespace simit::internal;

TEST(VertexToEdgeEndpointIndex, chain) {
  Set points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();
  
  Set edges(points, points);
  edges.add(p0, p1);
  edges.add(p0,p2);
  
  internal::VertexToEdgeEndpointIndex edgeindex(edges);

  ASSERT_EQ(edgeindex.getTotalEdges(), 2);
  ASSERT_EQ(2u, edgeindex.getWhichEdgesForElement(p0, 0).size());
  ASSERT_EQ(0u, edgeindex.getWhichEdgesForElement(p0, 1).size());
  ASSERT_TRUE(edgeindex.getWhichEdgesForElement(p1, 1).find(0) !=
              edgeindex.getWhichEdgesForElement(p1, 1).end());
}

TEST(VertexToEdgeIndex, chain) {
  Set points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();
  
  Set edges(points, points);
  edges.add(p0, p1);
  edges.add(p2,p0);
  
  internal::VertexToEdgeIndex edgeindex(edges);

  ASSERT_EQ(edgeindex.getTotalEdges(), 2);
  ASSERT_EQ(2u, edgeindex.getWhichEdgesForElement(p0, points).size());
  ASSERT_EQ(1u, edgeindex.getWhichEdgesForElement(p1, points).size());
  ASSERT_TRUE(edgeindex.getWhichEdgesForElement(p1, points).find(0)
              != edgeindex.getWhichEdgesForElement(p1, points).end());
}

TEST(NeighborIndex, chain) {
  Set points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();

  Set edges(points, points);
  edges.add(p0, p1);
  edges.add(p1, p2);

  internal::NeighborIndex nIndex(edges);
  ASSERT_EQ(2, nIndex.getNumNeighbors(p0));
  ASSERT_EQ(3, nIndex.getNumNeighbors(p1));
  ASSERT_EQ(nIndex.getNeighbors(p1)[0], 0);
}

TEST(NeighborIndex, triangles) {
  Set points;
  auto p0 = points.add();
  auto p1 = points.add();
  auto p2 = points.add();
  auto p3 = points.add();
  
  Set edges(points, points, points);
  edges.add(p0, p1, p2);
  edges.add(p1, p2, p3);
  
  internal::NeighborIndex nIndex(edges);
  ASSERT_EQ(nIndex.getNumNeighbors(p0), 3);
  ASSERT_EQ(nIndex.getNumNeighbors(p1), 4);
  ASSERT_EQ(nIndex.getNeighbors(p1)[0], 0);
}

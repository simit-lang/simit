#include "gtest/gtest.h"

#include <vector>

#include "types.h"

using namespace std;
using namespace simit::ir;
using namespace simit;

TEST(IndexSet, constructor) {
  ASSERT_EQ(IndexSet(4), 4);
  ASSERT_EQ(IndexSet(42), 42);
}

TEST(IndexSet, eq) {
  auto r0 = IndexSet(4);
  ASSERT_EQ(r0, r0);
  ASSERT_EQ(r0, IndexSet(4));
  ASSERT_NE(r0, IndexSet(8));
  // TODO: Add tests for SET, VARIABLE and combinations
}

TEST(IndexDomain, getSize) {
  vector<IndexSet> indices;
  indices.push_back(IndexSet(1));
  ASSERT_EQ(IndexDomain(indices).getSize(), 1u);

  indices.clear();
  indices.push_back(IndexSet(3));
  ASSERT_EQ(IndexDomain(indices).getSize(), 3u);

  indices.push_back(IndexSet(4));
  ASSERT_EQ(IndexDomain(indices).getSize(), 12u);
  // TODO: Add tests for SET, VARIABLE and combinations
}

TEST(IndexDomain, eq) {
  vector<IndexSet> indices0;
  vector<IndexSet> indices1;
  vector<IndexSet> indices2;

  indices0.push_back(IndexSet(1));
  indices1.push_back(IndexSet(1));
  indices2.push_back(IndexSet(2));

  ASSERT_EQ(IndexDomain(indices0), IndexDomain(indices1));
  ASSERT_NE(IndexDomain(indices0), IndexDomain(indices2));

  indices0.push_back(IndexSet(2));
  indices1.push_back(IndexSet(2));
  ASSERT_EQ(IndexDomain(indices0), IndexDomain(indices1));

  indices1.pop_back();
  indices1.push_back(IndexSet(3));
  ASSERT_NE(IndexDomain(indices0), IndexDomain(indices1));

  // TODO: Add tests for SET, VARIABLE and combinations
}

TEST(Type, getSize) {
  vector<IndexSet> indices0;
  vector<IndexSet> indices1;
  vector<IndexDomain> dimensions;

  indices0.push_back(IndexSet(2));
  indices0.push_back(IndexSet(3));
  dimensions.push_back(IndexDomain(indices0));

  indices1.push_back(IndexSet(5));
  indices1.push_back(IndexSet(7));
  dimensions.push_back(IndexDomain(indices1));

  ASSERT_EQ(TensorType(FLOAT, dimensions).getSize(), 210u);
  // TODO: Add tests for SET, VARIABLE and combinations
}

TEST(Type, eq) {
  vector<IndexSet> idxs0;
  vector<IndexSet> idxs1;
  vector<IndexDomain> dims0;
  vector<IndexDomain> dims1;

  idxs0.push_back(IndexSet(2));
  idxs0.push_back(IndexSet(3));
  dims0.push_back(IndexDomain(idxs0));
  dims1.push_back(IndexDomain(idxs0));
  ASSERT_EQ(TensorType(FLOAT, dims0), TensorType(FLOAT, dims1));
  ASSERT_NE(TensorType(FLOAT, dims0), TensorType(INT, dims1));

  idxs1.push_back(IndexSet(3));
  idxs1.push_back(IndexSet(2));
  dims1.push_back(idxs1);
  ASSERT_NE(TensorType(ComponentType::FLOAT, dims0), TensorType(ComponentType::FLOAT, dims1));
  // TODO: Add tests for SET, VARIABLE and combinations
}

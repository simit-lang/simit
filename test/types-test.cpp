#include "gtest/gtest.h"

#include <vector>

#include "types.h"

using namespace std;
using namespace simit::internal;

TEST(IndexSet, getSize) {
  ASSERT_EQ(IndexSet(4), 4);
  ASSERT_EQ(IndexSet(42), 42);
  // TODO: Add tests for SET and VARIABLE
}

TEST(IndexSet, eq) {
  auto r0 = IndexSet(4);
  ASSERT_EQ(r0, r0);
  ASSERT_EQ(r0, IndexSet(4));
  ASSERT_NE(r0, IndexSet(8));
  // TODO: Add tests for SET, VARIABLE and combinations
}

TEST(IndexSetProduct, size) {
  vector<IndexSet> indices;
  indices.push_back(IndexSet(1));
  ASSERT_EQ(IndexSetProduct(indices).getSize(), 1);

  indices.clear();
  indices.push_back(IndexSet(3));
  ASSERT_EQ(IndexSetProduct(indices).getSize(), 3);

  indices.push_back(IndexSet(4));
  ASSERT_EQ(IndexSetProduct(indices).getSize(), 12);
}

TEST(IndexSetProduct, eq) {
  vector<IndexSet> indices0;
  vector<IndexSet> indices1;
  vector<IndexSet> indices2;

  indices0.push_back(IndexSet(1));
  indices1.push_back(IndexSet(1));
  indices2.push_back(IndexSet(2));

  ASSERT_EQ(IndexSetProduct(indices0), IndexSetProduct(indices1));
  ASSERT_NE(IndexSetProduct(indices0), IndexSetProduct(indices2));

  indices0.push_back(IndexSet(2));
  indices1.push_back(IndexSet(2));
  ASSERT_EQ(IndexSetProduct(indices0), IndexSetProduct(indices1));

  indices1.pop_back();
  indices1.push_back(IndexSet(3));
  ASSERT_NE(IndexSetProduct(indices0), IndexSetProduct(indices1));

  // TODO: Add tests for SET, VARIABLE and combinations
}

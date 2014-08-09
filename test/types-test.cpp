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

TEST(IndexSetProduct, getSize) {
  vector<IndexSet> indices;
  indices.push_back(IndexSet(1));
  ASSERT_EQ(IndexSetProduct(indices).getSize(), 1);

  indices.clear();
  indices.push_back(IndexSet(3));
  ASSERT_EQ(IndexSetProduct(indices).getSize(), 3);

  indices.push_back(IndexSet(4));
  ASSERT_EQ(IndexSetProduct(indices).getSize(), 12);
  // TODO: Add tests for SET, VARIABLE and combinations
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

TEST(Type, getSize) {
  vector<IndexSet> indices0;
  vector<IndexSet> indices1;
  vector<IndexSetProduct> dimensions;

  indices0.push_back(IndexSet(2));
  indices0.push_back(IndexSet(3));
  dimensions.push_back(IndexSetProduct(indices0));

  indices1.push_back(IndexSet(5));
  indices1.push_back(IndexSet(7));
  dimensions.push_back(IndexSetProduct(indices1));

  ASSERT_EQ(Type(Type::FLOAT, dimensions).getSize(), 210);
  // TODO: Add tests for SET, VARIABLE and combinations
}

TEST(Type, eq) {
  vector<IndexSet> idxs0;
  vector<IndexSet> idxs1;
  vector<IndexSetProduct> dims0;
  vector<IndexSetProduct> dims1;

  idxs0.push_back(IndexSet(2));
  idxs0.push_back(IndexSet(3));
  dims0.push_back(IndexSetProduct(idxs0));
  dims1.push_back(IndexSetProduct(idxs0));
  ASSERT_EQ(Type(Type::FLOAT, dims0), Type(Type::FLOAT, dims1));
  ASSERT_NE(Type(Type::FLOAT, dims0), Type(Type::INT, dims1));

  idxs1.push_back(IndexSet(3));
  idxs1.push_back(IndexSet(2));
  dims1.push_back(idxs1);
  ASSERT_NE(Type(Type::FLOAT, dims0), Type(Type::FLOAT, dims1));
  // TODO: Add tests for SET, VARIABLE and combinations
}

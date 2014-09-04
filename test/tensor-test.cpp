#include "gtest/gtest.h"

#include "tensor.h"
using namespace simit;

TEST(Tensor, scalar) {
  Scalar<FLOAT> scalar;
  ASSERT_EQ(scalar.getOrder(), 0u);
}

TEST(Tensor, vector) {
  Vector<FLOAT> vector(3);
  ASSERT_EQ(vector.getOrder(), 1u);
  ASSERT_EQ(3u, vector.getDimension(0));
}

TEST(Tensor, matrix) {
  Matrix<FLOAT> matrix(3, 2);
  ASSERT_EQ(matrix.getOrder(), 2u);
  ASSERT_EQ(3u, matrix.getDimension(0));
  ASSERT_EQ(2u, matrix.getDimension(1));
}

TEST(Tensor, tensor3) {
  Tensor<FLOAT,3> tensor(3, 2, 2);
  ASSERT_EQ(tensor.getOrder(), 3u);
  ASSERT_EQ(3u, tensor.getDimension(0));
  ASSERT_EQ(2u, tensor.getDimension(1));
  ASSERT_EQ(2u, tensor.getDimension(2));
}

TEST(Tensor, tensor4) {
  Tensor<FLOAT,4> tensor(3, 2, 2, 2);
  ASSERT_EQ(tensor.getOrder(), 4u);
  ASSERT_EQ(3u, tensor.getDimension(0));
  ASSERT_EQ(2u, tensor.getDimension(1));
  ASSERT_EQ(2u, tensor.getDimension(2));
  ASSERT_EQ(2u, tensor.getDimension(3));

}

TEST(Tensor, tensor5) {
  Tensor<FLOAT,5> tensor(3, 2, 2, 2, 2);
  ASSERT_EQ(tensor.getOrder(), 5u);
  ASSERT_EQ(3u, tensor.getDimension(0));
  ASSERT_EQ(2u, tensor.getDimension(1));
  ASSERT_EQ(2u, tensor.getDimension(2));
  ASSERT_EQ(2u, tensor.getDimension(3));
  ASSERT_EQ(2u, tensor.getDimension(4));
}

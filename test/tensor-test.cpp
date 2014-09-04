#include "gtest/gtest.h"

#include "tensor.h"
using namespace simit;

TEST(Tensor, scalar) {
  Scalar<double> s(3.0);
  ASSERT_EQ(FLOAT, s.getType());
  ASSERT_EQ(s.getOrder(), 0u);
  ASSERT_EQ(3.0, s);
  s = 5.0;
  ASSERT_EQ(5.0, s);

  Int si(42);
  ASSERT_EQ(INT, si.getType());
  ASSERT_EQ(42, si);
  ASSERT_EQ(43, si = 43);

  Double sd(4.0);
  ASSERT_EQ(4.0, sd);
  ASSERT_EQ(4.0, sd = 4.0);
}

TEST(Tensor, vector) {
  Vector<double> vec(3);
  ASSERT_EQ(FLOAT, vec.getType());
  ASSERT_EQ(vec.getOrder(), 1u);
  ASSERT_EQ(3u, vec.getDimension(0));
  vec(0) = 3.0;
  vec(1) = 2.0;
  vec(2) = 7.0;
  ASSERT_EQ(3.0, vec(0));
  ASSERT_EQ(2.0, vec(1));
  ASSERT_EQ(7.0, vec(2));
}

TEST(Tensor, matrix) {
  Matrix<double> mat(3, 2);
  ASSERT_EQ(mat.getOrder(), 2u);
  ASSERT_EQ(3u, mat.getDimension(0));
  ASSERT_EQ(2u, mat.getDimension(1));
  mat(0,0) = 3.0;
  mat(2,1) = 2.0;
  mat(1,0) = 7.0;
  ASSERT_EQ(3.0, mat(0,0));
  ASSERT_EQ(2.0, mat(2,1));
  ASSERT_EQ(7.0, mat(1,0));
}

TEST(Tensor, tensor3) {
  Tensor<double,3> tensor(3, 2, 2);
  ASSERT_EQ(tensor.getOrder(), 3u);
  ASSERT_EQ(3u, tensor.getDimension(0));
  ASSERT_EQ(2u, tensor.getDimension(1));
  ASSERT_EQ(2u, tensor.getDimension(2));
  tensor(0,0,1) = 3.0;
  tensor(2,1,0) = 2.0;
  tensor(1,0,1) = 7.0;
  ASSERT_EQ(3.0, tensor(0,0,1));
  ASSERT_EQ(2.0, tensor(2,1,0));
  ASSERT_EQ(7.0, tensor(1,0,1));
}

//TEST(Tensor, tensor4) {
//  Tensor<double,4> tensor(3, 2, 2, 2);
//  ASSERT_EQ(FLOAT, tensor.getType());
//  ASSERT_EQ(tensor.getOrder(), 4u);
//  ASSERT_EQ(3u, tensor.getDimension(0));
//  ASSERT_EQ(2u, tensor.getDimension(1));
//  ASSERT_EQ(2u, tensor.getDimension(2));
//  ASSERT_EQ(2u, tensor.getDimension(3));
//  tensor(0,0,1,0) = 3.0;
//  tensor(2,1,0,0) = 2.0;
//  tensor(1,0,1,1) = 7.0;
//  ASSERT_EQ(3.0, tensor(0,0,1,0));
//  ASSERT_EQ(2.0, tensor(2,1,0,0));
//  ASSERT_EQ(7.0, tensor(1,0,1,1));
//}

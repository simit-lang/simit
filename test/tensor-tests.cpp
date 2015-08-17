#include "simit-test.h"

#include <vector>

#include "tensor.h"

using namespace std;
using namespace simit;

TEST(Tensor, Size) {
  DenseTensor<double> scalar;
  DenseTensor<double,1> vec1;
  DenseTensor<double,1,1> mat11;
  DenseTensor<double,1,1,1> ten111;
  ASSERT_EQ(1u, scalar.getSize());
  ASSERT_EQ(1u, vec1.getSize());
  ASSERT_EQ(1u, mat11.getSize());
  ASSERT_EQ(1u, ten111.getSize());

  DenseTensor<double,2> vec2;
  DenseTensor<double,3> vec3;
  ASSERT_EQ(2u, vec2.getSize());
  ASSERT_EQ(3u, vec3.getSize());

  DenseTensor<double,2,2> mat22;
  DenseTensor<double,2,3> mat23;
  DenseTensor<double,3,2> mat32;
  ASSERT_EQ(4u, mat22.getSize());
  ASSERT_EQ(6u, mat23.getSize());
  ASSERT_EQ(6u, mat32.getSize());

  DenseTensor<double,2,2,2> ten222;
  DenseTensor<double,2,2,3> ten223;
  DenseTensor<double,2,4,2> ten242;
  DenseTensor<double,5,2,2> ten522;
  ASSERT_EQ( 8u, ten222.getSize());
  ASSERT_EQ(12u, ten223.getSize());
  ASSERT_EQ(16u, ten242.getSize());
  ASSERT_EQ(20u, ten522.getSize());
}

TEST(Tensor, index) {
  DenseTensor<double,2,2> mat1 = {0.0, 1.0, 2.0, 0.0};
  DenseTensor<double,2,2> mat2;
  mat2(0,1) = 1.0;
  mat2(1,0) = 2.0;
  ASSERT_EQ(mat1, mat2);
}

TEST(Tensor, copy) {
  DenseTensor<double,2,2> mat1;
  mat1(0,1) = 1.0;

  auto mat2(mat1);
  mat2(1,0) = 2.0;

  auto mat3 = mat2;
  mat3(1,1) = 3.0;

  DenseTensor<double,2,2> expected1 = {0.0, 1.0, 0.0, 0.0};
  DenseTensor<double,2,2> expected2 = {0.0, 1.0, 2.0, 0.0};
  DenseTensor<double,2,2> expected3 = {0.0, 1.0, 2.0, 3.0};
  ASSERT_EQ(expected1, mat1);
  ASSERT_EQ(expected2, mat2);
  ASSERT_EQ(expected3, mat3);
}

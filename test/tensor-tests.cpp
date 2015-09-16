#include "simit-test.h"

#include <vector>

#include "tensor.h"

using namespace std;
using namespace simit;

TEST(Tensor, scalars) {
  Tensor<double> v1(1.0);
  ASSERT_EQ(1.0, v1);
  ASSERT_EQ(1.0, v1({}));
  ASSERT_EQ(v1, 1.0);
  ASSERT_NE(2.0, v1);
  ASSERT_NE(v1, 2.0);

  v1 = 2.0;
  ASSERT_EQ(2.0, v1);

  Tensor<double> v2 = 3.0;
  ASSERT_EQ(3.0, v2);

  // Test default values
  Double d;
  Tensor<float>  f;
  Int    i;
  Bool   b;
  ASSERT_EQ(0.0,  d);
  ASSERT_EQ(0.0f, f);
  ASSERT_EQ(0,    i);
  ASSERT_EQ(Tensor<bool>(false), b);

  d = 4.0;
  f = 5.0;
  i = 6;
  b = true;
  ASSERT_EQ(4.0,  d);
  ASSERT_EQ(5.0f, f);
  ASSERT_EQ(6,    i);
  ASSERT_EQ(Tensor<bool>(true), b);
}

TEST(Tensor, index) {
  Tensor<double,2,2> mat1 = {0.0, 1.0, 2.0, 0.0};
  Tensor<double,2,2> mat2;
  mat2(0,1) = 1.0;
  mat2(1,0) = 2.0;
  ASSERT_EQ(mat1, mat2);

  Tensor<double,2,3,2> ten1 = {0.0, 0.0,  0.0, 0.0,  0.0, 1.0,
                                    0.0, 0.0,  2.0, 0.0,  0.0, 0.0};
  Tensor<double,2,3,2> ten2;
  ten2(0,2,1) = 1.0;
  ten2(1,1,0) = 2.0;
  ASSERT_EQ(ten1, ten2);
}

TEST(Tensor, order) {
  Tensor<double> scalard;
  ASSERT_EQ(0u, scalard.getOrder());

  Tensor<float> scalarf;
  ASSERT_EQ(0u, scalarf.getOrder());

  Tensor<double,3> vec3d;
  ASSERT_EQ(1u, vec3d.getOrder());

  Tensor<float,3> vec3f;
  ASSERT_EQ(1u, vec3f.getOrder());

  Tensor<double,2> vec2d;
  ASSERT_EQ(1u, vec2d.getOrder());

  Tensor<double,2,2> mat22d;
  ASSERT_EQ(2u, mat22d.getOrder());

  Tensor<double,2,3> mat23d;
  ASSERT_EQ(2u, mat23d.getOrder());
}

TEST(Tensor, Size) {
  Tensor<double> scalar;
  Tensor<double,1> vec1;
  Tensor<double,1,1> mat11;
  Tensor<double,1,1,1> ten111;
  ASSERT_EQ(1u, scalar.getSize());
  ASSERT_EQ(1u, vec1.getSize());
  ASSERT_EQ(1u, mat11.getSize());
  ASSERT_EQ(1u, ten111.getSize());

  Tensor<double,2> vec2;
  Tensor<double,3> vec3;
  ASSERT_EQ(2u, vec2.getSize());
  ASSERT_EQ(3u, vec3.getSize());

  Tensor<double,2,2> mat22;
  Tensor<double,2,3> mat23;
  Tensor<double,3,2> mat32;
  ASSERT_EQ(4u, mat22.getSize());
  ASSERT_EQ(6u, mat23.getSize());
  ASSERT_EQ(6u, mat32.getSize());

  Tensor<double,2,2,2> ten222;
  Tensor<double,2,2,3> ten223;
  Tensor<double,2,4,2> ten242;
  Tensor<double,5,2,2> ten522;
  ASSERT_EQ( 8u, ten222.getSize());
  ASSERT_EQ(12u, ten223.getSize());
  ASSERT_EQ(16u, ten242.getSize());
  ASSERT_EQ(20u, ten522.getSize());
}

TEST(Tensor, copy) {
  Tensor<double,2,2> mat1;
  mat1(0,1) = 1.0;

  auto mat2(mat1);
  mat2(1,0) = 2.0;

  auto mat3 = mat2;
  mat3(1,1) = 3.0;

  Tensor<double,2,2> expected1 = {0.0, 1.0, 0.0, 0.0};
  Tensor<double,2,2> expected2 = {0.0, 1.0, 2.0, 0.0};
  Tensor<double,2,2> expected3 = {0.0, 1.0, 2.0, 3.0};
  ASSERT_EQ(expected1, mat1);
  ASSERT_EQ(expected2, mat2);
  ASSERT_EQ(expected3, mat3);
}

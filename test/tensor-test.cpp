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
  Vector<double,2> vec2Ref({10.0, 20.0});
  Vector<double,3> vec3Ref({10.0, 20.0, 30.0});
  Vector<double,4> vec4Ref({10.0, 20.0, 30.0, 40.0});

  Vector<double,Dynamic> dvec2Ref(2, {10.0, 20.0});
  Vector<double,Dynamic> dvec3Ref(3, {10.0, 20.0, 30.0});
  Vector<double,Dynamic> dvec4Ref(4, {10.0, 20.0, 30.0, 40.0});

  Vector<double,3> vec3({10.0, 20.0, 30.0});
  ASSERT_EQ(FLOAT, vec3.getType());
  ASSERT_EQ(vec3.getOrder(), 1u);
  ASSERT_EQ(3u, vec3.getDimension(0));
  ASSERT_EQ(10.0, vec3(0));
  ASSERT_EQ(20.0, vec3(1));
  ASSERT_EQ(30.0, vec3(2));
  ASSERT_EQ(vec3Ref, vec3);
  ASSERT_EQ(dvec3Ref, vec3);
  ASSERT_NE(dvec2Ref, vec3);
  ASSERT_NE(vec4Ref, vec3);
  vec3(0) = 3.0;
  vec3(1) = 2.0;
  vec3(2) = 7.0;
  ASSERT_EQ(3.0, vec3(0));
  ASSERT_EQ(2.0, vec3(1));
  ASSERT_EQ(7.0, vec3(2));
  ASSERT_NE(vec3, vec3Ref);

  Vector<double,Dynamic> dvec3(3, {10.0, 20.0, 30.0});
  ASSERT_EQ(FLOAT, dvec3.getType());
  ASSERT_EQ(dvec3.getOrder(), 1u);
  ASSERT_EQ(3u, dvec3.getDimension(0));
  ASSERT_EQ(10.0, dvec3(0));
  ASSERT_EQ(20.0, dvec3(1));
  ASSERT_EQ(30.0, dvec3(2));
  ASSERT_EQ(dvec3Ref, dvec3);
  ASSERT_NE(dvec4Ref, dvec3);
  dvec3(0) = 3.0;
  dvec3(1) = 2.0;
  dvec3(2) = 7.0;
  ASSERT_EQ(3.0, dvec3(0));
  ASSERT_EQ(2.0, dvec3(1));
  ASSERT_EQ(7.0, dvec3(2));
  ASSERT_NE(dvec3, dvec3Ref);

  Tensor<double,1> tvec3(3, {10.0, 20.0, 30.0});
  ASSERT_EQ(FLOAT, tvec3.getType());
  ASSERT_EQ(tvec3.getOrder(), 1u);
  ASSERT_EQ(3u, tvec3.getDimension(0));
  ASSERT_EQ(10.0, tvec3(0));
  ASSERT_EQ(20.0, tvec3(1));
  ASSERT_EQ(30.0, tvec3(2));
  ASSERT_EQ(vec3Ref, tvec3);
  ASSERT_EQ(dvec3Ref, tvec3);
  ASSERT_NE(vec4Ref, tvec3);
  ASSERT_NE(dvec4Ref, tvec3);
  tvec3(0) = 3.0;
  tvec3(1) = 2.0;
  tvec3(2) = 7.0;
  ASSERT_EQ(3.0, tvec3(0));
  ASSERT_EQ(2.0, tvec3(1));
  ASSERT_EQ(7.0, tvec3(2));
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

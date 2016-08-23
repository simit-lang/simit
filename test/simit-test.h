#ifndef SIMIT_TEST_H
#define SIMIT_TEST_H

#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <algorithm>

#include "function.h"
#include "backend/backend.h"
#include "error.h"

namespace simit {
namespace backend {
class Backend;
}}

#ifdef F32
typedef float simit_float;
#else
typedef double simit_float;
#endif

void printTimes();

inline std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

#define TEST_FILE_NAME std::string(TEST_INPUT_DIR) + "/" +   \
                       toLower(test_info_->test_case_name()) + "/" +  \
                       test_info_->name() + ".sim"

// Reduce precision of asserts/expects when using floats
#ifdef F32
#define SIMIT_EXPECT_FLOAT_EQ(a, b) EXPECT_NEAR(a, b, 0.00001)
#else
#define SIMIT_EXPECT_FLOAT_EQ(a, b) EXPECT_DOUBLE_EQ(a, b)
#endif

#ifdef F32
#define SIMIT_ASSERT_FLOAT_EQ(a, b) ASSERT_NEAR(a, b, 0.0001)
#else
#define SIMIT_ASSERT_FLOAT_EQ(a, b) ASSERT_NEAR(a, b, 0.0000000000001)
#endif

#define SIMIT_ASSERT_FLOAT_NEAR_EQ(a, b) ASSERT_NEAR(a, b, 0.00001)

std::unique_ptr<simit::backend::Backend> getTestBackend();

simit::Function loadFunction(std::string fileName, std::string funcName="main");
simit::Function loadFunctionWithTimers(std::string fileName, std::string 
    funcName="main");

#define Vec3f TensorType::make(ScalarType::Float, {IndexDomain(3)})

#define Mat3f TensorType::make(ScalarType::Float, \
                               {IndexDomain(3),IndexDomain(3)})

#define Vec3i TensorType::make(ScalarType::Int, {IndexDomain(3)})

#define Mat3i TensorType::make(ScalarType::Int, \
                               {IndexDomain(3),IndexDomain(3)})


#endif

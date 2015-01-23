#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <algorithm>

#include "graph.h"
#include "program.h"
#include "error.h"
#include "ir.h"

#ifdef F32
typedef float simit_float;
#else
typedef double simit_float;
#endif

inline std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

#define TEST_FILE_NAME std::string(TEST_INPUT_DIR) + "/" +   \
                       toLower(test_info_->test_case_name()) + "/" +  \
                       test_info_->name() + ".sim"

// Reduce precision of asserts when using floats
#ifdef F32
#define ASSERT_SIMIT_FLOAT_EQ(a, b) ASSERT_NEAR(a, b, 0.00001)
#else
#define ASSERT_SIMIT_FLOAT_EQ(a, b) ASSERT_DOUBLE_EQ(a, b)
#endif

inline simit::Function getFunction(std::string fileName,
                                   std::string functionName="main") {
  simit::Program program;
  int errorCode = program.loadFile(fileName);
  if (errorCode) {
    std::cerr << program.getDiagnostics().getMessage();
    return simit::Function();
  }

  simit::Function f = program.compile(functionName);
  if (!f.defined()) {
    std::cerr << program.getDiagnostics().getMessage();
  }

  return f;
}

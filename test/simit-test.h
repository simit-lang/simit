#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <algorithm>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"
#include "ir.h"

inline std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

#define TEST_FILE_NAME std::string(TEST_INPUT_DIR) + "/" +   \
                       toLower(test_info_->test_case_name()) + "/" +  \
                       test_info_->name() + ".sim"

#define GPU_TEST_FILE_NAME(test_case) \
  std::string(TEST_INPUT_DIR) + "/" + \
  toLower(test_case) + "/" +          \
  test_info_->name() + ".sim"

inline
std::unique_ptr<simit::Function> getFunction(std::string fileName,
                                             std::string functionName="main",
                                             int floatSize=8) {
  simit::Program program;
  int errorCode = program.loadFile(fileName);
  if (errorCode) {
    std::cerr << program.getDiagnostics().getMessage();
    return nullptr;
  }

  std::unique_ptr<simit::Function> f =
      program.compile(functionName, floatSize);
  if (errorCode) {
    std::cerr << program.getDiagnostics().getMessage();
    return nullptr;
  }

  return f;
}

// F32 version of test
class F32Test : public ::testing::Test {
protected:
  int oldSize;
  virtual void SetUp() {
    oldSize = simit::ir::ScalarType::floatBytes;
    simit::ir::ScalarType::floatBytes = sizeof(float);
  }
  virtual void TearDown() {
    simit::ir::ScalarType::floatBytes = oldSize;
  }
};

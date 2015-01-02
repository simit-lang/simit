#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <algorithm>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "error.h"

inline std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

#define TEST_FILE_NAME std::string(TEST_INPUT_DIR) + "/" +   \
                       toLower(test_info_->test_case_name()) + "/" +  \
                       test_info_->name() + ".sim"

inline
std::unique_ptr<simit::Function> getFunction(std::string fileName,
                                             std::string functionName="main") {
  simit::Program program;
  int errorCode = program.loadFile(fileName);
  if (errorCode) {
    std::cerr << program.getDiagnostics().getMessage();
    return nullptr;
  }

  std::unique_ptr<simit::Function> f =
      program.compile(functionName, sizeof(simit_float));
  if (errorCode) {
    std::cerr << program.getDiagnostics().getMessage();
    return nullptr;
  }

  return f;
}

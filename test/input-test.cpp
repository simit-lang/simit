#include "gtest/gtest.h"

#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <fstream>

#include "program.h"
#include "error.h"
#include "util.h"
#include "frontend.h"
#include "llvm_backend.h"

using namespace simit;
using namespace simit::util;
using namespace testing;
using namespace std;

// Test each file in the input folder
class ProgramTestParam {
public:
  string name;
  string path;
  string source;
  unsigned int line;

  bool failedIO = false;

  operator std::string() const {
    return name + path + ":" + to_string(line);
  };

  friend ostream &operator<<(ostream &out, const ProgramTestParam &param) {
    return out << string(param);
  }
};

vector<ProgramTestParam> readTestsFromFile(const std::string &dirpath,
                                           const std::string &filename) {
  string filepath = dirpath + "/" + filename;
  std::string source;
  loadText(filepath, &source);

  vector<ProgramTestParam> testParams;
  // Split the source into one source code for each program

  const string testDelim = "%%%";
  std::vector<std::string> programs;
  if (source.substr(0, testDelim.size()) == testDelim) {
    programs = split(source, testDelim, true);
  }
  else {
    programs.push_back(source);
  }

  string testSuiteName = filename;
  testSuiteName.erase(testSuiteName.end()-4, testSuiteName.end());

  unsigned int line = 1;
  for (uint i=0; i<programs.size(); i++) {
    ProgramTestParam testParam;
    testParam.path = filepath;
    testParam.line = line;

    std::istringstream ss(programs[i]);
    string header;
    if (!std::getline(ss, header)) {
      testParam.name = filename;
      testParam.failedIO = true;
      testParams.push_back(testParam);
      continue;
    }

    if (header.substr(0, testDelim.size()) == testDelim) {
      header.erase(header.begin(), header.begin()+3);
      if (header[0] == '-') {
        continue;
      }
      testParam.name = testSuiteName + "::" + trim(header) + "  ";
    }
    else {
      testParam.name = testSuiteName + "  ";
    }

    ss.str(programs[i]);
    ss.clear();

    testParam.source = string((std::istreambuf_iterator<char>(ss)),
                              (std::istreambuf_iterator<char>()));
    testParams.push_back(testParam);

    line += std::count(testParam.source.begin(), testParam.source.end(), '\n');
  }
  return testParams;
}

vector<ProgramTestParam> readTestsFromDir(const std::string &dirpath) {
  vector<ProgramTestParam> testParams;
  DIR *dir;
  struct dirent *ent;
  struct stat st;
  dir = opendir(dirpath.c_str());
  while ((ent = readdir(dir)) != NULL) {
    const string filename = ent->d_name;
    const string fpath = dirpath + "/" + filename;
    if (filename[0] == '.') {
      continue;
    }
    if (stat(fpath.c_str(), &st) == -1) {
      continue;
    }
    if ((st.st_mode & S_IFDIR) != 0) {
      continue;
    }

    auto fileTestParams = readTestsFromFile(dirpath, filename);
    testParams.insert(testParams.end(),
                      fileTestParams.begin(), fileTestParams.end());
  }
  closedir(dir);
  return testParams;
}

void runTest(ProgramTestParam param) {
  ASSERT_FALSE(param.failedIO) << "failed to read file " + param.path;

  Program program;
  if (program.loadString(param.source) != 0) {
    assert(program.hasErrors());
    for (auto &diag : program.getDiagnostics()) {
      ADD_FAILURE() << diag.getMessage();

        // TODO: Add back line info
//      string errorFile = param.path;
//      unsigned int errorLine = param.line + error.getFirstLine() - 1;
//      ADD_FAILURE_AT(errorFile.c_str(), errorLine) << error.getMessage();
    }
    FAIL();
    return;
  }

  if (program.verify() != 0) {
    for (auto &diag : program.getDiagnostics()) {
      ADD_FAILURE() << diag.getMessage();

      // TODO: Add back line info
//      string errorFile = param.path;
//      unsigned int errorLine = param.line + error.getFirstLine() - 1;
//      ADD_FAILURE_AT(errorFile.c_str(), errorLine) << error.getMessage();
    }
    FAIL();
    return;
  }
}

template <typename T>
class TestWithParamF32 : public TestWithParam<T> {
protected:
  int oldBytes;
  virtual void SetUp() {
    oldBytes = ir::ScalarType::floatBytes;
    ir::ScalarType::floatBytes = sizeof(float);
  }
  virtual void TearDown() {
    ir::ScalarType::floatBytes = oldBytes;
  }
};

#define SIM_TEST_SUITE(name) \
  class name : public TestWithParam<ProgramTestParam> {}; \
  TEST_P(name, inputs) {                                  \
    runTest(GetParam());                                  \
  }

#define SIM_TEST_SUITE_F32(name) \
  class name : public TestWithParamF32<ProgramTestParam> {}; \
  TEST_P(name, inputs) {                                     \
    runTest(GetParam());                                     \
  }

#define SIM_TEST(suite, name)                                                \
  std::string path();                                                        \
  INSTANTIATE_TEST_CASE_P(name, suite,                                       \
                          ValuesIn(readTestsFromFile(string(TEST_INPUT_DIR)+ \
                                                       "/"+string(#suite),   \
                                                     string(#name)+".sim")))

#define SIM_TEST_F32(suite, suite_dir, name)                                 \
  std::string path();                                                        \
  INSTANTIATE_TEST_CASE_P(name, suite,                                       \
                          ValuesIn(readTestsFromFile(string(TEST_INPUT_DIR)+ \
                                                     "/"+string(#suite_dir), \
                                                     string(#name)+".sim")))

/* Examples */
//INSTANTIATE_TEST_CASE_P(examples, input,
//                        testing::ValuesIn(readTestsFromDir(EXAMPLES_DIR)));


/* Tests */
SIM_TEST_SUITE(elements)
SIM_TEST(elements, blas0);
SIM_TEST(elements, blas1);
SIM_TEST(elements, blas2);
SIM_TEST(elements, blas3);
SIM_TEST(elements, la);
SIM_TEST(elements, intrinsics);
SIM_TEST(elements, tensors);
SIM_TEST(elements, slice);
SIM_TEST(elements, index_notation);
SIM_TEST(elements, springs);
SIM_TEST(elements, fem);
SIM_TEST(elements, fem_tensor);

/* Float-32 tests */
SIM_TEST_SUITE_F32(elements_f32)
SIM_TEST_F32(elements_f32, elements, intrinsics);

SIM_TEST_SUITE(declarations)
SIM_TEST(declarations, function_headers);
SIM_TEST(declarations, objects);
SIM_TEST(declarations, variables);
SIM_TEST(declarations, misc);

SIM_TEST_SUITE(controlflow)
SIM_TEST(controlflow, map);
SIM_TEST(controlflow, calls);
SIM_TEST(controlflow, if_stmt);
SIM_TEST(controlflow, loops);
SIM_TEST(controlflow, boolean);

#include "gtest/gtest.h"

#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <fstream>

#include "program.h"
#include "errors.h"
#include "logger.h"
#include "util.h"

using namespace simit;
using namespace util;
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

class simfile : public TestWithParam<ProgramTestParam> {};

std::vector<std::string> split(const string &str, const string &delim,
                               bool keepDelim = false) {
  std::vector<std::string> results;
  size_t prev = 0;
  size_t next = 0;

  while ((next = str.find(delim, prev)) != std::string::npos) {
    if (next - prev != 0) {
      string substr = ((keepDelim) ? delim : "") + str.substr(prev, next-prev);
      results.push_back(substr);
    }
    prev = next + delim.size();
  }

  if (prev < str.size()) {
    string substr = ((keepDelim) ? delim : "") + str.substr(prev);
    results.push_back(substr);
  }

  return results;
}

std::string trim(const string &str, const string &whitespace = " \t\n") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

std::string loadText(const std::string &filepath) {
  ifstream ifs(filepath);
  return string((std::istreambuf_iterator<char>(ifs)),
                (std::istreambuf_iterator<char>()));
}

vector<ProgramTestParam> readTestsFromFile(const std::string &dirpath,
                                      const std::string &filename) {
  string filepath = dirpath + "/" + filename;
  string source = loadText(filepath);

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

TEST_P(simfile, check) {
  ASSERT_FALSE(GetParam().failedIO) << "failed to read file " + GetParam().path;
  Program program;
  if (program.loadString(GetParam().source) != 0) {
    for (auto &error : program.getErrors()) {
      string errorFile = GetParam().path;
      unsigned int errorLine = GetParam().line + error.getFirstLine() - 1;
      ADD_FAILURE_AT(errorFile.c_str(), errorLine) << program.getErrorString();
    }
    FAIL();
  }

  if (program.compile() != 0) {
    for (auto &error : program.getErrors()) {
      string errorFile = GetParam().path;
      unsigned int errorLine = GetParam().line + error.getFirstLine() - 1;
      ADD_FAILURE_AT(errorFile.c_str(), errorLine) << program.getErrorString();
    }
    FAIL();
  }
}

#define TEST_SIMTEST_FILE(path, name)                                         \
  INSTANTIATE_TEST_CASE_P(name, simfile,                                      \
                          ValuesIn(readTestsFromFile(path,                    \
                                                     string(#name)+".sim")));


/* Examples */
INSTANTIATE_TEST_CASE_P(examples, simfile,
                        testing::ValuesIn(readTestsFromDir(EXAMPLES_DIR)));


/* Tests */
TEST_SIMTEST_FILE(TEST_INPUT_DIR, blas0);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, blas1);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, blas2);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, blas3);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, index_notation);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, function_headers);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, objects);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, variables);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, map);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, controlflow);
TEST_SIMTEST_FILE(TEST_INPUT_DIR, misc);

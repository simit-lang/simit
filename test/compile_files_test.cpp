#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include "Program.h"
#include "Logger.h"
#include <dirent.h>
#include <errno.h>
using namespace Simit;
using namespace std;

// Test each file in the input folder
typedef std::pair<std::string, std::string> TestParam;
class ProgramFileTest : public testing::TestWithParam<TestParam> {};
vector<TestParam> readTestsFromDisk(const std::string &dirname) {
  vector<TestParam> testParams;
  DIR *dir;
  struct dirent *ent;
  struct stat st;
  dir = opendir(dirname.c_str());
  while ((ent = readdir(dir)) != NULL) {
    const string fname = ent->d_name;
    const string fpath = dirname + "/" + fname;
    if (stat(fpath.c_str(), &st) == -1) {
      continue;
    }
    if ((st.st_mode & S_IFDIR) != 0) {
      continue;
    }
    testParams.push_back(TestParam(fname, fpath));
  }
  closedir(dir);
  return testParams;
}

TEST_P(ProgramFileTest, inputFiles) {
  std::string filename = GetParam().first;
  std::string filepath = GetParam().second;
  log("ProgramFileTest::" + filename);
  logIndent();
  log(filename);
  Program program;
  std::string errors;
  EXPECT_EQ(0, program.loadFile(filepath, errors));
  log();
  logDedent();
}

INSTANTIATE_TEST_CASE_P(, ProgramFileTest,
                        testing::ValuesIn(readTestsFromDisk(TEST_INPUT_DIR)));

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
class ProgramTest {
public:
  string name;
  string source;

  operator std::string() const { return name; };
  std::ostream& operator<<(std::ostream& os) {return os<<std::string(*this);}
};

class ProgramFileTest : public testing::TestWithParam<ProgramTest> {};

std::vector<std::string> split(const string &str, const string &delim) {
  std::vector<std::string> results;
  size_t prev = 0;
  size_t next = 0;

  while ((next = str.find(delim, prev)) != std::string::npos) {
    if (next - prev != 0) {
      results.push_back(str.substr(prev, next - prev));
    }
    prev = next + 3;
  }

  if (prev < str.size()) {
    results.push_back(str.substr(prev));
  }

  return results;
}

std::string trim(const string &str, const string &whitespace = " \t")
{
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

vector<ProgramTest> readTestsFromDisk(const std::string &dirname) {
  vector<ProgramTest> testParams;
  DIR *dir;
  struct dirent *ent;
  struct stat st;
  dir = opendir(dirname.c_str());
  while ((ent = readdir(dir)) != NULL) {
    const string fname = ent->d_name;
    const string fpath = dirname + "/" + fname;
    if (fname[0] == '.') {
      continue;
    }
    if (stat(fpath.c_str(), &st) == -1) {
      continue;
    }
    if ((st.st_mode & S_IFDIR) != 0) {
      continue;
    }

    std::ifstream ifs(fpath);
    std::string source((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()   ) );

    // Split the source into one source code for each program
    std::vector<std::string> programs = split(source, "%%%");
    for (uint i=0; i<programs.size(); i++) {
      std::istringstream ss(programs[i]);
      string header;
      if(!std::getline(ss, header)) {
        continue;
      }
      ProgramTest source;
      source.name = fname + "::" + trim(header);
      source.source = string((std::istreambuf_iterator<char>(ss)),
                             (std::istreambuf_iterator<char>()));
      testParams.push_back(source);
    }
  }
  closedir(dir);
  return testParams;
}

TEST_P(ProgramFileTest, inputFiles) {
  log(GetParam().name);
  logIndent();
  Program program;
  std::string errors;
  EXPECT_EQ(0, program.loadString(GetParam().source, errors));
  log();
  logDedent();
}

INSTANTIATE_TEST_CASE_P(, ProgramFileTest,
                        testing::ValuesIn(readTestsFromDisk(TEST_INPUT_DIR)));

#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include "Program.h"
#include "Logger.h"
#include <dirent.h>
#include <errno.h>
using namespace Simit;
using namespace std;

string indent(string str, int num) {
  std::istringstream ss(str);
  string indent(num*2, ' ');
  string strIndented;
  string line;

  while(std::getline(ss, line)) {
    strIndented += indent + line + "\n";
  }
  return strIndented;
}

// Test each file in the input folder
class ProgramTestParam {
public:
  string name;
  string source;

  operator std::string() const {
    return "\n" + indent(name + "\n", 1) + indent(source, 2);
  };

  friend ostream &operator<<(ostream &out, const ProgramTestParam &param) {
    return out << string(param);
  }
};

class ProgramFileTest : public testing::TestWithParam<ProgramTestParam> {};

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

std::string trim(const string &str, const string &whitespace = " \t\n") {
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

vector<ProgramTestParam> readTestsFromDisk(const std::string &dirname) {
  vector<ProgramTestParam> testParams;
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
      if (header[0] == '-') {
        continue;
      }

      ProgramTestParam testParam;
      testParam.name = fname + "::" + trim(header);
      testParam.source = trim(string((std::istreambuf_iterator<char>(ss)),
                                (std::istreambuf_iterator<char>())));
      testParams.push_back(testParam);
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
  int status = program.loadString(GetParam().source, errors);
  EXPECT_EQ(0, status);
  if (status != 0) {
    cerr << errors << endl;
  }
  log();
  logDedent();
}

INSTANTIATE_TEST_CASE_P(, ProgramFileTest,
                        testing::ValuesIn(readTestsFromDisk(TEST_INPUT_DIR)));

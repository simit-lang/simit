#include "gtest/gtest.h"
#include <iostream>
#include <fstream>
#include "Program.h"
#include "Logger.h"
using namespace Simit;
using namespace std;

#define PROGRAM_STRING_TEST(test_name, programText, expected)  \
TEST(ProgramStringTest, test_name) {                           \
  log("ProgramStringTest::" + string(#test_name));             \
  logIndent();                                                 \
  Program program;                                             \
  std::string errors;                                          \
  EXPECT_EQ(expected, program.loadString(programText, errors));\
  log("");                                                     \
  logDedent();                                                 \
}

PROGRAM_STRING_TEST(empty, "", 0);
PROGRAM_STRING_TEST(emptyProcDef, "proc main\nend", 0);
PROGRAM_STRING_TEST(invalid, "Invalid program", -1);

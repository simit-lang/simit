#include "gtest/gtest.h"

#include "Program.h"
#include "Logger.h"

using namespace std;
using namespace Simit;

#define PROGRAM_STRING_TEST(test_name, program_text, expected) \
TEST(ProgramStringTest, test_name) {                           \
  log("ProgramStringTest." + string(#test_name) + ":");        \
  logIndent();                                                 \
  Program program;                                             \
  EXPECT_EQ(expected, program.load(program_text));             \
  log("");                                                     \
  logDedent();                                                 \
}

PROGRAM_STRING_TEST(empty, "", 0);
PROGRAM_STRING_TEST(empty_proc_def, "proc main()\nend", 0);
PROGRAM_STRING_TEST(invalid, "Invalid program", 1);


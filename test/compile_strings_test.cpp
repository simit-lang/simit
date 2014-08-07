#include "gtest/gtest.h"

#include <iostream>
#include <fstream>

#include "program.h"

using namespace simit;
using namespace std;

#define PROGRAM_STRING_TEST(test_name, programText, expected)  \
TEST(ProgramStringTest, test_name) {                           \
  Program program;                                             \
  EXPECT_EQ(expected, program.loadString(programText));        \
}

PROGRAM_STRING_TEST(empty, "", 0);
PROGRAM_STRING_TEST(emptyProcDef, "proc main\nend", 0);
PROGRAM_STRING_TEST(invalid, "Invalid program", 1);
PROGRAM_STRING_TEST(unterminated_comment, "%{ comment ", 1);

#include <gtest/gtest.h>
#include <string>
#include <iostream>

int main(int argc, char **argv) {
  // If there is just one argument and it is not a gtest option, then filter
  // the tests using that argument surrounded by wildcards.
  size_t lastArgLen = strlen(argv[argc-1]);
  std::string filter;
  if (argc > 1 &&
      (lastArgLen == 1 ||
       (lastArgLen >= 2 && std::string(argv[argc-1]).substr(0,2) != "--"))) {
    filter = std::string(argv[1]);

    char *dotPtr = strchr(argv[1], '.');
    if (!dotPtr) {
      filter = "*" + filter + "*";
    }
    else if (dotPtr[1] == '\0') {
      filter = filter + "*";
    }

    filter = std::string("--gtest_filter=") + filter;
    argv[1] = (char*)filter.c_str();
  }

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  std::cout << "3 expected failures" << std::endl;
  return result;
}

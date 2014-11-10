#include <gtest/gtest.h>
#include <string>
#include <iostream>

// These are just extern declared from llvm/Support/CommandLine.h since that's
// not currently in the build for simit-test and I'm lazy.
namespace llvm {
namespace cl {
extern
void ParseCommandLineOptions(int argc, const char * const *argv,
                             const char *Overview = nullptr);
extern
void ParseEnvironmentOptions(const char *progName, const char *envvar,
                             const char *Overview = nullptr);
}}

int main(int argc, char **argv) {
  // Get optional LLVM opt-style arguments from the SIMIT_LLVM_DEBUG_ARGS
  // environment variable:
  llvm::cl::ParseEnvironmentOptions("simit-test", "SIMIT_LLVM_DEBUG_ARGS");
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
  return RUN_ALL_TESTS();
}

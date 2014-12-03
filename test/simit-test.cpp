#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <vector>

#include "program.h"
#include "util.h"

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

  // Handle leftover flags
  for (int i = 0; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg[0] == '-') {
      std::vector<std::string> keyValPair = simit::util::split(arg, "=");
      if (keyValPair.size() == 1) {
        std::cerr << "Unrecognized arg: " << arg << std::endl;
        return 1;
      }
      else if (keyValPair.size() == 2) {
        if (keyValPair[0] == "-backend") {
          if (std::find(simit::VALID_BACKENDS.begin(),
                        simit::VALID_BACKENDS.end(),
                        keyValPair[1]) != simit::VALID_BACKENDS.end()) {
            simit::kBackend = keyValPair[1];
          }
          else {
            std::cerr << "Invalid backend: " << keyValPair[1] << std::endl;
            return 1;
          }
        }
        else {
          std::cerr << "Unrecognized arg: " << keyValPair[0] << std::endl;
          return 1;
        }
      }
      else {
        std::cerr << "Misformatted arg: " << arg << std::endl;
        return 1;
      }
    }
  }
  
  return RUN_ALL_TESTS();
}

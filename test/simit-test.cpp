#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <vector>

#include "program.h"
#include "util.h"
#include "init.h"
#include "ir.h"

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

#ifdef F32
// F32 environment setup
class F32Environment : public ::testing::Environment {
public:
  int oldSize;
  virtual void SetUp() {
    oldSize = simit::ir::ScalarType::floatBytes;
    simit::ir::ScalarType::floatBytes = sizeof(float);
  }
  virtual void TearDown() {
    simit::ir::ScalarType::floatBytes = oldSize;
  }
};
#endif

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
  std::string simitBackend = "llvm";
  for (int i = 0; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.substr(0,2) == "--") {
      std::vector<std::string> keyValPair = simit::util::split(arg, "=");
      if (keyValPair.size() == 1) {
        std::cerr << "Unrecognized arg: " << arg << std::endl;
        return 1;
      }
      else if (keyValPair.size() == 2) {
        if (keyValPair[0] == "--backend") {
          simitBackend = keyValPair[1];
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


#ifdef F32
  // Add F32 test environemnt
  ::testing::AddGlobalTestEnvironment(new F32Environment);
  // Set float size
  int floatSize = sizeof(float);
#else
  int floatSize = sizeof(double);
#endif

  simit::init(simitBackend, floatSize);

  return RUN_ALL_TESTS();
}

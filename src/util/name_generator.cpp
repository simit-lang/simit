#include "name_generator.h"

using namespace std;

namespace simit {
namespace util {

// class NameGenerator
std::string NameGenerator::getName() {
  return getName("tmp");
}

std::string NameGenerator::getName(const std::string &suggestion) {
  if (takenNames.find(suggestion) == takenNames.end()) {
    takenNames[suggestion] = 1;
    return suggestion;
  }
  else {
    string name = suggestion;
    do {
      name = suggestion + to_string(takenNames[suggestion]++);
    } while (takenNames.find(name) != takenNames.end());
    return  getName(name);
  }
}

}}

#include "name_generator.h"

using namespace std;

namespace simit {
namespace util {

// class NameGenerator
std::string NameGenerator::getName() {
  return getName(defaultName);
}

std::string NameGenerator::getName(const std::string &suggestion) {
  if (suggestion == "") return getName();

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

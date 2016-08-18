#ifndef SIMIT_GPU_VAR_CLEANER_H
#define SIMIT_GPU_VAR_CLEANER_H

#include <algorithm>

#include "var.h"

namespace simit {
namespace ir {

class GpuVarCleaner : public VarCleaner {
private:
  std::set<std::string> seenNames;
  std::string makeUniqueName(std::string name) {
    if (!util::contains(seenNames, name)) {
      seenNames.insert(name);
      return name;
    }
    int index = 0;
    while (util::contains(seenNames, name + "_" + std::to_string(index))) {
      ++index;
    }
    name = name + "_" + std::to_string(index);
    seenNames.insert(name);
    return name;
  }
public:
  virtual std::string cleanImpl(std::string name) {
    name = makeUniqueName(name);
    // Remove all dots in variable names
    std::replace(name.begin(), name.end(), '.', '_');
    // Remove all @ symbols in variable names
    std::replace(name.begin(), name.end(), '@', 'A');
    return name;
  }
};

}} // simit::ir

#endif

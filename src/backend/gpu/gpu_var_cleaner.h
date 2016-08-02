#ifndef SIMIT_GPU_VAR_CLEANER_H
#define SIMIT_GPU_VAR_CLEANER_H

#include <algorithm>

#include "var.h"

namespace simit {
namespace ir {

class GpuVarCleaner : public VarCleaner {
public:
  virtual std::string cleanImpl(std::string name) const {
    // Remove all dots in variable names
    std::replace(name.begin(), name.end(), '.', '_');
    // Remove all @ symbols in variable names
    std::replace(name.begin(), name.end(), '@', 'A');
    return name;
  }
};

}} // simit::ir

#endif

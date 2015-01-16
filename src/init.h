#ifndef INIT_H
#define INIT_H

#include <algorithm>
#include <string>

#include "error.h"
#include "ir.h"
#include "program.h"

namespace simit {

extern const std::vector<std::string> VALID_BACKENDS;
extern std::string kBackend;

inline void init(std::string backend="llvm", int floatSize=8) {
  uassert(std::find(VALID_BACKENDS.begin(),
                    VALID_BACKENDS.end(), backend) != VALID_BACKENDS.end())
      << "Invalid backend: " << backend;
  kBackend = backend;
  ir::ScalarType::floatBytes = floatSize;
}


}  // namespace simit

#endif

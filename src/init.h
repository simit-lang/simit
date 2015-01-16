#ifndef INIT_H
#define INIT_H

#include "error.h"
#include "ir.h"
#include "program.h"

namespace simit {

inline void init(std::string backend="llvm", int floatSize=8) {
  uassert(std::find(VALID_BACKENDS.begin(),
                    VALID_BACKENDS.end(), backend) != VALID_BACKENDS.end())
      << "Invalid backend: " << backend;
  kBackend = backend;
  ir::ScalarType::floatBytes = floatSize;
}


}  // namespace simit

#endif

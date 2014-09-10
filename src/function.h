#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include <cstdlib>
#include <vector>

// TODO: Remove
#include "ir.h"

namespace simit {

class Function {
public:
  Function() : runPtr(NULL) {}
  virtual ~Function() {}

  // TODO: Change shared_ptr<simit::internal::Literal> to simit::Tensor*
  virtual void bind(const std::vector<std::shared_ptr<internal::Literal>> &arguments,
                    const std::vector<std::shared_ptr<internal::Literal>> &results) = 0;

  inline void run() { runPtr(); }

  virtual void print(std::ostream &os) const {};

protected:
  typedef void (*RunPtrType)();
  inline void setRunPtr(RunPtrType runPtr) { this->runPtr = runPtr; }

private:
  RunPtrType runPtr;
};

std::ostream &operator<<(std::ostream &os, const Function &f);

} // namespace simit
#endif

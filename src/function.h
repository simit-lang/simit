#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include "tensor.h" // TODO: Replace with forward declaration

namespace simit {
class Set;

namespace backend {
class Function;
}

/// A callable Simit function. You can bind arguments and externs to a function
/// using the `bind` methods and call it using the `run` and `runSafe` method.
///
/// If you call the function using `run` (recommended for performance) you have
/// to first call `init` to initialize bound arguments and externs. Furthermore,
/// you must make the bound arguments and externs available to the function by
/// calling `mapArgs`. Finally, when the host program needs to read results and
/// externs it must call `unmapArgs` to make updates made by the function
/// visible.
///
/// If you call the function using `runSafe` (recommended for testing) you don't
/// need to call `init`, `mapArgs` or `unmapArgs` as these will be called
/// automatically.
class Function {
public:
  Function();
  Function(backend::Function *function);

  void bind(const std::string &argName, Tensor *tensor);
  void bind(const std::string &argName, Set *set);

  void init();
  bool isInit();

  inline void run() {
    iassert(isInit()) << "Function has not been initialized";
    funcPtr();
  }

  void runSafe();

  void mapArgs();
  void unmapArgs(bool updated=true);

  bool defined() {return impl != nullptr;}

  void printMachine(std::ostream &os);
  friend std::ostream &operator<<(std::ostream &os, const Function &f);

private:
  std::shared_ptr<backend::Function> impl;

  // To make the run method faster we store the function pointer here.
  std::function<void()> funcPtr;
};

}
#endif

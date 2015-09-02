#ifndef SIMIT_FUNCTION_H
#define SIMIT_FUNCTION_H

#include <string>
#include <functional>
#include "tensor.h"

namespace simit {
class Set;

namespace backend {
class Function;
}

/// A callable Simit function. You can bind arguments and externs (bindables) to
/// a function using the `bind` methods and call it using the `run` and
/// `runSafe` method.
///
/// If you call the function using `run` (recommended for performance) you have
/// to first call `init` to initialize bound arguments and externs. Furthermore,
/// you must make the bound arguments and externs available to the function by
/// calling `mapArgs`. Finally, when the host program needs to read results and
/// externs it must call `unmapArgs` to make updates made by the function
/// visible.
///
/// If you call the function using `runSafe` (recommended for testing) you don't
/// need to call `init`, `mapArgs` or `unmapArgs` as they will be called
/// automatically.
class Function {
public:
  Function();

  /// Create a function from a backend::Function. backend::Function objects can
  /// be created using the backend::Backend::compile methods.
  Function(backend::Function *function);

  /// Bind the set to the given argument.
  void bind(const std::string& bindable, simit::Set* set);

  /// Bind the tensor to the given argument.
  template <typename CType, int... Dims>
  void bind(const std::string& bindable, Tensor<CType,Dims...>* tensor) {
    bind(bindable, tensor->getType(), tensor->getData());
  }

  /// Bind tensor data to the bindable with type checks.
  void bind(const std::string& bindable, const TensorType& ttype, void* data);

  /// Bind tensor data to the bindable.
  void bind(const std::string& bindable, void* data);

  /// Bind sparse tensor data in the CSR format to the bindable.
  /// See e.g. \link https://en.wikipedia.org/wiki/Sparse_matrix
  void bind(const std::string& bindable, const int* rowPtr, const int* colInd,
            void* data);

  /// Initialize the function. This must be done between calls to bind arguments
  /// and calls to run. If runSafe is used, there init will be called
  /// automatically as needed.
  void init();

  /// Run the function. Make sure to bind arguments and map arguments, and to
  /// init the function before calling this method. Also make sure to map/unmap
  /// arguments if you need to access them between calls to run.
  inline void run() {
    funcPtr();
  }

  /// Run the function. This method will automatically map/unmap arguments and
  /// initialize the function as necessary. However, it will incur additional
  /// overhead over manually initializing and mapping arguments.
  void runSafe();

  void mapArgs();
  void unmapArgs(bool updated=true);

  /// True if the function has been defined, false otherwise.
  bool defined() const {return impl != nullptr;}

  /// Print the function to the stream. The output depends on the backend. For
  /// example, the LLVM backend will print LLVM IR.
  void print(std::ostream& os) const;

  /// Print the function to the stream as machine assembly code.
  void printMachine(std::ostream& os) const;

private:
  std::shared_ptr<backend::Function> impl;

  // To make the run method faster we store the function pointer here.
  std::function<void()> funcPtr;
};

/// Write the function to the stream. The output depends on the backend,
/// for example the LLVM backend will write LLVM IR. Same as Function::print.
std::ostream& operator<<(std::ostream& os, const Function& f);

}
#endif

#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <ostream>
#include <vector>
#include <memory>

#include "init.h"
#include "tensor.h" // TODO: Replace with forward declaration
#include "uncopyable.h"

namespace simit {

extern const std::vector<std::string> VALID_BACKENDS;
extern std::string kBackend;

class Set;
class Diagnostics;

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

private:
  std::shared_ptr<backend::Function> impl;

  // To make the run method faster we store the function pointer here.
  std::function<void()> funcPtr;
};


/// A Simit program. You can load Simit source code using the \ref loadString
/// and \ref loadFile, register input sets using the \ref registerSet method,
/// and compile the program using the \ref compile method.
class Program : private interfaces::Uncopyable {
public:
  /// Create a new Simit program.
  Program();
  ~Program();

  /// Add the Simit code in the given string to the program.
  /// \return 0 on success, and 1 if the Simit code has errors. If the code
  ///         has errors these can be retrieved through the \ref getErrors
  ///         and \ref getErrorString methods.
  int loadString(const std::string &programString);

  /// Add the Simit code in the given file to the program.
  /// \return 0 on success, 1 if the Simit code has errors, and 2 if the file
  ///         could not be read. If the code has errors these can be retrieved 
  ///         through the \ref getErrors and \ref getErrorString methods.
  int loadFile(const std::string &filename);

  /// Returns the names of all the functions in the program.
  std::vector<std::string> getFunctionNames() const;

  /// Compile and return a runnable function, or an undefined function if an
  /// error occured.
  Function compile(const std::string &function);

  /// Verify the program by executing in-code comment tests.
  int verify();

  bool hasErrors() const;
  const Diagnostics &getDiagnostics() const;

  /// Writes a human-readable string represeting the program to the stream.
  friend std::ostream &operator<<(std::ostream&, const Program&);

private:
  struct ProgramContent;
  ProgramContent *content;
};

}
#endif

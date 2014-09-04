#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <ostream>
#include <vector>
#include <memory>

namespace simit {

class Diagnostics;
class Error;
class Set;
class Tensor;
class Function;

/// A Simit program. You can load Simit source code using the \ref loadString
/// and \ref loadFile, register input sets using the \ref registerSet method,
/// and compile the program using the \ref compile method.
class Program {
 public:
  /// Create a new Simit program with the given name.
  Program(const std::string &name="");
  ~Program();

  /// Get program name.
  std::string getName() const;

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

  /// Compile and return a runnable function or NULL if an error occurred.
  std::unique_ptr<Function> compile(const std::string &function);

  /// Verify the program by executing in-code comment tests.
  int verify();

  bool hasErrors() const;
  const Diagnostics &getDiagnostics() const;

  /// Writes a human-readable string represeting the program to the stream.
  friend std::ostream &operator<<(std::ostream&, const Program&);

 private:
  class ProgramContent;
  ProgramContent *impl;

  // Uncopyable
  Program(const Program&);
  Program& operator=(const Program&);
};

}
#endif

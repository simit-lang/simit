#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <ostream>
#include <vector>

namespace simit {
namespace internal {
class ProgramContent;
}

class Error;
class Set;
class Tensor;

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

  /// Binds the set to the given name and makes it available to Simit programs.
  /// Inside Simit programs the set can be accessed through extern declarations
  /// whose type match the set type and whose name match the given name.
  int bindSet(const simit::Set &set, const std::string &name);

  /// Binds the Tensor to the given name and makes it available to Simit
  /// programs. Inside Simit programs the tensor can be accessed through extern
  /// declarations whose type match the tensor type and whose name match the
  /// given name. */
  int bindTensor(const std::string &name, const simit::Tensor &tensor);

  /// Compile the program.
  /// \return 0 on success, 1 if the program is incomplete.
  int compile();

  /// Verify the program by executing in-code comment tests.
  int verify();

  /// Run the program.
  /// \return 0 on success, 1 if the program could not be run.
  int run();

  /// Get a vector of errors that occurred when trying to add code to the
  /// program.
  std::vector<simit::Error> &getErrors();

  /// Get a string that describes errors that occurred when trying to add code
  /// to the program.
  std::string getErrorString();

  /// Writes a human-readable string represeting the program to the stream.
  friend std::ostream &operator<<(std::ostream&, const Program&);

 private:
  internal::ProgramContent *impl;
};

}
#endif

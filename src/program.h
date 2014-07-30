#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <ostream>
#include <list>

namespace simit {
namespace internal {
class Program;
}

class Error;

/** A Simit program. You can load Simit source code using the \ref loadString
  * and \ref loadFile, register input sets using the \ref registerSet method,
  * and compile the program using the \ref compile method.  */
class Program {
 public:
    /** Create a new Simit program with the given name. */
  Program(const std::string &name="");
  ~Program();

  /** Get program name. */
  std::string getName() const;

  /** Add the Simit code in the given string to the program.
    * \return 0 on success, and 1 if the Simit code has errors. If the code
    *         has errors these can be retrieved through the \ref getErrors
    *         and \ref getErrorString methods.
    */
  int loadString(const std::string &programString);

  /** Add the Simit code in the given file to the program.
    * \return 0 on success, 1 if the Simit code has errors, and 2 if the file
    *         could not be read. If the code has errors these can be retrieved 
    *         through the \ref getErrors and \ref getErrorString methods.
    */
  int loadFile(const std::string &filename);

  /** Compile the program.
    * \return 0 on success, 1 if the program is incomplete. */
  int compile();

  /** Run the program.
    * \return 0 on success, 1 if the program could not be run. */
  int run();

  /** Get a list of errors that occurred when trying to add code to the
    * program. */
  std::list<simit::Error> &getErrors();

  /** Get a string that describes errors that occurred when trying to add code
    * to the program. */
  std::string getErrorString();

  /** Writes a human-readable string represeting the program to the stream. */
  friend std::ostream &operator<<(std::ostream&, const Program&);

 private:
  internal::Program *impl;
};

}
#endif

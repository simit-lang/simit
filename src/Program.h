#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <set>
#include <list>
#include <memory>

namespace simit {

class Frontend;
class IRNode;
class Test;
class Error;

/** A Simit program. You can load Simit source code using the \ref loadString
  * and \ref loadFile, register input sets using the \ref registerSet method,
  * and compile the program using the \ref compile method.  */
class Program {
 public:
  Program();
  ~Program();

  /** Add the Simit code in the given string to the program.
    * \return 0 on success, and 1 if the Simit code has errors. If the code
    *         has errors these can be retrieved through the \ref getErrors
    *         and \ref getErrorString methods.
    */
  int loadString(std::string programString);

  /** Add the Simit code in the given file to the program.
    * \return 0 on success, 1 if the Simit code has errors, and 2 if the file
    *         could not be read. If the code has errors these can be retrieved 
    *         through the \ref getErrors and \ref getErrorString methods.
    */
  int loadFile(std::string filename);

  /** Compile the program.
    * \return 0 on success, 1 if the program is incomplete. */
  int compile();

  /** Get a list of errors that occurred when trying to add code to the
    * program. */
  std::list<simit::Error> &getErrors();

  /** Get a string that describes errors that occurred when trying to add code
    * to the program. */
  std::string getErrorString();

 private:
  Frontend *frontend;
  std::set<IRNode*> IRNodes;
};

}

#endif

#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <set>
#include <list>

namespace simit {

class Frontend;
class IRNode;
class Test;
class Error;

class Program {
 public:
  Program();
  virtual ~Program();

  int loadString(std::string programString);
  int loadFile(std::string filename);

  std::string getErrorString();
  std::list<std::shared_ptr<simit::Error>> getErrors();

  int compile();
  
 private:
  Frontend *frontend;
  std::set<IRNode*> IRNodes;
};

}

#endif

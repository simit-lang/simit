#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <string>

namespace simit {

class Program;

class Frontend {
 public:
  Frontend();
  ~Frontend();

  int parseString(std::string programString, Program *program);
  int parseFile(std::string filename, Program *program);

 private:

};
}

#endif
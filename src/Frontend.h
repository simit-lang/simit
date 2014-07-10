#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <string>

namespace Simit {
  class Program;
  class Test;
  class IRNode;

  class Frontend {
  public:
    Frontend();
    ~Frontend();

    int parseString(std::string programString, Simit::Program *program);
    int parseFile(std::string filename, Simit::Program *program);

  private:

  };
}

#endif
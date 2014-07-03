#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <list>

namespace Simit {
  class Frontend;
  
  class Program {
  public:
    Program();
    virtual ~Program();
    
    int loadString(std::string program);
    int loadString(std::string program, std::string &errors);

    int loadFile(std::string filename);
    int loadFile(std::string filename, std::string &errors);

    int compile();
    
  private:
    std::unique_ptr<Frontend> frontend;
  };
  
}

#endif
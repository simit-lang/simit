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
    
    int load(std::string programText);
    int load(std::string programText, std::string &errors);

    int load(std::ifstream programFile);

    int compile();
    
  private:
    std::unique_ptr<Frontend> frontend;
    std::list<std::string> programText;
  };
  
}

#endif
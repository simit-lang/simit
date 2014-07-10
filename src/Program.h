#ifndef SIMIT_PROGRAM_H
#define SIMIT_PROGRAM_H

#include <string>
#include <vector>

namespace Simit {
  class Frontend;
  class IRNode;
  class Test;
  
  class Program {
  public:
    Program();
    virtual ~Program();
    
    int loadString(std::string programString);
    int loadFile(std::string filename);
    std::string errors();

    int compile();

    void addError(std::string errors);
    void addTest(Simit::Test *test);
  private:
    Frontend *frontend;
    std::vector<IRNode*> irNodes;
    std::string errorString;

    std::vector<Simit::Test*> tests;
  };
  
}

#endif
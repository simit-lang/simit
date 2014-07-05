#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <cstdio>
#include <string>
#include <vector>

namespace Simit {
  class IRNode;
}

extern FILE *yyin;
int yyparse(Simit::IRNode **irNode, std::string *errors);
struct yy_buffer_state *yy_scan_string(const char *);
int yylex_destroy();

namespace Simit {

  class Frontend {
  public:
    Frontend();
    ~Frontend();

    int parseString(std::string program,
                    std::vector<std::shared_ptr<IRNode> > &irNodes,
                    std::string &errors);

    int parseFile(std::string filename,
                  std::vector<std::shared_ptr<IRNode> > &irNodes,
                  std::string &errors);

  private:

  };
}

#endif
#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <cstdio>
#include <string>
#include <vector>

namespace Simit {
  class IRNode;
}

extern FILE *yyin;
int yyparse(Simit::IRNode **irNode);
int yylex (void);
int yy_scan_string	(const char *str);
void yyerror(Simit::IRNode **irNode, const char *s);

namespace Simit {

  class Frontend {
  public:
    Frontend();
    ~Frontend();

    IRNode *parse(const std::string &programString, std::string &errors);
    int parse(const std::string &programString,
              std::vector<std::shared_ptr<IRNode> > *irNodes,
              std::string *errors);

  private:

  };

}

#endif
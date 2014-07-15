#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <string>

namespace simit {

  class IRNode {
  public:
    IRNode() {}
    virtual ~IRNode() {}

    virtual operator std::string() const = 0;
  };

  class Function {
  public:
    Function();
    virtual ~Function();

    virtual operator std::string() const;
  };

}

#endif

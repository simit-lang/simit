#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <string>

namespace Simit {

  class IRNode {
  public:
    IRNode() {}
    virtual ~IRNode() {}

    virtual operator std::string() const { return "IRNode"; };
    std::ostream& operator<<(std::ostream& os) {return os<<std::string(*this);}
  };

  class Function {
  public:
    Function();
    virtual ~Function();

    virtual operator std::string() const;
  };

}

#endif

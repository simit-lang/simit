#ifndef SIMIT_TEST_H
#define SIMIT_TEST_H

#include <string>

namespace simit {

class Test {
 public:
  Test(std::string name) : name(name) {}
  virtual ~Test() {}

  virtual operator std::string() const {
    return "Test " + name;
  };

 private:
  std::string name;
};

}

#endif

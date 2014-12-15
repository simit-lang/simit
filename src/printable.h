#ifndef SIMIT_PRINTABLE_H
#define SIMIT_PRINTABLE_H

#include <ostream>

namespace simit {
namespace interfaces {

class Printable {
public:
  friend std::ostream &operator<<(std::ostream &os, const Printable &pr);

private:
  virtual void print(std::ostream &os) const = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Printable &printable) {
  printable.print(os);
  return os;
}

}}
#endif

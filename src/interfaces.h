#ifndef SIMIT_INTERFACES_H
#define SIMIT_INTERFACES_H

#include <ostream>

namespace simit {
namespace util {

/// C++11 interface that prevents a class from being copied.
class Uncopyable {
 protected:
  Uncopyable() = default;
  ~Uncopyable() = default;

 private:
  Uncopyable(const Uncopyable&) = delete;
  Uncopyable& operator=(const Uncopyable&) = delete;
};


class Printable {
public:
  virtual ~Printable() {};
  virtual void print(std::ostream &os) const = 0;
};

inline std::ostream &operator<<(std::ostream &os, const Printable &printable) {
  printable.print(os);
  return os;
}

}}  // namespace simit::util
#endif

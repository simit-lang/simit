#ifndef SIMIT_INTERFACES_H
#define SIMIT_INTERFACES_H

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

}}  // namespace simit::util
#endif

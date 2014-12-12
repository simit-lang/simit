#ifndef SIMIT_UNCOPYABLE_H
#define SIMIT_UNCOPYABLE_H

namespace simit {
namespace interfaces {

class Uncopyable {
protected:
  Uncopyable() = default;
  ~Uncopyable() = default;

private:
  Uncopyable(const Uncopyable&) = delete;
  Uncopyable& operator=(const Uncopyable&) = delete;
};

}}
#endif

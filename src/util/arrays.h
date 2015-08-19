#ifndef SIMIT_ARRAYS_H
#define SIMIT_ARRAYS_H

namespace simit {
namespace util {

template <typename T>
void zero(T* data, size_t size) {
  T* tdata = static_cast<T*>(data);
  std::fill(tdata, tdata+size, T());
}
template <typename T>
void zero(void* data, size_t size) {
  T* tdata = static_cast<T*>(data);
  std::fill(tdata, tdata+size, T());
}

}}
#endif

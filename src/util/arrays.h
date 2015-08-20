#ifndef SIMIT_ARRAYS_H
#define SIMIT_ARRAYS_H

namespace simit {
namespace util {

template <typename T>
void zero(T* data, size_t size) {
  std::fill(data, data+size, T());
}
template <typename T>
void zero(void* data, size_t size) {
  zero(static_cast<T*>(data), size);
}


template <typename T>
static void printArray(std::ostream &os, const T* data, unsigned size,
                       const std::string &separator=", ") {
  if (size == 1) {
    os << data[0];
  }
  else {
    for (size_t i=1; i < size; ++i) {
      os << ", " << data[i];
    }
  }
}
template <typename T>
static void printArray(std::ostream &os, const void* data, unsigned size,
                       const std::string &separator=", ") {
  printArray(os, static_cast<T*>(data), size, separator);
}

}}
#endif

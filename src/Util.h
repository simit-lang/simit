#ifndef SIMIT_UTIL_H
#define SIMIT_UTIL_H

#include <string>
#include <sstream>

namespace util {

template <typename T>
std::ostream& operator<<(std::ostream &out, const T *dim) {
  return out << *dim;
}

template <typename T>
std::ostream& operator<<(std::ostream &out, const std::shared_ptr<T> dim) {
  return out << *dim;
}

template <typename T>
std::ostream& operator<<(std::ostream &out, const std::unique_ptr<T> dim) {
  return out << *dim;
}

template <typename Iterator>
std::string join(Iterator begin, Iterator end, const std::string &separator) {
  std::ostringstream result;
  if (begin != end) {
    result << *begin++;
  }
  while (begin != end) {
    result << separator << *begin++;
  }
  return result.str();
}

template <typename Collection>
std::string join(const Collection &collection, const std::string &separator) {
  return join(collection.begin(), collection.end(), separator);
}

template <typename Collection>
std::string rjoin(const Collection &collection, const std::string &separator) {
  return join(collection.rbegin(), collection.rend(), separator);
}

}

#endif

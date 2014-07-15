#ifndef SIMIT_UTIL_H
#define SIMIT_UTIL_H

#include <string>
#include <sstream>

namespace util {
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
}

#endif

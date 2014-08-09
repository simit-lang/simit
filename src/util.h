#ifndef SIMIT_UTIL_H
#define SIMIT_UTIL_H

#include <string>
#include <sstream>
#include <memory>

namespace simit {
namespace util {

/** Turn anything that can be written to a stream into a string. */
template <class T>
std::string toString(const T &val) {
  std::stringstream sstream;
  sstream << val;
  return sstream.str();
}

/** Join the elements between begin and end in a sep-separated string. */
template <typename Iterator>
std::string join(Iterator begin, Iterator end, const std::string &sep) {
  std::ostringstream result;
  if (begin != end) {
    result << *begin++;
  }
  while (begin != end) {
    result << sep << *begin++;
  }
  return result.str();
}

/** Join the elements in the collection in a sep-separated string. */
template <typename Collection>
std::string join(const Collection &collection, const std::string &sep) {
  return join(collection.begin(), collection.end(), sep);
}

/** Reverse join the elements in the collection in a sep-separated string. */
template <typename Collection>
std::string rjoin(const Collection &collection, const std::string &sep) {
  return join(collection.rbegin(), collection.rend(), sep);
}

// Some << overloadings to let the join functions print out pointer collections.
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

/** Indent each line in str by num spaces. */
std::string indent(std::string str, unsigned int num);

}} // namespace simit::util

#endif

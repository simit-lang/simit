#ifndef SIMIT_UTIL_H
#define SIMIT_UTIL_H

#include <string>
#include <sstream>
#include <memory>
#include <vector>

namespace simit {
namespace util {

/// Turn anything that can be written to a stream into a string.
template <class T>
std::string toString(const T &val) {
  std::stringstream sstream;
  sstream << val;
  return sstream.str();
}

/// Some << overloads to let the join functions print out pointer collections.
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

/// Join the elements between begin and end in a sep-separated string.
template <typename Iterator>
std::string join(Iterator begin, Iterator end, const std::string &sep=", ") {
  std::ostringstream result;
  if (begin != end) {
    result << *begin++;
  }
  while (begin != end) {
    result << sep << *begin++;
  }
  return result.str();
}

/// Join the elements in the collection in a sep-separated string.
template <typename Collection>
std::string join(const Collection &collection, const std::string &sep=", ") {
  return join(collection.begin(), collection.end(), sep);
}

/// Reverse join the elements in the collection in a sep-separated string.
template <typename Collection>
std::string rjoin(const Collection &collection, const std::string &sep=", ") {
  return join(collection.rbegin(), collection.rend(), sep);
}

/// Indent each line in str by num spaces.
std::string indent(std::string str, unsigned int num);

/// Split the string.
std::vector<std::string> split(const std::string &str, const std::string &delim,
                               bool keepDelim = false);

// Load text from file, returning 0 if succesfull, false otherwise.
int loadText(const std::string &file, std::string *text);

// Trim whitespace from string
std::string trim(const std::string &str, const std::string &ws = " \t\n");


/// Vector that owns the pointers in it and deletes them on destruction.
template <class T>
class OwnershipVector {
public:
  typedef typename std::vector<T>::iterator iterator;

  ~OwnershipVector() { for (auto &obj : vec) { delete obj; } }

  void push_back(const T &val) { vec.push_back(val); }
  void clear() { vec.clear(); }

  size_t size() const { return vec.size(); }

  iterator begin() { return vec.begin(); }
  iterator end() { return vec.end(); }

  T &operator[](size_t n) { return vec[n]; }
  const T &operator[](size_t n) const { return vec[n]; }

private:
  std::vector<T> vec;
};

}} // namespace simit::util

#endif

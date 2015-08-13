#ifndef SIMIT_UTIL_H
#define SIMIT_UTIL_H

#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cassert>

namespace simit {
namespace util {

/// Turn anything that can be written to a stream into a string.
template <class T>
std::string toString(const T &val) {
  std::stringstream sstream;
  sstream << val;
  return sstream.str();
}

/// Some << overloads to let the join functions print smart-pointer collections.
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

/// Load text from file, returning 0 if succesfull, false otherwise.
int loadText(const std::string &file, std::string *text);

// /Trim whitespace from string
std::string trim(const std::string &str, const std::string &ws = " \t\n");

/// Query whether a collection contains an element
template <class C, typename V>
bool contains(const C &container, const V &value) {
  return std::find(container.begin(),container.end(),value) != container.end();
}

/// Query whether a set contains an element
template <typename V>
bool contains(const std::set<V> &container, const V &value) {
  return container.find(value) != container.end();
}

/// Query whether a map contains an element
template <typename K, typename V>
bool contains(const std::map<K,V> &container, const K &key) {
  return container.find(key) != container.end();
}

template <typename Collection>
class ReverseIterable {
public:
  typedef typename Collection::reverse_iterator reverse_iterator;
  ReverseIterable(Collection &c) : collection(c) {}
  reverse_iterator begin() {return collection.rbegin();}
  reverse_iterator end() {return collection.rend();}
private:
  Collection &collection;
};

/// Iterate over a collection in reverse using a range for loop:
/// for (auto &element : util::reverse(collection)) {...}
template <typename T>
ReverseIterable<T> reverse(T &collection) {
  return ReverseIterable<T>(collection);
}

template <typename Collection>
class ReverseConstIterable {
public:
  typedef typename Collection::const_reverse_iterator const_reverse_iterator;
  ReverseConstIterable(const Collection &c) : collection(c) {}
  const_reverse_iterator begin() const {return collection.rbegin();}
  const_reverse_iterator end() const {return collection.rend();}
private:
  const Collection &collection;
};

/// Iterate over a collection in reverse using a range for loop:
/// for (auto &element : util::reverse(collection)) {...}
template <typename T>
ReverseIterable<T> reverse(const T &collection) {
  return ReverseConstIterable<T>(collection);
}

template <typename Collection>
class ExcludeFirstIterable {
public:
  typedef typename Collection::iterator iterator;
  ExcludeFirstIterable(Collection &c) : collection(c) {}
  iterator begin() {
    return (collection.begin() == collection.end() ?   collection.end()
                                                   : ++collection.begin());
  }
  iterator end() {return collection.end();}
private:
  Collection &collection;
};

template<typename T>
ExcludeFirstIterable<T> excludeFirst(T &collection) {
  return ExcludeFirstIterable<T>(collection);
}

template <typename Collection>
class ExcludeFirstConstIterable {
public:
  typedef typename Collection::const_iterator const_iterator;
  ExcludeFirstConstIterable(const Collection &c) : collection(c) {}
  const_iterator begin() const {
    return (collection.begin() == collection.end() ?   collection.end()
                                                   : ++collection.begin());
  }
  const_iterator end() const {return collection.end();}
private:
  const Collection &collection;
};

template<typename T>
ExcludeFirstConstIterable<T> excludeFirst(const T &collection) {
  return ExcludeFirstConstIterable<T>(collection);
}


/// Retrieve the location in the collection of the given value
template <class Collection, typename Value>
size_t locate(const Collection &collection, const Value &value) {
  assert(util::contains(collection, value));
  return std::distance(collection.begin(), std::find(collection.begin(),
                                                     collection.end(), value));
}

template <class Collection, typename ParamValue, typename ResultValue>
std::vector<ResultValue> map(const Collection &collection,
                             std::function<ResultValue(ParamValue)> f) {
  std::vector<ResultValue> result;
  result.resize(collection.size());
  std::transform(collection.begin(), collection.end(), result.begin(), f);
  return result;
}

}}

#endif

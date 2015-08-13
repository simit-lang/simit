#ifndef SIMIT_UTIL_COLLECTIONS_H
#define SIMIT_UTIL_COLLECTIONS_H

namespace simit {
namespace util {

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

/// Retrieve the location in the collection of the given value
template <class Collection, typename Value>
size_t locate(const Collection &collection, const Value &value) {
  assert(util::contains(collection, value));
  return std::distance(collection.begin(), std::find(collection.begin(),
                                                     collection.end(), value));
}

/// Map the elements of a collection to a vector, using the function f.
template <class Collection, typename ParamValue, typename ResultValue>
std::vector<ResultValue> map(const Collection &collection,
                             std::function<ResultValue(ParamValue)> f) {
  std::vector<ResultValue> result;
  result.resize(collection.size());
  std::transform(collection.begin(), collection.end(), result.begin(), f);
  return result;
}

// Iterables
template <typename Collection>
class ReverseIterable {
public:
  typedef typename Collection::reverse_iterator reverse_iterator;
  ReverseIterable(Collection &c) : c(c) {}
  reverse_iterator begin() {return c.rbegin();}
  reverse_iterator end() {return c.rend();}
private:
  Collection &c;
};

template <typename Collection>
class ReverseConstIterable {
public:
  typedef typename Collection::const_reverse_iterator const_reverse_iterator;
  ReverseConstIterable(const Collection &c) : c(c) {}
  const_reverse_iterator begin() const {return c.rbegin();}
  const_reverse_iterator end() const {return c.rend();}
private:
  const Collection &c;
};

template <typename Collection>
class ExcludeFirstIterable {
public:
  typedef typename Collection::iterator iterator;
  ExcludeFirstIterable(Collection &c) : c(c) {}
  iterator begin() {return (c.begin() == c.end() ? c.end() : ++c.begin());}
  iterator end() {return c.end();}
private:
  Collection &c;
};

template <typename Collection>
class ExcludeFirstConstIterable {
public:
  typedef typename Collection::const_iterator const_iterator;
  ExcludeFirstConstIterable(const Collection &c) : c(c) {}
  const_iterator begin() const {return c.begin()==c.end()?c.end():++c.begin();}
  const_iterator end() const {return c.end();}
private:
  const Collection &c;
};

/// Iterate over a collection in reverse using a range for loop:
/// for (auto &element : util::reverse(collection)) {...}
template <typename T>
ReverseIterable<T> reverse(T &collection) {
  return ReverseIterable<T>(collection);
}

/// Iterate over a collection in reverse using a range for loop:
/// for (auto &element : util::reverse(collection)) {...}
template <typename T>
ReverseIterable<T> reverse(const T &collection) {
  return ReverseConstIterable<T>(collection);
}

/// Iterate over the elements in a collection, excluding the first element.
template<typename T>
ExcludeFirstIterable<T> excludeFirst(T &collection) {
  return ExcludeFirstIterable<T>(collection);
}

/// Iterate over the elements in a collection, excluding the first element.
template<typename T>
ExcludeFirstConstIterable<T> excludeFirst(const T &collection) {
  return ExcludeFirstConstIterable<T>(collection);
}


}}
#endif

#ifndef SIMIT_SYMBOLTABLE_H
#define SIMIT_SYMBOLTABLE_H

#include <cassert>
#include <string>
#include <map>
#include <list>

#include "util.h"

namespace simit {
namespace internal {

/// Tracks symbols and their associated IR nodes across a stack of scopes.
template <typename Key, typename Value>
class ScopedMap {
public:
  typedef std::map<Key, Value> Map;
  typedef typename std::list<Map>::const_iterator Iterator;

  ScopedMap()  {   scope(); }
  ~ScopedMap() { unscope(); }

  /// Add a new level of symbol scoping.
  void scope()   { scopes.push_front(Map()); }

  /// Remove the top symbol scope.
  void unscope() { scopes.pop_front(); }

  /// Return the first match
  void insert(const Key &symbol, const Value &value) {
    scopes.front()[symbol] = value;
  }

  // True iff the symbol table contains the given symbol.
  bool contains(const Key &symbol) const {
    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return true;
      }
    }
    return false;
  }

  /// Return the first match.  It is an error to call get if the symbol is not
  /// in the symbol table.
  Value &get(const Key &symbol) {
    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return scope.at(symbol);
      }
    }
    assert(false && "Attempting to get a symbol that is not in symbol table.");
  }

  const Value &get(const Key &symbol) const {
    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return scope.at(symbol);
      }
    }
    assert(false && "Attempting to get a symbol that is not in symbol table.");
  }

  /// Iterator over symbol scopes.
  Iterator begin() const { return scopes.begin(); }

  /// Iterator over symbol scopes.
  Iterator end() const { return scopes.end(); }

  /// Print symbol table to stream.
  template <typename K, typename V>
  friend std::ostream &operator<<(std::ostream &os, const ScopedMap<K,V> &st) {
    os << "SymbolTable:\n";
    for (auto scope : st.scopes) {
      for (auto symPair : scope) {
        std::string symString = simit::util::toString(symPair.second);
        os << simit::util::indent(symPair.first + ":" + symString + ", ", 1);
      }
      os << "\n";
    }
    return os;
  }

private:
  std::list<Map> scopes;
};

}} // namespace simit::internal

#endif

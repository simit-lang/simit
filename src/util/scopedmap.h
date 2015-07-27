#ifndef SIMIT_SYMBOLTABLE_H
#define SIMIT_SYMBOLTABLE_H

#include <string>
#include <map>
#include <list>
#include <ostream>

#include "error.h"
#include "util.h"

namespace simit {
namespace util {

/// Tracks symbols and their associated IR nodes across a stack of scopes.
template <typename Key, typename Value>
class ScopedMap {
public:
  typedef std::map<Key, Value> Map;
  typedef typename std::list<Map>::const_iterator Iterator;

  ScopedMap()  {   scope(); }
  ~ScopedMap() { unscope(); }

  void clear() {
    iassert(scopes.size() == 1);
    scopes.front().clear();
  }

  /// Add a new level of symbol scoping.
  void scope()   { scopes.push_front(Map()); }

  /// Remove the top symbol scope.
  void unscope() { scopes.pop_front(); }

  // Insert key-value pair into current scope
  void insert(const Key &symbol, const Value &value) {
    scopes.front()[symbol] = value;
  }

  // Insert key-value pair into current scope
  void insert(const std::pair<Key, Value> &symVal) {
    scopes.front().insert(symVal);
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
    ierror << "Attempting to load symbol" << symbol
           << "which is not in the symbol table";
    return scopes.begin()->begin()->second;
  }

  const Value &get(const Key &symbol) const {
    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return scope.at(symbol);
      }
    }
    iassert(false) << "Attempting to get a symbol that is not in symbol table.";
    return scopes.begin()->at(symbol); // silence warning
  }

  /// Iterator over symbol scopes.
  Iterator begin() const { return scopes.begin(); }

  /// Iterator over symbol scopes.
  Iterator end() const { return scopes.end(); }

  /// Print symbol table to stream.
  inline friend std::ostream &operator<<(std::ostream &os,
                                         const ScopedMap<Key,Value> &st) {
    os << "SymbolTable:\n";
    for (auto scope : st.scopes) {
      os << "- ";
      auto it = scope.begin();
      if (it != scope.end()) {
        std::string symString = simit::util::toString(it->second);
        os << it->first << " -> " << symString << "\n";
        ++it;
      }
      for (; it != scope.end(); ++it) {
        std::string symString = simit::util::toString(it->second);
        os << "  " << it->first << " -> " << symString << "\n";
      }
    }
    return os;
  }

private:
  std::list<Map> scopes;
};

}} // namespace simit::internal

#endif

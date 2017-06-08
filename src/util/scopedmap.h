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
  enum class SearchScope {All, CurrentOnly};

  typedef std::map<Key, Value> Scope;
  typedef typename std::list<Scope>::const_iterator Iterator;

  ScopedMap(const Scope &scope = Scope()) {this->scope(scope);}
  ~ScopedMap() { unscope(); }

  size_t levels() const { return scopes.size(); }

  void clear() {
    simit_iassert(levels() == 1);
    scopes.front().clear();
  }

  /// Add a new level of symbol scoping.
  void scope(const Scope &scope = Scope()) { scopes.push_front(scope); }

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

  // True iff the symbol table contains the given symbol in the relevant scopes.
  bool contains(const Key &symbol, 
                const SearchScope searchScope = SearchScope::All) const {
    switch (searchScope) {
      case SearchScope::CurrentOnly:
        return scopes.front().find(symbol) != scopes.front().end();
      default:
        break;
    }

    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return true;
      }
    }
    return false;
  }

  /// Return the first match.  It is an error to call get if the symbol is not
  /// in the symbol table.
  Value &get(const Key &symbol, 
             const SearchScope searchScope = SearchScope::All) {
    switch (searchScope) {
      case SearchScope::CurrentOnly:
        simit_iassert(scopes.front().find(symbol) != scopes.front().end()) <<
            "Attempting to load symbol (" << symbol << ") not in current scope";
        return scopes.front().at(symbol);
      default:
        break;
    }

    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return scope.at(symbol);
      }
    }
    assert(false);
    simit_ierror << "Attempting to load symbol " << symbol
           << " which is not in the symbol table";
    return scopes.begin()->begin()->second;  // silence warning
  }

  const Value &get(const Key &symbol, 
                   const SearchScope searchScope = SearchScope::All) const {
    switch (searchScope) {
      case SearchScope::CurrentOnly:
        simit_iassert(scopes.front().find(symbol) != scopes.front().end()) <<
            "Attempting to load symbol (" << symbol << ") not in current scope";
        return scopes.front().at(symbol);
      default:
        break;
    }

    for (auto &scope : scopes) {
      if (scope.find(symbol) != scope.end()) {
        return scope.at(symbol);
      }
    }
    simit_ierror << "Attempting to get a symbol that is not in symbol table.";
    return scopes.begin()->at(symbol); // silence warning
  }

  /// Iterator over symbol scopes.
  Iterator begin() const {return scopes.begin();}

  /// Iterator over symbol scopes.
  Iterator end() const {return scopes.end();}

  /// Print symbol table to stream.
  inline friend
  std::ostream &operator<<(std::ostream &os, const ScopedMap<Key,Value> &st) {
    os << "SymbolTable:\n";
    auto scopeIt = st.scopes.rbegin();
    auto end = st.scopes.rend();
    if (scopeIt != end) {
      st.printScope(os, *scopeIt++);
    }
    while (scopeIt != end) {
      os << "\n";
      st.printScope(os, *scopeIt++);
    }
    return os;
  }

private:
  std::list<Scope> scopes;

  void printScope(std::ostream &os, const Scope &scope) const {
    os << "- ";
    auto it = scope.begin();
    if (it != scope.end()) {
      std::string symString = simit::util::toString(it->second);
      os << it->first << " -> " << symString;
      ++it;
    }
    for (; it != scope.end(); ++it) {
      std::string symString = simit::util::toString(it->second);
      os << "\n  " << it->first << " -> " << symString;
    }
  }
};

}} // namespace simit::internal

#endif

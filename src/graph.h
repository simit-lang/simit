#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cassert>
#include <vector>

#include "tensor.h"

using namespace std;
using namespace simit;


namespace simit {

/// An opaque handle for accessing Fields
typedef int FieldHandle;

class Set;
  
/// An opaque handle for accessing an Element
struct ElementHandle {
  const int idx;              // index in the Set
  const Set* set;             // set that this belongs to
  
  /// Get the value of a field of this element
  template <typename T>
  void get(const int field, T* val);
  
  ElementHandle(const int idx, Set* set) : idx(idx), set(set) { }
};
  
namespace internal {

/// A field on the members of the Set.
///
/// Invariant: items < capacity
class Field {
  // TODO: use PIMPL
public:
  Type type;                    // simit type of data
  
  Field(Type type) : type(type) {
    //TODO: support ELEMENT
    if (type == Type::ELEMENT)
      assert(false && "Sets do not currently support ELEMENT types");
    if (type == Type::INT)
      data = (void*) calloc(capacityIncrement, sizeof(int));
    if (type == Type::FLOAT)
      data = (void*) calloc(capacityIncrement, sizeof(double));
    
    capacity = capacityIncrement;
    items = 0;
  }
  
  ~Field() { free(data); }
  
  /// Add an item by adding a new field entry, setting it to the given value
  template <typename T>
  int add(T val) {
    if (items > capacity-1)
      increaseCapacity();
    
    set(items++, val);
    return items;
  }
  
  /// Set the field of item idx to a new value
  template <typename T>
  void set(const int idx, T val);
  
  /// Get the value of a field of an item
  template <typename T>
  void get(const int idx, T* val);
  
  /// Remove an item, avoiding fragmentation.
  void remove(const int idx);
  
  /// Get the number of items
  int size() { return items; }
  
private:
  void* data;                   // buffer for the data
  int capacity;        // current capacity
  int items;           // current number of items
  
  static const int capacityIncrement = 1024;
  
  void increaseCapacity();
  
  // disable copy constructors
  Field(const Field& f);
  Field& operator=(const Field& f);
};

  
  
  
} // namespace internal

using namespace internal;
  
/// A Simit Set.
/// Sets are used to represent collections within C++,
/// and can be passed as bound inputs to Simit programs.
///
/// Invariant: fields[i].size() == size()
class Set {
  
 private:
  vector<Field*> fields;          // fields of the items in the set
  int items;                      // number of items in the set
  
  // disable copy constructors
  Set(const Set& s);
  Set& operator=(const Set& s);
  
 public:
  Set() : items(0) { }
  
  ~Set() {
    for (auto f: fields)
      delete f;
  }
  
  /// Get the number of fields in the Set
  int numFields() { return fields.size(); }
  
  /// Return the number of items in the Set
  int size() { return items; }
  
  /// Add a new field
  FieldHandle addField(Type type) { fields.push_back(new Field(type)); return fields.size()-1; }
  
  /// Return the fields over this Set
  vector<FieldHandle> getFields() {
    vector<FieldHandle> fieldHandles;
    for (int i=0; i<numFields(); i++) {
      fieldHandles.push_back(i);
    }
    
    return fieldHandles;
  }
  
  /// Add a new item, returning its handle
  ElementHandle addItem();
  
  /// Set a field on an item in the Set
  template<typename T>
  void set(ElementHandle element, const FieldHandle field, T val) {
    assert((fields[field]->type == type_of<T>()) && "Incorrect field type.");
  
    fields[field]->set(element.idx, val);
  }
  
  /// Get the value of a field on an item in the Set
  template<typename T>
  void get(ElementHandle element, const FieldHandle field, T* val) const {
    assert((fields[field]->type == type_of<T>()) && "Incorrect field type.");
    
    fields[field]->get(element.idx, val);
  }
  
  /// Remove an item from the Set
  void remove(const ElementHandle element) {
    for (auto f : fields)
      f->remove(element.idx);
    items--;
  }
  
  /// Iterator that iterates over the elements in a Set
  ///
  /// This iterator is an input_iterator, and thus can only be
  /// dereferenced as an rvalue.  Furthermore, it can only return
  /// const references to Elements.
  struct ElementIterator {
    // some typedefs to make interop with std easier
    typedef input_iterator_tag iterator_category;
    typedef ElementHandle value_type;
    typedef ptrdiff_t difference_type;
    typedef ElementHandle& reference;
    typedef ElementHandle* pointer;
    
    int cur_idx;    // current item index
    Set* set;       // set we're iterating over
    
    ElementIterator(Set* set, int idx=0) : cur_idx(idx), set(set) { }
    ElementIterator(const ElementIterator& other) : cur_idx(other.cur_idx), set(other.set) { }
    
    reference operator*() const {
      return *(new ElementHandle(cur_idx, set));
    }
    
    pointer operator->() const {
      return new ElementHandle(cur_idx, set);
    }
    
    ElementIterator& operator++() {
      cur_idx++;
      return *this;
    }
    
    ElementIterator operator++(int) {
      cur_idx++;
      return *this;
    }
    
    bool operator!=(const ElementIterator& other) {
      return !(set==other.set) || !(cur_idx == other.cur_idx);
    }
    
    bool operator==(const ElementIterator& other) {
      return (set==other.set) && (cur_idx == other.cur_idx);
    }
  };
  
  /// Create an ElementIterator for this Set, set to the first element
  ElementIterator begin() { return ElementIterator(this, 0); }

  /// Create an ElementIterator for terminating iteration over this Set
  ElementIterator end() { return ElementIterator(this, size()); }
  

};
  
  
inline bool operator<(const Set::ElementIterator& e1, const Set::ElementIterator& e2) {
  assert(e1.set == e2.set);
  return e1.cur_idx < e2.cur_idx;
}
  


template <typename T>
void ElementHandle::get(const FieldHandle field, T* val) {
  assert (set != nullptr);
  set->get(*this, field, val);
}

  
} // namespace simit

#endif

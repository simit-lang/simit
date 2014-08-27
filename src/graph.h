#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cassert>
#include <vector>
#include <string>
#include <map>

#include "tensor.h"


namespace simit {

/// An opaque handle for accessing Fields
typedef int FieldHandle;
  
/// An opaque handle for accessing an Element
typedef int ElementHandle;

/// A Simit Set.
/// Sets are used to represent collections within C++,
/// and can be passed as bound inputs to Simit programs.
///
/// invariant: fields[i].getSize() == getSize()
class Set {
  
 public:
  Set() : elements(0), capacity(capacityIncrement) { }
  
  ~Set() {
    for (auto f: fields)
      delete f;
  }
  
  /// Return the number of elements in the Set
  int getSize() { return elements; }
  
  /// Get a FieldHandle corresponding to the string fieldName
  FieldHandle getField(std::string fieldName) {
    return fieldNames[fieldName];
  }
  
  /// Add a new field
  FieldHandle addField(Type type, std::string fieldName) {
    Field* f = new Field(type);
    f->data = calloc(capacity, f->size_of_type);
    fields.push_back(f);
    
    FieldHandle field = fields.size()-1;
    fieldNames[fieldName] = field;
    
    return field;
  }
  
  /// Add a new element, returning its handle
  ElementHandle addElement() {
    if (elements > capacity-1)
      increaseCapacity();
    return elements++;
  }
  
  /// Set a field on an element in the Set
  template<typename T>
  void set(ElementHandle element, FieldHandle field, T val) {
    assert((fields[field]->type == typeOf<T>()) && "Incorrect field type.");
    
    *((T*)(fields[field]->data) + element) = val;
  }
  
  /// Get the value of a field on an element in the Set
  template<typename T>
  void get(ElementHandle element, FieldHandle field, T* val) {
    assert((fields[field]->type == typeOf<T>()) && "Incorrect field type.");
    
    T* data = (T*)(fields[field]->data);
    *val = *(data + element);
  }
  
  /// Remove an element from the Set
  void remove(ElementHandle element) {
    for (auto f : fields){
      if (f->type == Type::ELEMENT)
        assert(false && "ELEMENT types not supported yet.");
      if (f->type == Type::FLOAT) {
        double* data = (double*)f->data;
        data[element] = data[elements-1];
      }
      if (f->type == Type::INT) {
        int* data = (int*)f->data;
        data[element] = data[elements-1];
      }
    }
    elements--;
  }
  
  /// Iterator that iterates over the elements in a Set
  ///
  /// This iterator is an input_iterator, and thus can only be
  /// dereferenced as an rvalue.  Furthermore, it can only return
  /// const references to Elements.
  class ElementIterator {
   public:
    // some typedefs to make interop with std easier
    typedef std::input_iterator_tag iterator_category;
    typedef ElementHandle value_type;
    typedef ptrdiff_t difference_type;
    typedef ElementHandle& reference;
    typedef ElementHandle* pointer;
    
    ElementIterator(Set* set, int idx=0) : cur_idx(idx), set(set) { }
    ElementIterator(const ElementIterator& other) : cur_idx(other.cur_idx),
    set(other.set) { }
    
    friend inline bool operator<(const Set::ElementIterator& e1,
                                 const Set::ElementIterator& e2);
    
    reference operator*() {
      return cur_idx;
    }
    
    pointer operator->()  {
      return &cur_idx;
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
    
   private:
    int cur_idx;    // current element index
    Set* set;       // set we're iterating over
  };
  
  /// Create an ElementIterator for this Set, set to the first element
  ElementIterator begin() { return ElementIterator(this, 0); }
  
  /// Create an ElementIterator for terminating iteration over this Set
  ElementIterator end() { return ElementIterator(this, getSize()); }
  
 private:
  // A field on the members of the Set.
  //
  // Invariant: elements < capacity
  struct Field {
    Type type;                    // simit type of data
    size_t size_of_type;          // sizeof(type)
    
    Field(Type type) : type(type), data(nullptr) {
      //TODO: support ELEMENT
      if (type == Type::ELEMENT)
        assert(false && "Sets do not currently support ELEMENT types");
      
      if (type == Type::INT)
        size_of_type = sizeof(int);
      if (type == Type::FLOAT)
        size_of_type = sizeof(double);
    }
    
    ~Field() { if (data != nullptr) free(data); }
    
    void* data;                   // buffer for the data
    
    // disable copy constructors
   private:
    Field(const Field& f);
    Field& operator=(const Field& f);
  };

  std::vector<Field*> fields;          // fields of the elements in the set
  std::map<std::string, FieldHandle> fieldNames; // name to field lookups
  int elements;                      // number of elements in the set
  int capacity;                   // current capacity of the set
  static const int capacityIncrement = 1024; // increment for capacity increases
  
  // disable copy constructors
  Set(const Set& s);
  Set& operator=(const Set& s);
  
  // increase capacity of all fields
  void increaseCapacity() {
    for (auto f : fields) {
      // this strategy gets rid of the conditional
      realloc(f->data, (capacity+capacityIncrement)*f->size_of_type);
      memset((char*)(f->data)+capacity*f->size_of_type,
             0, capacityIncrement*f->size_of_type);
    }
    capacity += capacityIncrement;
  }
  

};
  
  
inline bool operator<(const Set::ElementIterator& e1,
                      const Set::ElementIterator& e2) {
  assert(e1.set == e2.set);
  return e1.cur_idx < e2.cur_idx;
}

  
} // namespace simit

#endif

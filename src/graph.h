#ifndef SIMIT_GRAPH_H
#define SIMIT_GRAPH_H

#include <cassert>
#include <vector>
#include "tensor.h"

using namespace std;
using namespace simit;


namespace simit {
namespace internal {
  /** A field on the members of the Set.
   *
   * Invariant: items < capacity
   */
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
    
    /** Add an item by adding a new field entry, setting it to the given value */
    template <typename T>
    unsigned int add(T val) {
      if (items > capacity-1)
        increaseCapacity();
      
      set(items++, val);
      return items;
    }
    
    /** Set the field of item idx to a new value */
    template <typename T>
    void set(const unsigned int idx, T val);
    
    /** Get the value of a field of an item */
    template <typename T>
    void get(const unsigned int idx, T* val);
    
    /** Remove an item, avoiding fragmentation. */
    void remove(const unsigned int idx);
    
    /** Get the number of items */
    unsigned int size() { return items; }
    
  private:
    void* data;                   // buffer for the data
    unsigned int capacity;        // current capacity
    unsigned int items;           // current number of items
    
    static const unsigned int capacityIncrement = 1024;
    
    void increaseCapacity();
    
    // disable copy constructors
    Field(const Field& f);
    Field& operator=(const Field& f);
  };
  
  
  
  
} // namespace internal

  using namespace internal;
  
/** A Simit Set.
  *
  * Sets are used to represent collections within C++,
  * and can be passed as bound inputs to Simit programs.
  *
  * Invariant: fields[i].size() == size()
  */
class Set {
  
 private:
  vector<Field*> fields;          // fields of the items in the set
  unsigned int items;             // number of items in the set
  
  // disable copy constructors
  Set(const Set& s);
  Set& operator=(const Set& s);
  
 public:
  Set() : items(0) { }
  
  ~Set() {
    for (auto f: fields)
      delete f;
  }
  
  /** Get the number of fields in the Set */
  unsigned int numFields() { return fields.size(); }
  
  /** Return the number of items in the Set */
  unsigned int size() { return items; }
  
  /** Add a new field */
  void addField(Type type) { fields.push_back(new Field(type)); }
  
  /** Add a new item, returning its index */
  unsigned int addItem();
  
  /** Set a field on an item in the Set */
  template<typename T>
  void set(unsigned int idx, unsigned int field, T val) {
    assert((fields[field]->type == type_of<T>()) && "Incorrect field type.");
  
    fields[field]->set(idx, val);
  }
  
  /** Get the value of a field on an item in the Set */
  template<typename T>
  void get(unsigned int idx, unsigned int field, T* val) {
    assert((fields[field]->type == type_of<T>()) && "Incorrect field type.");
    
    fields[field]->get(idx, val);
  }
  
  /** Remove an item from the Set */
  void remove(unsigned int idx) {
    for (auto f : fields)
      f->remove(idx);
    items--;
  }
  
  
};

  

  
}

#endif
